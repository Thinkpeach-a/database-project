#include "DBStatements.hpp"

namespace ECE141 {
	const std::initializer_list<Keywords> DBStatements::createDBList = { Keywords::create_kw, Keywords::database_kw };
	const std::initializer_list<Keywords> DBStatements::dropDBList = { Keywords::drop_kw, Keywords::database_kw };
	const std::initializer_list<Keywords> DBStatements::showDBsList = { Keywords::show_kw, Keywords::databases_kw };
	const std::initializer_list<Keywords> DBStatements::dumpDBList = { Keywords::dump_kw, Keywords::database_kw };

	using DBS = DBStatements;
	//ShowDB

	bool ShowDBsStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(DBS::showDBsList);
	}

	void ShowDBsStatement::parse(TokenSequencer& aSequencer) {
		if (!aSequencer.hasData(";")) { throw Errors::punctuationExpected; }
	}

	bool CreateDBStatement::recognize(TokenSequencer& aSequencer) {
		return aSequencer.reset().matchNext(DBS::createDBList);
	}

	void CreateDBStatement::parse(TokenSequencer& aSequencer) {
		if (!aSequencer.isIdentifier().getData(dbName)) {
			throw Errors::unknownCommand;
		}
		if (!aSequencer.next().hasData(";")) {
			throw Errors::punctuationExpected;
		}
	}

	bool DropDBStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(DBS::dropDBList);
	}

	void DropDBStatement::parse(TokenSequencer& aSequencer) {

		if (!aSequencer.isIdentifier().getData(dbName)) {
			throw Errors::unknownCommand;
		}
		if (!aSequencer.next().hasData(";")) {
			throw Errors::punctuationExpected;
		}
	}

	bool UseDBStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext({ Keywords::use_kw });
	}

	void UseDBStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.isIdentifier().getData(dbName)) {
			throw Errors::unknownCommand;
		}
		if (!aSequencer.next().hasData(";")) {
			throw Errors::punctuationExpected;
		}
	}

	bool DumpDBStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext(DBS::dumpDBList);
	}

	void DumpDBStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.isIdentifier().getData(dbName)) {
			throw Errors::unknownCommand;
		}
		if (!aSequencer.next().hasData(";")) {
			throw Errors::punctuationExpected;
		}
	}

	bool ShowIndexStatement::recognize(TokenSequencer& aSequencer)
	{
		return aSequencer.reset().matchNext({ Keywords::show_kw, Keywords::index_kw });
	}

	void ShowIndexStatement::parse(TokenSequencer& aSequencer)
	{
		if (!aSequencer.isIdentifier().getData(keyName).next()
			.matchNext({ Keywords::from_kw }).isIdentifier().getData(tableName)) {
			throw Errors::syntaxError;
		}
	}
}