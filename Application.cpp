//
//  CommandProcessor.cpp
//
//  Created by rick gessner on 3/30/20
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <iostream>
#include "Application.hpp"
#include "Tokenizer.hpp"
#include <memory>
#include <exception>
#include <algorithm>
#include <vector>
#include "Config.hpp"
#include "DBProcessor.hpp"
#include "Statement.hpp"
#include "DBStatements.hpp"
#include "FolderView.hpp"
#include "SelectStatement.hpp"

namespace ECE141 {

	size_t Config::cacheSize[] = { 0,0,0 };

	Application::Application(std::ostream& anOutput)
		: CmdProcessor(anOutput) {
		nextProcessor = new DBProcessor(anOutput, this);
		activeDatabase = nullptr;
	}

	bool isKnown(Keywords aKeyword) {
		static Keywords theKnown[] =
		{ Keywords::quit_kw,Keywords::version_kw, Keywords::help_kw };
		auto theIt = std::find(std::begin(theKnown),
			std::end(theKnown), aKeyword);
		return theIt != std::end(theKnown);
	}

	CmdProcessor* Application::recognizes(Tokenizer& aTokenizer) {
		if (isKnown(aTokenizer.current().keyword)) {
			return this;
		}
		return nextProcessor->recognizes(aTokenizer);
	}
	//for the whole application to run the actual statement
	void Application::run(Statement* aStatement) {
		(this->*cmdRouteMap.at(aStatement->getType()))(aStatement);
	}

	//-------------------------------------
	// version help and quit commands
	Database*& Application::getDatabase() {
		return activeDatabase;
	}

	void Application::quitCommand(Statement* aStmt)
	{
		output << "DB::141 is shutting down\n";
		throw Errors::userTerminated;
	}

	void Application::helpCommand(Statement* aStmt)
	{
		output << "Help system available\n";
	}

	void Application::versionCommand(Statement* aStmt)
	{
		output << "Version " << getVersion() << "\n";
	}

