#include "SelectStatement.hpp"
#include "Filters.hpp"


namespace ECE141 {
	bool SelectStatement::recognize(TokenSequencer& aSequencer) {
		return aSequencer.reset().matchNext({ Keywords::select_kw });
	}

	void SelectStatement::parse(TokenSequencer& aSequencer, Database* aDatabase) {

		StatusResult theResult;

		// More than once is invalid
		std::set<Keywords> theSelectParameters = { Keywords::where_kw, Keywords::group_kw,
		Keywords::order_kw, Keywords::limit_kw,  Keywords::cross_kw, Keywords::full_kw, Keywords::inner_kw,
		Keywords::left_kw,  Keywords::right_kw };
		
		parseFieldNames(aSequencer);

		if (!aSequencer.matchNext({ Keywords::from_kw }).getData(query.EntityName).next()) { throw Errors::invalidCommand; }

		while (aSequencer.tokenizer.more() && !(aSequencer.getData() == ";")) {
			Keywords theKeyword = aSequencer.getKeyword();

			if (!aSequencer.next()) { throw Errors::syntaxError; }
			
			if (theSelectParameters.count(theKeyword) > 0) {

				switch (theKeyword) {
				case Keywords::where_kw:
					parseWhere(aSequencer, aDatabase);
					break;
				case Keywords::group_kw:
					parseGroupBy(aSequencer);
					break;
				case Keywords::order_kw:
					parseOrderBy(aSequencer);
					break;
				case Keywords::limit_kw:
					parseLimit(aSequencer);
					break;
				default:
					if (in_array<Keywords>(Helpers::gJoinTypes, theKeyword)) {
						parseJoin(aSequencer, theKeyword);
					}
					else {
						throw Errors::unexpectedKeyword;
					}
				}
			}
			else {
				throw Errors::invalidArguments;
			}
		}
		//SelectStatement parse
			aSequencer.reset().iterate([&](const Token& aToken) {
			query.selectString += aToken.data + ' ';
			return aToken.data == ";";
				});
	}
	void SelectStatement::parseFieldNames(TokenSequencer& aSequencer) {
		if (!aSequencer.loopParse(query.columnNames,
			[&](const Token& aToken) {return aToken.data == ","; }, // skip commas
			[&](const Token& aToken) {return TokenType::keyword == aToken.type; }, // stop if keyword 
			[&](const Token& aToken) {return (TokenType::identifier == aToken.type ) || ("*" == aToken.data); }) // read only identifiers
			) {
			throw Errors::syntaxError;
		}
	}

	void SelectStatement::parseWhere(TokenSequencer& aSequencer, Database* aDatabase) {
		//Entity anEntity; //TODO: make it not random
		//query.whereFilter.parse(aSequencer.tokenizer, anEntity);
		// Skip for now
		std::optional<Entity> theEntity = aDatabase->getEntity(query.EntityName);
		if (theEntity) {
			StatusResult theResult = query.whereFilter.parse(aSequencer.tokenizer, theEntity.value());
			if (!theResult) { throw theResult.error; }
		}
		else {
			throw Errors::unknownTable;
		}
		
	}
	void SelectStatement::parseOrderBy(TokenSequencer& aSequencer) {
		if (!aSequencer.matchNext({ Keywords::by_kw })) { throw Errors::syntaxError; }
		if (!aSequencer.loopParse(query.orderNames,
			[&](const Token& aToken) {return "," == aToken.data; },
			[&](const Token& aToken) {return aToken.type == TokenType::keyword; },
			[&](const Token& aToken) {return aToken.type == TokenType::identifier; }
			)) {
			throw Errors::syntaxError;
		}
	}
	void SelectStatement::parseGroupBy(TokenSequencer& aSequencer) {
		if (!aSequencer.matchNext({ Keywords::by_kw })) { throw Errors::syntaxError; }
		if (!aSequencer.loopParse(query.groupNames,
			[&](const Token& aToken) {return aToken.data == ","; },
			[&](const Token& aToken) {return aToken.type == TokenType::keyword; },
			[&](const Token& aToken) {return aToken.type == TokenType::identifier; }
		)) {
			throw Errors::syntaxError;
		}
	}
	void SelectStatement::parseLimit(TokenSequencer& aSequencer) {
		std::vector<std::string> theLimitVals;
		if (!aSequencer.loopParse(theLimitVals,
			[&](const Token& aToken) {return aToken.data == ","; },
			[&](const Token& aToken) {return aToken.type == TokenType::keyword; },
			[&](const Token& aToken) {return aToken.type == TokenType::number; }
		)) {
			throw Errors::syntaxError;
		}
		size_t theVectorLen = theLimitVals.size();
		if (theVectorLen == 1) {
			// Only have count
			query.limitStartIndex = 0;
			query.limitCount = std::stoll(theLimitVals[0]);
		}
		else if(theVectorLen == 2) {
			// Has start (index, count)
			query.limitStartIndex = std::stoll(theLimitVals[0]);
			query.limitCount = std::stoll(theLimitVals[1]);
		}
		else {
			throw Errors::syntaxError;
		}
	}
	void SelectStatement::parseJoin(TokenSequencer& aSequencer, Keywords& aJoinType) {
		
		if (aJoinType != Keywords::left_kw) {
			throw Errors::notImplemented;
		}

		if (!aSequencer.matchNext({ Keywords::join_kw })) { throw Errors::syntaxError; }
		// FROM users LEFT JOIN orders ON users.id = orders.user_id
		// Attempt to populate join

		//SELECT column_name(s)
		//FROM table1
		//	LEFT JOIN table2
		//	ON table1.column_name = table2.column_name;
		//query.join.joinType = aJoinType;
		//query.join.leftTable = query.EntityName;

		if (!aSequencer.isIdentifier().getData(query.join.rightTable).next()
			.matchNext({Keywords::on_kw})) { throw syntaxError; }
		std::map<std::string, std::string> theScopeMap;
		theScopeMap.insert(parseScope(aSequencer));
		
		if (!aSequencer.hasData("=").next()) { throw Errors::syntaxError; }

		theScopeMap.insert(parseScope(aSequencer));

		// get LHS
		if (theScopeMap.count(query.EntityName)) {
			query.join.leftValue = theScopeMap.at(query.EntityName);
		}
		else { throw Errors::unknownTable; }
		if (theScopeMap.count(query.join.rightTable)) {
			query.join.rightValue = theScopeMap.at(query.join.rightTable);
		}
		else { throw Errors::unknownTable; }

		query.join.leftTable = query.EntityName;
		query.hasJoin = true;
	}

