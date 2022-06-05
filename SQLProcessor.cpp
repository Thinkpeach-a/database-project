//
//  SQLProcessor.cpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//

#include "SQLProcessor.hpp"
#include "Application.hpp"
#include "TokenSequencer.hpp"
#include "SelectStatement.hpp"

namespace ECE141 {
	SQLProcessor::SQLProcessor(std::ostream& anOutput, Application* anApp)
		: CmdProcessor(anOutput), app{ anApp } {
	}

	SQLProcessor::~SQLProcessor() {}

	CmdProcessor* SQLProcessor::recognizes(Tokenizer& aTokenizer) {
		TokenSequencer theSeq(aTokenizer);
        //whether the statements belong to the table statement
		if (CreateTableStatement::recognize(theSeq) ||
			DropTableStatement::recognize(theSeq) ||
			DescribeTableStatement::recognize(theSeq) ||
			ShowTablesStatement::recognize(theSeq) ||
            InsertRowStatement::recognize(theSeq) ||
			SelectStatement::recognize(theSeq) ||
			DeleteStatement::recognize(theSeq) ||
			UpdateStatement::recognize(theSeq))
		{
			theSeq.reset();
			return this;
		}
		theSeq.reset();
		return nextProcessor;
	}
    //make statement from tokenizer
	Statement* SQLProcessor::makeStatement(Tokenizer& aTokenizer) {
		TokenSequencer theSeq(aTokenizer);
		Statement* theStatement = nullptr;
		SelectStatement* theSelectStatement = nullptr;

		if (CreateTableStatement::recognize(theSeq)) {
			theStatement = new CreateTableStatement();
		}
		else if (DropTableStatement::recognize(theSeq)) {
			theStatement = new DropTableStatement();
		}
		else if (DescribeTableStatement::recognize(theSeq)) {
			theStatement = new DescribeTableStatement();
		}
		else if (ShowTablesStatement::recognize(theSeq)) {
			theStatement = new ShowTablesStatement();
		}
        else if (InsertRowStatement::recognize(theSeq)){
            theStatement = new InsertRowStatement();
        }

		if (theStatement) { // parse normal statement
			theStatement->parse(theSeq);
		}
		// Parse Statement with Where, need to pass database reference 
		// Eventually want to refactor coupling issues
		else if (SelectStatement::recognize(theSeq)) {
			theSelectStatement = new SelectStatement();
		}
		else if (UpdateStatement::recognize(theSeq)) {
			theSelectStatement = new UpdateStatement();
		}
		else if (DeleteStatement::recognize(theSeq)) {
			theSelectStatement = new DeleteStatement();
		} // Could not recognize or parse
		else {
			throw Errors::invalidCommand;
		}

		if (!theStatement) {
			if (theSelectStatement) {
				theSelectStatement->parse(theSeq, app->getDatabase());
				theStatement = dynamic_cast<Statement*>(theSelectStatement);
			}
			else { // if new fails
				throw Errors::invalidCommand; 
			}
		}
		
		theSeq.skipStatement();
		return theStatement;
	}

	// table related functions
	void SQLProcessor::createTable(Statement* aStmt)
	{
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			auto theStatement = dynamic_cast<CreateTableStatement*>(aStmt);
			theActiveDatabase->createTable(theStatement->getEntity());
			// Always creates 1 block
			output << Helpers::QueryOk(1, Config::getTimer().elapsed());
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void SQLProcessor::describeTable(Statement* aStmt)
	{
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			auto theStatement = dynamic_cast<DescribeTableStatement*>(aStmt);
			theActiveDatabase->describeTable(theStatement->getTableName(), output);
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void SQLProcessor::dropTable(Statement* aStmt)
	{
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			auto theStatement = dynamic_cast<DropTableStatement*>(aStmt);
			theActiveDatabase->dropTable(theStatement->getTableName(), output);
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void SQLProcessor::showTables(Statement* aStmt)
	{
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			auto theStatement = dynamic_cast<ShowTablesStatement*>(aStmt);
			theActiveDatabase->showTables(output);
		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void SQLProcessor::insertRow(Statement* aStmt) {
		Database*& theActiveDatabase = app->getDatabase();
		if (theActiveDatabase) {
			auto theStatement = dynamic_cast<InsertRowStatement*>(aStmt);
			//some actions...
			theActiveDatabase->insertRows(theStatement->getTableName(), theStatement->getNamesVector(),
				theStatement->getRowsVector());
			output << Helpers::QueryOk(theStatement->getRowsVector().size(), Config::getTimer().elapsed());

		}
		else {
			throw Errors::noDatabaseSpecified;
		}
	}

	void SQLProcessor::showQuery(Statement* aStmt) {
		SelectStatement* theStatement = dynamic_cast<SelectStatement*>(aStmt);
		Database*& theActiveDatabase = app->getDatabase();
		theActiveDatabase->selectFromTable(theStatement->getQuery(), output);
	}

	void SQLProcessor::updateRows(Statement* aStmt)
	{
		UpdateStatement* theStatement = dynamic_cast<UpdateStatement*>(dynamic_cast<SelectStatement*>(aStmt));
		Database*& theActiveDatabase = app->getDatabase();
		theActiveDatabase->updateRows(theStatement->getQuery(), theStatement->updateMap, output);
	}

	void SQLProcessor::deleteRows(Statement* aStmt)
	{
		SelectStatement* theStatement = dynamic_cast<SelectStatement*>(aStmt);
		Database*& theActiveDatabase = app->getDatabase();
		theActiveDatabase->deleteRows(theStatement->getQuery(), output);
	}

	void SQLProcessor::run(Statement* aStmt)
	{
		(this->*cmdRouteMap.at(aStmt->getType()))(aStmt);
	}
}