	//database commands
	void Application::createDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<CreateDBStatement*>(aStmt);
		if (!activeDatabase) {
			delete activeDatabase;
		}
		activeDatabase = new Database(theStatement->getDBName(), CreateDB());
		if (!activeDatabase) { throw Errors::databaseCreationError; }
		output << "Query OK, 1 row affected" << getTime();
	}

	void Application::dumpDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<DumpDBStatement*>(aStmt);
		std::string theDBName = theStatement->getDBName();
		if (activeDatabase && activeDatabase->getName() == theDBName) {
			activeDatabase->dump(output);
			output << activeDatabase->getBlockCount() + 1 << " rows in set" << getTime();
		}
		else if (dbExists(Config::getDBPath(theDBName))) {
			Database theTempDB(theDBName, OpenDB());
			theTempDB.dump(output);
			output << theTempDB.getBlockCount() + 1 << " rows in set" << getTime(); // +1 to count metablock
		}
		else {
			throw Errors::unknownDatabase;
		}
	}

	void Application::dropDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<DropDBStatement*>(aStmt);
		std::string theName = theStatement->getDBName();
		std::string theDBPath = Config::getDBPath(theName);
		uint64_t theBlockCount = 0;
		if (dbExists(theDBPath)) {
			if (!activeDatabase) {} // if nullptr, skip next step
			else if (activeDatabase->getName() == theName) {
				theBlockCount = activeDatabase->getBlockCount();
				delete activeDatabase;
				activeDatabase = nullptr;
			}
			// delete file
			fs::remove(theDBPath);
			output << "Query OK, " << 0 << " rows affected" << getTime();
		}
		else {
			throw Errors::unknownDatabase;
		}
	}

	void Application::showDatabases(Statement* aStmt)
	{
		FolderView theView(Config::getStoragePath());
		theView.show(output);
		output << theView.files.size() << " rows in set" << getTime();
	}

	void Application::useDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<UseDBStatement*>(aStmt);
		std::string theDBName = theStatement->getDBName();
		if (dbExists(Config::getDBPath(theDBName))) {
			if (activeDatabase) { delete activeDatabase; }
			activeDatabase = new Database(theDBName, OpenDB());
			output << "Database changed\n";
		}
		else {
			throw Errors::unknownDatabase;
		}
	}

	// table related functions
	void Application::createTable(Statement* aStmt)
	{
		if (activeDatabase) {
			auto theStatement = dynamic_cast<CreateTableStatement*>(aStmt);
			activeDatabase->createTable(theStatement->getEntity());
			// Always creates 1 block
			output << Helpers::QueryOk(1, Config::getTimer().elapsed());
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void Application::describeTable(Statement* aStmt)
	{
		if (activeDatabase) {
			auto theStatement = dynamic_cast<DescribeTableStatement*>(aStmt);
			uint64_t attributeCount = 0;
			activeDatabase->describeTable(theStatement->getTableName(), output);
			output << getTime();
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void Application::dropTable(Statement* aStmt)
	{
		if (activeDatabase) {
			auto theStatement = dynamic_cast<DropTableStatement*>(aStmt);
			activeDatabase->dropTable(theStatement->getTableName(), output);
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void Application::showTables(Statement* aStmt)
	{
		if (activeDatabase) {
			auto theStatement = dynamic_cast<ShowTablesStatement*>(aStmt);
			activeDatabase->showTables(output);
			output << getTime();
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void Application::insertRow(Statement* aStmt) {
		if (activeDatabase) {
			auto theStatement = dynamic_cast<InsertRowStatement*>(aStmt);
			//some actions...
			activeDatabase->insertRows(theStatement->getTableName(), theStatement->getNamesVector(),
				theStatement->getRowsVector());
			output << "Query OK, " << theStatement->getRowsVector().size() << " rows affected" << getTime();

		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void Application::showQuery(Statement* aStmt) {
		SelectStatement* theStatement = dynamic_cast<SelectStatement*>(aStmt);

		activeDatabase->selectFromTable(theStatement->getQuery(), output);
		output << getTime();
	}

	void Application::updateRows(Statement* aStmt)
	{
		UpdateStatement* theStatement = dynamic_cast<UpdateStatement*>(dynamic_cast<SelectStatement*>(aStmt));
		activeDatabase->updateRows(theStatement->getQuery(), theStatement->updateMap, output);
	}

	void Application::deleteRows(Statement* aStmt)
	{
		SelectStatement* theStatement = dynamic_cast<SelectStatement*>(aStmt);
		activeDatabase->deleteRows(theStatement->getQuery(), output);
	}

	bool Application::dbExists(const std::string& aDBName) {
		return fs::exists(aDBName);
	}

	std::string Application::getTime() {
		std::stringstream theOutput;
		theOutput << " (" << Config::getTimer().elapsed() << " secs)\n";
		return theOutput.str();
	}

	//makestatement
	Statement* Application::makeStatement(Tokenizer& aTokenizer) {
		Statement* theStatement = nullptr;
		Token theToken = aTokenizer.current();
		if (isKnown(theToken.keyword)) {
			//aTokenizer.next(); //skip ahead...
			switch (theToken.keyword) {
			case Keywords::quit_kw:
				theStatement = new Statement(StatementType::quit);
				break;
			case Keywords::version_kw:
				theStatement = new Statement(StatementType::get_version);
				break;
			case Keywords::help_kw:
				theStatement = new Statement(StatementType::show_help);
				break;
			default:
				break;
			}
		}
		TokenSequencer aSeq(aTokenizer);
		aSeq.skipStatement();

		return theStatement;
	}

	//build a tokenizer, tokenize input, ask processors to handle...
	StatusResult Application::handleInput(std::istream& anInput) {
		Tokenizer theTokenizer(anInput);
		StatusResult theResult = theTokenizer.tokenize();
		if (theTokenizer.size() == 0) { return theResult; } // Skip Empty Commands

		try {
			while (theResult && theTokenizer.more()) {
				Config::getTimer().reset();
				if (auto* theProc = recognizes(theTokenizer)) {
					if (auto* theCmd = theProc->makeStatement(theTokenizer)) {
						if (theResult) {
							theProc->run(theCmd);
							if (theResult) theTokenizer.skipIf(';');
							delete theCmd;
						}
					}
				}
				else theResult = unknownCommand;
			}
			if (!theTokenizer.more()) {
				if (theTokenizer.current().data != ";") { theResult = Errors::punctuationExpected; }
			}
			return theResult;
		}
		catch (Errors theError) {
			if (theError != Errors::userTerminated) {
				std::cout << "The caught error code: " << theError << std::endl;
			}
			return theError;
		}
		catch (std::exception& theException) {
			std::cout << theException.what() << '\n';
			return Errors::unknownError;
		}

	}

}
