//
//  AppProcessor.hpp
//  Database1
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Application_hpp
#define Application_hpp

#include <memory>
#include <map>
#include <functional>
#include <filesystem>

#include "Database.hpp"
#include "CmdProcessor.hpp"
#include "Statement.hpp"
#include "SQLStatements.hpp"
#include "DBProcessor.hpp"
#include "SQLQuery.hpp"
#include "Config.hpp"

namespace ECE141 {
	class Application;

	namespace fs = std::filesystem;

	// For dynamically dispatching commands
	using AppCmdRouter = void(ECE141::Application::*)(Statement*);

	class Application : public CmdProcessor {
	public:

		/// <summary>
		/// Controller for DB, helps dispatch commands to other CmdProcessor subclasses
		/// and 
		/// </summary>
		/// <param name="anOutput">stream to pass all DB operations' output</param>
		Application(std::ostream& anOutput);
		virtual ~Application() {};

		std::string   getVersion() { return Config::getVersion(); }

		/// <summary>
		/// Interface to database using user input stream.
		/// </summary>
		/// <param name="anInput">input stream of commands</param>
		/// <returns></returns>
		virtual StatusResult  handleInput(std::istream& anInput);

		/// <summary>
		/// Handles recognition for help, version, quit commands, given tokenized Tokens.
		/// </summary>
		/// <param name="aTokenizer"></param>
		/// <returns></returns>
		CmdProcessor*         recognizes(Tokenizer& aTokenizer) override;
		/// <summary>
		/// Statement factory for help, version, quit commands, given tokenized Tokens.
		/// </summary>
		/// <param name="aTokenizer"></param>
		/// <param name="aResult"></param>
		/// <returns></returns>
		Statement*            makeStatement(Tokenizer& aTokenizer) override;
		
		/// <summary>
		/// Dynamic Dispatching function
		/// </summary>
		/// <param name="aStmt"></param>
		/// <returns></returns>
		void          run(Statement* aStmt);
		Database*&            getDatabase();

		//-----------------------------------------
		// Statement Processing Functions, Dispatch's functions
		// to the right place.
		
		void quitCommand(Statement* aStmt);
		void helpCommand(Statement* aStmt);
		void versionCommand(Statement* aStmt);
		
		//DB Commands

		void createDatabase(Statement* aStmt);
		void dumpDatabase(Statement* aStmt);
		void dropDatabase(Statement* aStmt);
		void showDatabases(Statement* aStmt);
		void useDatabase(Statement* aStmt);

		//SQL Statement

		void createTable(Statement* aStmt);
		void describeTable(Statement* aStmt);
		void dropTable(Statement* aStmt);
		void showTables(Statement* aStmt);
		void insertRow(Statement* aStmt);

		void showQuery(Statement* aStmt);
		void updateRows(Statement* aStmt);
		void deleteRows(Statement* aStmt);

		bool dbExists(const std::string& aDBName);

		//----------------------------------------
		//command routing map
		const std::map<StatementType, AppCmdRouter> cmdRouteMap = {
			{StatementType::show_help, &Application::helpCommand},
			{StatementType::quit, &Application::quitCommand},
			{StatementType::get_version, &Application::versionCommand},
			{StatementType::create_db, &Application::createDatabase},
			{StatementType::drop_db, &Application::dropDatabase},
			{StatementType::dump_db, &Application::dumpDatabase},
			{StatementType::show_dbs, &Application::showDatabases},
			{StatementType::use_db, &Application::useDatabase},
			{StatementType::create_tb, &Application::createTable},
			{StatementType::describe_tb, &Application::describeTable},
			{StatementType::drop_tb, &Application::dropTable},
			{StatementType::show_tbs, &Application::showTables},
            {StatementType::insert_row, &Application::insertRow},
			{StatementType::select, &Application::showQuery},
			{StatementType::update_rws, &Application::updateRows},
			{StatementType::delete_rws, &Application::deleteRows},
		};

	private:

		// Holds activate database
		Database* activeDatabase;

		// Helper function
		std::string getTime();

	};
}

#endif /* Application_hpp */
