#include "SQLStatements.hpp"

namespace ECE141 {
	bool hastheSameName(std::vector<std::string>& attributesNames);
	const std::initializer_list<Keywords> SQLStatement::createTablesList = { Keywords::create_kw, Keywords::table_kw };
	const std::initializer_list<Keywords> SQLStatement::describeTableList = { Keywords::describe_kw };
	const std::initializer_list<Keywords> SQLStatement::dropTableList = { Keywords::drop_kw, Keywords::table_kw };
	const std::initializer_list<Keywords> SQLStatement::showTablesList{ Keywords::show_kw, Keywords::tables_kw };
	const std::initializer_list<Keywords> SQLStatement::insertRowList{ Keywords::insert_kw, Keywords::into_kw };

	using SQLS = SQLStatement;

	bool CreateTableStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(SQLS::createTablesList);
	}
	//use parse to complete the statement

	void CreateTableStatement::parse(TokenSequencer& aSequencer)
	{
		std::string tableName;
		if (!aSequencer.isIdentifier().getData(tableName).next().hasData("(").next()) { throw Errors::syntaxError; }

		entity.updateName(tableName);
		//stop when meet ) or ;
		while (aSequencer && ")" != aSequencer.getData() && ";" != aSequencer.getData()) {
			parseAttributes(aSequencer, entity);
		}

	}

	void CreateTableStatement::parseAttributes(TokenSequencer& aSequencer, Entity& anEntity)
	{
		Attribute theAttribute;
		std::string theName;
		// Get attribute name, type, then loop for constraints
		// Name
		if (aSequencer.isIdentifier().getData(theName).next()) { theAttribute.setName(theName); }
		// Type
		Keywords theType = aSequencer.getKeyword();
		if (typeMap.count(theType) == 0) { throw Errors::keyValueMismatch; }
		theAttribute.setDataType(typeMap[theType]);
		if (Keywords::varchar_kw == theType) { // also parse length tokens for string
			if (aSequencer.next().hasData("(").next().isType(TokenType::number)) {
				theAttribute.setSize(aSequencer.getValue());
				// skip closing paren
				if (!(aSequencer.next().hasData(")"))) { throw Errors::operatorExpected; }
			}
			else {
				throw Errors::syntaxError;
			}
		}
		std::string theEndChar = aSequencer.next().getData();
		while (aSequencer && "," != theEndChar && ")" != theEndChar) {	// Loop for constraints
			parseConstraints(aSequencer, theAttribute);
			theEndChar = aSequencer.getData();
		}
		// skip past comma
		aSequencer.next();

		entity.addAttribute(theAttribute);
	}

	void CreateTableStatement::parseConstraints(TokenSequencer& aSequencer, Attribute& anAttribute)
	{
		aSequencer.saveCurrentIndex();
		if (aSequencer.reset().matchNext({ Keywords::primary_kw, Keywords::key_kw })) {
			anAttribute.setPrimaryKey(true);
		}
		else if (aSequencer.reset().matchNext({ Keywords::auto_increment_kw })) {
			anAttribute.setAutoIncrement(true);
		}
		else if (aSequencer.reset().matchNext({ Keywords::not_kw, Keywords::null_kw })) {
			anAttribute.setNullable(false);
		}
		else if (aSequencer.reset().matchNext({ Keywords::default_kw })) {
			std::string theDefaultVal;
			aSequencer.getData(theDefaultVal);
			anAttribute.setDefault(true, theDefaultVal);
			aSequencer.next();
		}

		else {
			throw Errors::unknownAttribute;
		}
	}


	bool DropTableStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(SQLS::dropTableList);
	}

	void DropTableStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.isIdentifier().getData(tableName)) { throw Errors::unexpectedKeyword; }

		if (!aSequencer.next().hasData(";")) { throw Errors::punctuationExpected; }
	}

	bool DescribeTableStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(SQLS::describeTableList);
	}

	void DescribeTableStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.isIdentifier().getData(tableName)) {
			throw Errors::unknownCommand;
		}
		if (!aSequencer.next().hasData(";")) {
			throw Errors::punctuationExpected;
		}

	}

	bool ShowTablesStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(SQLS::showTablesList);
	}

	void ShowTablesStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.reset().matchNext(SQLS::showTablesList).hasData(";")) {
			throw Errors::punctuationExpected;
		}
	}

	bool InsertRowStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(SQLS::insertRowList);
	}

	void InsertRowStatement::parse(TokenSequencer& aSequencer) {
		//get the table name Insert into users('','') values('abc','cd')
		ParseHelper theParseHelper{ aSequencer.tokenizer };
		// std::string tableName;
		// std::vector<std::string> theAttributesNames;
		if (aSequencer.isIdentifier().getData(tableName).next().hasData("(").next()) {
			if (!parseAttributesNames(aSequencer, theAttributeNames)) {
				throw Errors::syntaxError;
			}
			//after parsing the name make validation: no repeat names:
				//check whether the list contains two same names:

			if (aSequencer.matchNext(Keywords::values_kw)) {
				//match the insertion value
				while (aSequencer.tokenizer.more()) {
					std::vector<Value> theReceiveValue;
					if (!parseAttributeValues(aSequencer, theReceiveValue)) {
						throw Errors::syntaxError;
					}
					else {
						//check if the receivevalve has the same size with the fields;
						if (theReceiveValue.size() != theAttributeNames.size()) { throw Errors::syntaxError; }
						theAttributeValues.push_back(theReceiveValue);
						if (";" == aSequencer.getData()) {
							break;
						}
						if ("," == aSequencer.getData()) {
							aSequencer.next();
						}
						else {
							throw Errors::syntaxError;
						}
					}
				}
			}

		}
		else {
			throw Errors::syntaxError;
		}
	}

	bool InsertRowStatement::parseAttributesNames(TokenSequencer& aSequencer, std::vector<std::string>& attributesNames) {
		ParseHelper theParseHelper{ aSequencer.tokenizer };
		if (!theParseHelper.parseIdentifierList(attributesNames)) {
			return false;
		}
		//check the next two values
		if (!aSequencer.hasData(")").next()) {
			return false;
		}
		if (hastheSameName(attributesNames)) {
			return false;
		}
		return true;
	}

	bool InsertRowStatement::parseAttributeValues(TokenSequencer& aSequencer, std::vector<Value>& rowValues) {
		ParseHelper theParseHelper{ aSequencer.tokenizer };
		if (aSequencer.hasData("(").next()) {
			if (theParseHelper.parseValueList(rowValues)) {

			}
			else {
				return false;
			}

		}
		else {
			return false;
		}
		return true;
	}
	bool hastheSameName(std::vector<std::string>& attributesNames) {
		std::set<std::string> theNames;
		for (auto iter = attributesNames.begin(); iter != attributesNames.end(); iter++) {

			//if we cannot find the name
			if (theNames.find(*iter) == theNames.end()) {
				theNames.insert(*iter);
			}
			else {
				return true;
			}

		}
		return false;
	}

}
