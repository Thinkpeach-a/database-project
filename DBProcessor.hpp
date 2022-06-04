//
//  DBProcessor.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include <stdio.h>
#include "CmdProcessor.hpp"
#include "Tokenizer.hpp"
#include "Database.hpp"
#include "Application.hpp"


namespace ECE141 {

	class DBStatement;
	class Application;
	class DBProcessor;

	using DBCmdRouter = void(ECE141::DBProcessor::*)(Statement*);

	class DBProcessor : public CmdProcessor {
	public:

		/// <summary>
		/// Statement Factory for Database related Commands, takes in tokenizer and 
		/// pointer to the controller Application.
		/// </summary>
		/// <param name="anOutput"></param>
		/// <param name="anApp"></param>
		DBProcessor(std::ostream& anOutput, Application* anApp);
		~DBProcessor();

		/// <summary>
		/// Recognizes create database, drop database, show databases, dump database, use. 
		/// </summary>
		/// <param name="aTokenizer"></param>
		/// <returns>the DBProcessor if one of the recognizes returns true,
		///  else a SQL processor</returns>
		CmdProcessor* recognizes(Tokenizer& aTokenizer) override;

		/// <summary>
		/// Creates a DBStatement/DB related Statement of one of the 
		/// recognized types.
		/// </summary>
		/// <param name="aTokenizer"></param>
		/// <param name="aResult"></param>
		/// <returns></returns>
		Statement* makeStatement(Tokenizer& aTokenizer) override;

		void run(Statement* aStmt) override;

		// Command Routing
		void createDatabase(Statement* aStmt);
		void dumpDatabase(Statement* aStmt);
		void dropDatabase(Statement* aStmt);
		void showDatabases(Statement* aStmt);
		void useDatabase(Statement* aStmt);
		void showIndices(Statement* aStmt);
		void showIndexes(Statement* aStmt);

	protected:
		// Reference to controller
		Application* app;

		const std::map<StatementType, DBCmdRouter> cmdRouteMap = {
		{StatementType::create_db, &DBProcessor::createDatabase},
		{StatementType::drop_db, &DBProcessor::dropDatabase},
		{StatementType::dump_db, &DBProcessor::dumpDatabase},
		{StatementType::show_dbs, &DBProcessor::showDatabases},
		{StatementType::use_db, &DBProcessor::useDatabase},
		{StatementType::show_idx, &DBProcessor::showIndices},
		{StatementType::show_idxs, &DBProcessor::showIndexes},
		};
	};

}
#endif /* DBProcessor_hpp */
