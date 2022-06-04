#ifndef TableLoader_hpp
#define TableLoader_hpp

#include <string>

#include "Storage.hpp"
#include "Database.hpp"
#include "SQLQuery.hpp"
#include "TableView.hpp"

namespace ECE141 {

	// Workflow
	// Create Table Loader
	// loadRows
	// - Load all rows
	// - if join, load both
	//	 - update Header in tables
	// 
	class TableLoader {
	public:
		TableLoader(Storage*& aStorage, Database*& aDatabase) : storage(aStorage), database(aDatabase) {};

		TableLoader& loadTables(SQLQuery& aQuery);

		void showOutputs(std::ostream& anOutput);

	protected:
		Storage*& storage;
		Database*& database;

		std::vector<std::shared_ptr<Row>> leftRows;
		std::vector<std::shared_ptr<Row>> rightRows;
		std::vector<std::shared_ptr<Row>> outputRows;

		//Primitives
		static void sortRows(std::string& aName, std::vector<std::shared_ptr<Row>>& aRowsList);

		void conditionalLoad(std::string& anEnitityName, Filters& aFilter, std::vector<std::shared_ptr<Row>>& aRowsList);
		void applyJoin(std::string& aLeftCol, std::string& aRightCol);
		void applyLimit(std::vector<std::shared_ptr<Row>>& aRowsList, size_t aStartIndex = 0, size_t aLimitCount = 0);

		std::shared_ptr<Row> buildJoinedRow(std::shared_ptr<Row>& aFirstRow, std::shared_ptr<Row>& aSecondRow) { return nullptr; };

	};

}


#endif // TableLoader_hpp

