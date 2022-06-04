#ifndef SelectStatement_hpp
#define SelectStatement_hpp

#include <set>

#include "SQLStatements.hpp"
#include "SQLQuery.hpp"
#include "Helpers.hpp"
#include "Database.hpp"

namespace ECE141 {

	class SelectStatement : public SQLStatement {
	public:
		SelectStatement(StatementType aType = StatementType::select) : SQLStatement{ aType } {}

		static bool recognize(TokenSequencer& aSequencer);
		virtual void parse(TokenSequencer& aSequencer, Database* aDatabase);
		std::string getName() { return tableName; }

		SQLQuery& getQuery() { return query; }

	protected:
		std::string tableName;

		SQLQuery query;

		void parseFieldNames(TokenSequencer& aSequencer);
		void parseWhere(TokenSequencer& aSequencer, Database* aDatabase);
		void parseOrderBy(TokenSequencer& aSequencer);
		void parseGroupBy(TokenSequencer& aSequencer);
		void parseLimit(TokenSequencer& aSequencer);
		void parseJoin(TokenSequencer& aSequencer, Keywords& aJoinType);

		std::pair<std::string, std::string> parseScope(TokenSequencer& aSequencer);
	};

	class UpdateStatement : public SelectStatement {
	public:
		UpdateStatement() : SelectStatement{ StatementType::update_rws } {};
		
		static bool recognize(TokenSequencer& aSequencer);
		void parse(TokenSequencer& aSequencer, Database* aDatabase) override;

		KeyValues updateMap;

	};

	class DeleteStatement : public SelectStatement {
	public:
		DeleteStatement() : SelectStatement{ StatementType::delete_rws } {};

		static bool recognize(TokenSequencer& aSequencer);
		void parse(TokenSequencer& aSequencer, Database* aDatabase) override;

	};


}


#endif // SelectStatement_hpp
