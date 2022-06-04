//
//  SQLProcessor.hpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include "CmdProcessor.hpp"
#include "Tokenizer.hpp"
#include "Application.hpp"
#include "SQLStatements.hpp"

namespace ECE141 {

	class Statement;
	class DBProcessor; //define this later...
	class Entity;
	class Database;
	class SQLProcessor;

	using SQLCmdRouter = void(ECE141::SQLProcessor::*)(Statement*);

	class SQLProcessor : public CmdProcessor {
	public:

		/// <summary>
		/// SQL Statement Factory for interactions with Tables,
		/// last processor in chain of command
		/// </summary>
		/// <param name="anOutput">stream to pass outputs</param>
		/// <param name="anApp">pointer to controller</param>
		SQLProcessor(std::ostream& anOutput, Application* anApp);
		virtual ~SQLProcessor();

		/// <summary>
		/// Recognizes table and data operation commands
		/// </summary>
		/// <param name="aTokenizer">a separated tokenizer</param>
		/// <returns>self if recognizes statement, else nullptr</returns>
		CmdProcessor* recognizes(Tokenizer& aTokenizer) override;

		/// <summary>
		/// Creates a pointer to a subclass of Statement 
		/// for SQL related commands.
		/// </summary>
		/// <param name="aTokenizer"></param>
		/// <param name="aResult"></param>
		/// <returns></returns>
		Statement* makeStatement(Tokenizer& aTokenizer) override;

		void createTable(Statement* aStmt);
		void describeTable(Statement* aStmt);
		void dropTable(Statement* aStmt);
		void showTables(Statement* aStmt);
		void insertRow(Statement* aStmt);

		void showQuery(Statement* aStmt);
		void updateRows(Statement* aStmt);
		void deleteRows(Statement* aStmt);

		void run(Statement* aStmt) override;

	protected:
		// Pointer to controller
		const std::map<StatementType, SQLCmdRouter> cmdRouteMap = {
			{StatementType::create_tb, &SQLProcessor::createTable},
			{StatementType::describe_tb, &SQLProcessor::describeTable},
			{StatementType::drop_tb, &SQLProcessor::dropTable},
			{StatementType::show_tbs, &SQLProcessor::showTables},
			{StatementType::insert_row, &SQLProcessor::insertRow},
			{StatementType::select, &SQLProcessor::showQuery},
			{StatementType::update_rws, &SQLProcessor::updateRows},
			{StatementType::delete_rws, &SQLProcessor::deleteRows},
		};

		Application* app;
	};

}
#endif /* SQLProcessor_hpp */