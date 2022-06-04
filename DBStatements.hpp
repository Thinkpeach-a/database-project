#ifndef DBStatements_hpp
#define DBStatements_hpp

#include <string>
#include <initializer_list>

#include "Statement.hpp"
#include "Database.hpp"
#include "TokenSequencer.hpp"

namespace ECE141 {
	class DBStatements : public Statement {
	public:
        //contains all the information about the
        //database name: keywords
		DBStatements(Database* aDatabase, StatementType aType = StatementType::unknown_type) :
			Statement{ aType }, database{ aDatabase } {}

		virtual ~DBStatements() {}

		std::string getDBName() { return dbName; }
        
		static const std::initializer_list<Keywords> createDBList;
		static const std::initializer_list<Keywords> dropDBList;
		static const std::initializer_list<Keywords> showDBsList;
		static const std::initializer_list<Keywords> dumpDBList;
	protected:
		Database* database;
		std::string dbName = "";
	};
    
    
	class ShowDBsStatement : public Statement {
	public:
		ShowDBsStatement() : Statement{ StatementType::show_dbs } {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
	};

	class CreateDBStatement : public DBStatements {
	public:
		CreateDBStatement(Database* aDatabase) :
			DBStatements(aDatabase, StatementType::create_db) {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
	};

	class DropDBStatement : public DBStatements {
	public:
        //initialize the database with the name to
        //drop
		DropDBStatement(Database* aDatabase) :
			DBStatements(aDatabase, StatementType::drop_db) {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
	};

	class UseDBStatement : public DBStatements {
	public:
        //initialize the database with the name
        //to use
		UseDBStatement(Database* aDatabase) :
			DBStatements(aDatabase, StatementType::use_db) {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
	};

	class DumpDBStatement : public DBStatements {
	public:
        //initialize the  dumbdbstatement with atype
		DumpDBStatement(Database* aDatabase) :
			DBStatements(aDatabase, StatementType::dump_db) {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
	};

	class ShowIndexStatement : public Statement {
	public:
		ShowIndexStatement() : Statement(StatementType::show_idx) {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;

		std::string getTable() { return tableName; }
		std::string getKey() { return keyName; }

	protected:
		std::string tableName;
		std::string keyName;
	};

	class ShowIndexesStatement : public Statement {
	public:
		ShowIndexesStatement() : Statement(StatementType::show_idxs) {}

		static bool recognize(TokenSequencer& aSequencer){
			return aSequencer.reset().matchNext({Keywords::show_kw, Keywords::indexes_kw});
		};

		void parse(TokenSequencer& aSequencer) override {};
	};
}
#endif // DBStatements_hpp