	std::pair<std::string, std::string> SelectStatement::parseScope(TokenSequencer& aSequencer) {
		std::string theKey;
		std::string theValue;

		if (!aSequencer.isIdentifier().getData(theKey).next().hasData(".").next().isIdentifier().getData(theValue).next()) {
			throw Errors::syntaxError;
		}
		return std::make_pair(theKey, theValue);
	}

	bool UpdateStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext({ Keywords::update_kw });
	}
	void UpdateStatement::parse(TokenSequencer& aSequencer, Database* aDatabase)
	{
		//Format = UPDATE Users SET "zipcode" = 92127 WHERE zipcode>92100;
		if (!aSequencer.isType(TokenType::identifier).getData(query.EntityName).next().matchNext({Keywords::set_kw})) {
			throw Errors::syntaxError;
		}
		//TODO: parse zipcode=thing
		std::string theKey;
		std::string theRawValue;
		Value       theValue;

		std::optional<Entity> theEntity = aDatabase->getEntity(query.EntityName);
		if (!theEntity) { throw Errors::unknownTable; }


		while (aSequencer.getData() != ";") {
			if (!aSequencer) { throw Errors::syntaxError; }
			// Parse Key
			if (!aSequencer.isIdentifier().getData(theKey)) { throw Errors::identifierExpected; }
			// Parse =
			if (!aSequencer.next().hasData("=")) { throw Errors::punctuationExpected; }
			// Parse & Verify Value
			if (!aSequencer.next().getData(theRawValue)) { throw Errors::identifierExpected; }
			theValue = BasicTypes::toValue(theRawValue, theEntity->getAttribute(theKey)->getType());

			updateMap[theKey] = theValue;

			// if "," continue, else break
			if (!(aSequencer.next().getData() == ",")) { break; }
		}
		if (aSequencer.matchNext({ Keywords::where_kw })) {
			parseWhere(aSequencer, aDatabase);
			query.hasWhere = true;
		}
		else {
			query.hasWhere = false;
		}
	}
	bool DeleteStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext({ Keywords::delete_kw, Keywords::from_kw });
	}
	void DeleteStatement::parse(TokenSequencer& aSequencer, Database* aDatabase)
	{
		//Format = DELETE FROM Users WHERE zipcode>92000;
		if (!aSequencer.isType(TokenType::identifier).getData(query.EntityName).next()) {
			throw Errors::syntaxError;
		}
		if (aSequencer.matchNext({ Keywords::where_kw })) {
			parseWhere(aSequencer, aDatabase);
			query.hasWhere = true;
		}
		else {
			query.hasWhere = false;
		}
	}
}


