//
//  Database.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <fstream> 
#include <map>
#include <iostream>

#include "Storage.hpp"
#include "Row.hpp"
#include "Entity.hpp"
#include "SQLQuery.hpp"
#include "BasicTypes.hpp"
#include "Config.hpp"
#include "Index.hpp"
#include "LRUCache.hpp"
#include "TableView.hpp"

namespace ECE141 {
	using ValueRowPair = std::pair<Value, Row>;
	using Rows = std::vector<Row>;
	using Entities = std::vector<Entity>;
	using RowOpt = std::optional<Row>;
	using RowsOpt = std::vector<RowOpt>;
	using RowsPtr = std::vector<std::shared_ptr<Row>>;

	class Database :public Storable {
	public:
		/*Database() {}*/
		Database(const std::string aPath, CreateDB);
		Database(const std::string aPath, OpenDB);
		~Database();

		void    createTable(Entity& anEntity);

		void    dropTable(std::string aTableName, std::ostream& aStream);

		

		void    showTables(std::ostream& aStream);

		void    describeTable(std::string anEntityName, std::ostream& anOuptut);

		void    selectFromTable(SQLQuery& aQuery, std::ostream& anOuptut);

		void    updateRows(SQLQuery& aQuery, KeyValues aReplaceMap, std::ostream& anOuptut);

		void    deleteRows(SQLQuery& aQuery, std::ostream& anOuptut);

		void    showIndices(std::string aTableName, std::string aKeyName, std::ostream& anOutput);

		void    showIndexes(std::ostream& anOutput);

		// TODO: Replace type with something more readable
		void    insertRows(std::string aName, std::vector<std::string> aNameList, std::vector<std::vector<Value>> aValuesList);

		void    dump(std::ostream& anOutput);

		uint64_t        getBlockCount();

		std::string getName() { return name; }

		std::optional<Entity> getEntity(std::string aName);

		uint64_t getRowIndex(std::string& aName);

	protected:

		//to encode and decode the block0 information
		void encode(Block& aBlock) override;
		void decode(Block& aBlock) override;

		std::string     name;
		std::fstream    stream;   //low level stream used by storage...
		bool            changed = false;  //might be helpful, or ignore if you prefer.

		// Hard coded version lengthen
		std::string     version = Config::getVersion();

		// Keeps tracks of the total number of entities we have
		uint64_t       entityCount = 0;

		Storage* storage;  //storage

		//std::map<std::string, Entity> entities;
		//std::map<std::string, Rows> tableRows; //storage of rows
		std::map<std::string, uint64_t> entityBlockMap; //block index of different tables

		std::map<std::string, std::shared_ptr<Entity>> entityMap; //map of entities stored in memory

		IndexMap indexMap;

		std::map<std::string, uint64_t> indexBlockMap;

		// Map to help print block headers into table
		inline static const std::map<char, std::string> typeStringMap{
		   {'D' , "Data"},
		   {'F', "Free"},
		   {'M' , "Meta"},
		   {'E', "Entity"},
		   {'I', "Index"},
		   {'U' , "Unknown"}
		};


		// View Cache
		std::unique_ptr<LRUCache<std::string, std::shared_ptr<TableView>>> viewCache; // "Select * from Table"
		std::unordered_map <std::string, bool> viewUpdated; // EntityName, updatedFlag
		//-------------------------------------------------------------
		// Helper Functions

		void    loadEntity(uint64_t aBlockIndex);
		void    loadEntityIndex();

		//TODO: Implement Fully
		inline void updateEntityMetaBlock();

		void verifyEntity(Entity& anEntity);

		std::vector<Row*> orderRows(std::vector<Row*>& aRowsList, std::vector<std::string>& anOrderList);

		// TODO: Make separate class
		void sortRows(std::string& aName, RowsPtr& aRowsList);

		void conditionalLoad(std::string& anEnitityName, Filters& aFilter, RowsPtr& aRowsList);
		void applyJoin(RowsPtr &aLeftRows, RowsPtr &aRightRows, RowsPtr &aOutputRows, SQLQuery& aQuery);
		void applyLimit(RowsPtr& aRowsList, std::ostream& anOutput, SQLQuery& aQuery);

		std::shared_ptr<Row> buildJoinedRow(std::shared_ptr<Row>& aFirstRow, Entity& aLeftEntity, std::shared_ptr<Row>& aSecondRow, Entity& aRightEntity);
	};


}
#endif /* Database_hpp */
