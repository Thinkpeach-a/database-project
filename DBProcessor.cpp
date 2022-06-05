//
//  DBProcessor.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "DBProcessor.hpp"

#include <map>
#include <fstream>
#include "Config.hpp"
#include "DBProcessor.hpp"
#include "FolderView.hpp"
#include "FolderReader.hpp"
#include "Database.hpp"
#include "Helpers.hpp"
#include "TokenSequencer.hpp"
#include "SQLProcessor.hpp"
#include "DBStatements.hpp"

namespace ECE141 {

	//like a typedef!!!
	using StmtFactory = Statement * (*)(DBProcessor* aProc,
		Tokenizer& aTokenize);

	DBProcessor::DBProcessor(std::ostream& anOutput, Application* anApp) :
		CmdProcessor{ anOutput }, app{ anApp } {
		nextProcessor = new SQLProcessor(output, app);
	}

	DBProcessor::~DBProcessor() {
		// app pointer is stack object, so it will be freed.
	}


	//CmdProcessor interface ...
	CmdProcessor* DBProcessor::recognizes(Tokenizer& aTokenizer) {
		TokenSequencer theSeq(aTokenizer);
		if (CreateDBStatement::recognize(theSeq) ||
			DropDBStatement::recognize(theSeq) ||
			ShowDBsStatement::recognize(theSeq) ||
			DumpDBStatement::recognize(theSeq) ||
			UseDBStatement::recognize(theSeq) ||
			ShowIndexStatement::recognize(theSeq) ||
			ShowIndexesStatement::recognize(theSeq)
			) {
			//Reset tokenizer.
			theSeq.reset();
			return this;
		}
		theSeq.reset();
		return nextProcessor->recognizes(aTokenizer);
	}

	// USE: retrieve a statement based on given text input...
	Statement* DBProcessor::makeStatement(Tokenizer& aTokenizer) {
		TokenSequencer theSeq(aTokenizer);
		Statement* theStatement = nullptr;

		if (CreateDBStatement::recognize(theSeq)) {
			theStatement = new CreateDBStatement(app->getDatabase());
		}
		else if (DropDBStatement::recognize(theSeq)) {
			theStatement = new DropDBStatement(app->getDatabase());
		}
		else if (ShowDBsStatement::recognize(theSeq)) {
			theStatement = new ShowDBsStatement();
		}
		else if (UseDBStatement::recognize(theSeq)) {
			theStatement = new UseDBStatement(app->getDatabase());
		}
		else if (DumpDBStatement::recognize(theSeq)) {
			theStatement = new DumpDBStatement(app->getDatabase());
		}
		else if (ShowIndexStatement::recognize(theSeq)) {
			theStatement = new ShowIndexStatement();
		}
		else if (ShowIndexesStatement::recognize(theSeq)) {
			theStatement = new ShowIndexesStatement();
		}
		// Fill data from statement
		if (theStatement) {
			theStatement->parse(theSeq);
		}
		else {
			throw Errors::invalidCommand;
		}
		theSeq.skipStatement();
		return theStatement;
	}

	void DBProcessor::createDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<CreateDBStatement*>(aStmt);
		Database*& theActiveDatabase = app->getDatabase();
		if (!theActiveDatabase) {
			delete theActiveDatabase;
		}
		theActiveDatabase = new Database(theStatement->getDBName(), CreateDB());
		if (!theActiveDatabase) { throw Errors::databaseCreationError; }

		output << Helpers::QueryOk(1, Config::getTimer().elapsed());
	}

	void DBProcessor::dumpDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<DumpDBStatement*>(aStmt);
		std::string theDBName = theStatement->getDBName();
		Database*& theActiveDatabase = app->getDatabase();

		if (theActiveDatabase && theActiveDatabase->getName() == theDBName) {
			theActiveDatabase->dump(output);
			output << Helpers::rowsInSet(theActiveDatabase->getBlockCount() + 1, Config::getTimer().elapsed());
		}
		else if (app->dbExists(Config::getDBPath(theDBName))) {
			Database theTempDB(theDBName, OpenDB());
			theTempDB.dump(output);
			output << Helpers::rowsInSet(theTempDB.getBlockCount() + 1,
				Config::getTimer().elapsed()); // +1 to count metablock
		}
		else {
			throw Errors::unknownDatabase;
		}
	}

	void DBProcessor::dropDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<DropDBStatement*>(aStmt);
		std::string theName = theStatement->getDBName();
		std::string theDBPath = Config::getDBPath(theName);
		Database*& theActiveDatabase = app->getDatabase();

		//uint64_t theBlockCount = 0;
		if (app->dbExists(theDBPath)) {
			if (!theActiveDatabase) {} // if nullptr, skip next step
			else if (theActiveDatabase->getName() == theName) {
				theActiveDatabase->getBlockCount();
				
				delete theActiveDatabase;
				theActiveDatabase = nullptr;
			}
			// delete file
			fs::remove(theDBPath);
			output << Helpers::QueryOk(0, Config::getTimer().elapsed());
		}
		else {
			throw Errors::unknownDatabase;
		}
	}

	void DBProcessor::showDatabases(Statement* aStmt)
	{
		FolderView theView(Config::getStoragePath());
		theView.show(output);
		output << Helpers::rowsInSet(theView.files.size(), Config::getTimer().elapsed());
	}

	void DBProcessor::useDatabase(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<UseDBStatement*>(aStmt);
		std::string theDBName = theStatement->getDBName();
		Database*& theActiveDatabase = app->getDatabase();

		if (app->dbExists(Config::getDBPath(theDBName))) {
			if (theActiveDatabase) { delete theActiveDatabase; }
			theActiveDatabase = new Database(theDBName, OpenDB());
			output << "Database changed\n";
		}
		else {
			throw Errors::unknownDatabase;
		}
	}

	void DBProcessor::showIndices(Statement* aStmt)
	{
		auto theStatement = dynamic_cast<ShowIndexStatement*>(aStmt);
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			theActiveDatabase->showIndices(theStatement->getTable(), theStatement->getKey(), output);
		}
	}

	void DBProcessor::showIndexes(Statement* aStmt)
	{
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			theActiveDatabase->showIndexes(output);
		}
	}

	void DBProcessor::run(Statement* aStmt)
	{
		(this->*cmdRouteMap.at(aStmt->getType()))(aStmt);
	}


	//-------------------------------------------------------
}
