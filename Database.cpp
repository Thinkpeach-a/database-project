//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <filesystem>
#include <iterator>

#include "Database.hpp"
#include "TableView.hpp"
#include "BlockIO.hpp"
#include "TableView.hpp"
#include "BasicTypes.hpp"
#include "Application.hpp"
#include "Helpers.hpp"
#include "TableLoader.hpp"



namespace ECE141 {
	// Constants
	static const std::vector<std::string>         entityHeaderList{ "Field","Type","Null","Key","Default","Extra" };
	static const std::map<DataTypes, std::string> dataTypesWords{ {DataTypes::no_type,"no_type"},{DataTypes::bool_type,"bool"},{DataTypes::datetime_type,"dataTime"},{DataTypes::float_type,"float"},{DataTypes::int_type,"int"},{DataTypes::varchar_type,"varchar"} };
	static const std::map<bool, std::string>      boolToWords{ {false,"NO"}, {true,"YES"} };
	static const std::vector<uint64_t>              entityWidthList{ 15,15,15,15,15,20 };

	static const std::vector<std::string>    dbDumpHeaders{ "Count", "Type","Id", "Prev", "Next" };
	static const std::vector<uint64_t>         dbDumpWidths{ 10, 15, 20, 20, 20 };

	// Create new database and places new file in path specified by Config.
	Database::Database(const std::string aName, CreateDB)
		: name(aName), changed(true) {
		std::string thePath = Config::getDBPath(name);

		//create for read/write
		stream.open(thePath, std::ios::trunc | std::ios::out |
			std::ios::in | std::ios::binary); // Storage uses binary read write

		storage = new Storage(stream, CreateDB());
		if (!storage) { throw Errors::databaseCreationError; }
		// Update meta block with basic information
		Block theBlock;
		encode(theBlock);
		storage->writeMetaBlock(theBlock);

		viewCache = std::make_unique<LRUCache<std::string, TableView>>(Config::getCacheSize(CacheType::view));
	}

	// Load existing database from file, based on path specified in Config.
	Database::Database(const std::string aName, OpenDB)
		: name(aName), changed(false) {

		std::string thePath = Config::getDBPath(name);
		if (!std::filesystem::exists(thePath)) {
			std::cout << "file not exist";
		}
		else {
			stream.open(thePath, std::ios::out | std::ios::in | std::ios::binary);

			storage = new Storage(stream, OpenDB());
			if (!storage) { throw Errors::databaseLoadError; }

			//initialize data:
			Block theBlock;
			storage->readBlock(0, theBlock);
			decode(theBlock);

			viewCache = std::make_unique<LRUCache<std::string, TableView>>(Config::getCacheSize(CacheType::view));
			loadEntityIndex();
		}
	}

	Database::~Database() {

		Block theBlock;
		// Save entities
		for (auto& theEntityPair : entityMap) {

			uint64_t theBlockIndex = entityBlockMap[theEntityPair.first];
			storage->readBlock(theBlockIndex, theBlock);
			theBlock.reset();
			theEntityPair.second->encode(theBlock);
			storage->writeBlock(theBlockIndex, theBlock);
		}


		// Free Indexes before storage goes out of scope
		for (auto& theIndexPair : indexMap) {
			theIndexPair.second.reset();
		}
		delete storage;
	}

	// Removes a table from the database by removing the table and freeing all it's rows
	// from Storage and the Database's memory.
	void Database::dropTable(std::string aTableName, std::ostream& aStream) {
		if (entityBlockMap.count(aTableName) == 0) { throw Errors::unknownTable; }
		uint64_t theRemovedCount =
			storage->releaseBlockUntil([&](const Block& aBlock, uint64_t aCount) {
			return true; // Tables are singly linked list of blocks, release until the end.
				}, entityBlockMap[aTableName]);

		aStream << Helpers::QueryOk(theRemovedCount, Config::getTimer().elapsed());
		// Stored Values need to be updated
		entityMap.erase(aTableName);
		entityBlockMap.erase(aTableName);

		indexMap.erase(aTableName);

		size_t theIndexBlock = indexBlockMap[aTableName];
		indexBlockMap.erase(aTableName);

		storage->releaseBlockUntil([](const Block&, uint64_t) {return true; }, theIndexBlock);

		viewUpdated.erase(aTableName);
		updateEntityMetaBlock();
	}

	//Create a table from anEntity and write it in the file;
	void Database::createTable(Entity& anEntity) {
		Block theBlock;

		verifyEntity(anEntity);

		std::string theName = anEntity.getName();

		// Write the block
		anEntity.setEntityIndex(++entityCount);

		anEntity.encode(theBlock);
		// Figures out pointers
		uint64_t theIndex = storage->writeNewBlock(theBlock, NULL_BLOCK_IND);

		// Add data to memory
		entityBlockMap[theName] = theIndex;
		entityMap[theName] = std::make_shared<Entity>(anEntity);
		viewUpdated[theName] = true;


		//Construct Index
		uint64_t theIndexBlockNum = storage->getFreeBlockIndex();
		auto     thePrimaryKey = anEntity.getPrimaryKey();

		indexMap[theName] = std::make_shared<Index>(storage, CreateIndex(), thePrimaryKey->getName(), theIndexBlockNum, thePrimaryKey->getType());
		indexBlockMap[theName] = theIndexBlockNum;

		updateEntityMetaBlock();
	}

	//Helper to load a single entity from a Entity block into the database.
	void Database::loadEntity(uint64_t aBlockIndex) {
		Entity theEntity;
		Block theBlock;

		storage->readBlock(aBlockIndex, theBlock);
		if (theBlock.header.type == static_cast<char>(BlockType::entity_block)) {
			theEntity.decode(theBlock);
		}

		entityMap[theEntity.getName()] = std::make_shared<Entity>(theEntity);
	}

	void Database::loadEntityIndex()
	{
		auto theIndexIter = indexBlockMap.begin();

		Entity theEntity;

		// Create 2 Blocks so if read error occurs, does not affect
		// the rest of the progress
		Block  theEntityBlock;
		Block  theIndexBlock;
		std::string theEntityName;

		for (auto& theEntityPair : entityBlockMap) {
			theEntityBlock.reset();
			theIndexBlock.reset();

			theEntityName = theEntityPair.first;

			storage->readBlock(theEntityPair.second, theEntityBlock);

			theEntity.decode(theEntityBlock);


			entityMap[theEntityName] = std::make_shared<Entity>(theEntity);
			indexMap[theEntityName] = std::make_shared<Index>(storage, LoadIndex(), theEntity.getPrimaryKey()->getName(), indexBlockMap[theEntityName]);
			// caching map
			viewUpdated[theEntityName] = true;
		}

	}

	std::optional<Entity> Database::getEntity(std::string aName)
	{
		if (entityMap.count(aName)) {
			return *entityMap[aName];
		}
		return std::nullopt;
	}

	uint64_t Database::getRowIndex(std::string& aName)
	{
		if (entityBlockMap.count(aName) != 0) {
			return entityBlockMap[aName];
		}
		return NULL_BLOCK_IND;
	}

	//encode entitynames and their nextposition:
	void Database::encode(Block& aBlock) {

		aBlock.writeToPayload(version);
		aBlock.writeToPayload(entityCount);
		// Write keys and values into payload
		aBlock.writeToPayload(static_cast<uint64_t>(entityBlockMap.size()));

		auto theIndexMapIter = indexBlockMap.begin();
		for (auto const& pair : entityBlockMap) {
			aBlock.writeToPayload(pair.first);
			aBlock.writeToPayload(pair.second);
			aBlock.writeToPayload(theIndexMapIter->second);
		}


	}

	//decode the block0 into string and index
	void Database::decode(Block& aBlock) {
		uint64_t theTableCount;
		std::string theKey;
		uint64_t    theEntityValue;
		uint64_t    theIndexValue;


		aBlock.readFromPayload(version);
		aBlock.readFromPayload(entityCount);

		// Read keys and values from payload into table
		aBlock.readFromPayload(theTableCount);
		for (auto theIndex = 0; theIndex < theTableCount; ++theIndex) {
			aBlock.readFromPayload(theKey);
			aBlock.readFromPayload(theEntityValue);
			aBlock.readFromPayload(theIndexValue);
			entityBlockMap[theKey] = theEntityValue;
			indexBlockMap[theKey] = theIndexValue;
		}
	}

	inline void Database::updateEntityMetaBlock() {
		Block theBlock;
		encode(theBlock);
		storage->writeMetaBlock(theBlock);
	}

	inline void Database::verifyEntity(Entity& anEntity) {
		// Check Name is Free
		if (entityMap.count(anEntity.getName())) { throw Errors::tableExists; }

		// Check Primary Key exists and is unique
		size_t thePrimaryKeyCount = 0;
		for (auto& theAttribute : anEntity.attributes) {
			if (theAttribute.isPrimaryKey()) { ++thePrimaryKeyCount; }
		}
		if (thePrimaryKeyCount != 1) { throw Errors::primaryKeyRequired; }
		//TODO: is there more to verify?
	}

	std::vector<Row*> Database::orderRows(std::vector<Row*>& aRowsList, std::vector<std::string>& anOrderList) {
		// TODO: Should recursively orderRows until theOrderlist is empty

		// Pull out a list of attributes
		// Sort the list of Values, and the RowsList in the same order

		if (anOrderList.size() == 0) {
			return aRowsList;
		}
		std::string theSortName = anOrderList[0];

		std::sort(aRowsList.begin(), aRowsList.end(), [&](Row* aFirstRow, Row* aSecondRow) {
			return BasicTypes::toString(aFirstRow->getData()[theSortName]) <
				BasicTypes::toString(aSecondRow->getData()[theSortName]);
			});

		// Make copy to allow for swapping, also allow caching later on

		return aRowsList;
	}

	void Database::sortRows(std::string& aName, RowsPtr& aRowsList)
	{
		//TODO: Do Recursion
		std::sort(aRowsList.begin(), aRowsList.end(),
			[&](std::shared_ptr<Row> aFirstRow, std::shared_ptr<Row> aSecondRow) {
				return BasicTypes::toString(aFirstRow->getData()[aName]) <
					BasicTypes::toString(aSecondRow->getData()[aName]);
			});
	}

	inline void Database::conditionalLoad(std::string& anEnitityName, Filters& aFilter, RowsPtr& aRowsList)
	{
		//std::cout << "Trying to load Entity: " << anEnitityName << "\n";
		size_t theEntityIndex = getRowIndex(anEnitityName);
		if (theEntityIndex == NULL_BLOCK_IND) { throw Errors::unknownEntity; }

		Row theTempRow;
		Rows theRowsList;

		//Check if if conditioned on primary key
		//TODO: Wait for updated Filters class
		if (false) {
			// TODO: waiting for more Filter ultilities to help with primary key matching/loading
		}
		else {
			size_t theRowStart = storage->getNextPointer(theEntityIndex);
			storage->iterate([&](Block& aBlock, uint64_t anIndex) {
				theTempRow.decode(aBlock);

				//TODO: Wait for update filter interface with scoping
				if (aFilter.matches(theTempRow.getData())) {
					aRowsList.push_back(std::make_shared<Row>(theTempRow));
				}

				return aBlock.header.nextPtr;
				}, theRowStart);
		}
	}

	void Database::applyJoin(RowsPtr& aLeftRows, RowsPtr& aRightRows, RowsPtr& aOutputRows, SQLQuery& aQuery)
	{
		std::vector<std::string> theTableHeader;
		std::string theEntityName;

		std::shared_ptr<Row> theNullPtr;

		Entity& theLeftEntity = *entityMap[aQuery.join.leftTable];
		Entity& theRightEntity = *entityMap[aQuery.join.rightTable];

		if (aLeftRows.size() == 0) { return; }
		// Sort both tables according to join column
		sortRows(aQuery.join.leftValue, aLeftRows);
		sortRows(aQuery.join.rightValue, aRightRows);

		// Iterate through left table, if right table matches, iterate right, if it stops matching, iterate left
		auto theRightTableIter = aRightRows.begin();
		for (auto& theLeftRow : aLeftRows) {
			size_t theCount = 0;
			while (theRightTableIter != aRightRows.end()) {
				Value theLeftValue = theLeftRow->getData()[aQuery.join.leftValue];
				Value theRightValue = (*theRightTableIter)->getData()[aQuery.join.rightValue];

				if (theLeftValue > theRightValue) {
					++theRightTableIter;
				}
				else if (theLeftValue == theRightValue) {
					aOutputRows.push_back(buildJoinedRow(theLeftRow, theLeftEntity, *theRightTableIter, theRightEntity));
					++theRightTableIter;
					++theCount;
				}
				else {
					break;
				}
			}
			if (theCount == 0) {
				//Insert Empty Row if it doesn't work
				aOutputRows.push_back(buildJoinedRow(theLeftRow, theLeftEntity, theNullPtr, theRightEntity));
			}
		}
	}

	void Database::applyLimit(RowsPtr& aRowsList, std::ostream& anOutput, SQLQuery& aQuery)
	{
		// Table Headers
		if (aQuery.columnNames.size() < 1) { throw Errors::syntaxError; }

		std::vector<std::string> theRowHeader;
		std::vector<uint64_t> theHeaderWidths;

		if ("*" == aQuery.columnNames[0]) {
			auto& theRowData = aRowsList[0]->getData();

			for (auto& theRowAttribute : theRowData) {
				theRowHeader.push_back(theRowAttribute.first);
			}
		}
		else {
			theRowHeader = aQuery.columnNames;
		}

		size_t theHeaderSize = theRowHeader.size();
		for (size_t theCount = 0; theCount < theHeaderSize; ++theCount) {
			theHeaderWidths.push_back(20);
		}

		TableView theTable(theRowHeader, theHeaderWidths);

		std::vector<Value> theColumnData;
		// Insert Rows into the Table

		// Apply Limit bounds
		size_t theRowsSize = aRowsList.size();
		size_t theLimit = (theRowsSize < aQuery.limitCount || 0 == aQuery.limitCount)
			? aRowsList.size() : aQuery.limitCount;

		for (auto theIndex = aQuery.limitStartIndex; theIndex < theLimit; ++theIndex) {
			auto& theTempRow = aRowsList[theIndex];
			theColumnData.clear();
			auto& theRowData = theTempRow->getData(); //???
			for (auto& theRowName : theRowHeader) {
				theColumnData.push_back(theRowData[theRowName]);
			}
			theTable.insertRow(theColumnData);
		}
		theTable.show(anOutput);
		anOutput << Helpers::rowsInSet(theLimit, Config::getTimer().elapsed());
	}

	std::shared_ptr<Row> Database::buildJoinedRow(std::shared_ptr<Row>& aFirstRow, Entity& aFirstEntity,
		std::shared_ptr<Row>& aSecondRow, Entity& aSecondEntity)
	{
		Row aRow;
		auto& theFirstData = aFirstRow->getData();
		auto& theSecondData = aSecondRow->getData();

		for (auto& theAttribute : aFirstEntity.getAttributes()) {
			aRow.set(theAttribute.getName(), aFirstRow->getData()[theAttribute.getName()]);
		}
		if (aSecondRow) {
			for (auto& theAttribute : aSecondEntity.getAttributes()) {
				aRow.set(theAttribute.getName(), aSecondRow->getData()[theAttribute.getName()]);
			}
		}
		else {
			for (auto& theAttribute : aSecondEntity.getAttributes()) {
				aRow.set(theAttribute.getName(), NullType());
			}
		}

		return std::make_shared<Row>(aRow);
	}




	void Database::showTables(std::ostream& aStream) {
		TableView theTable({ "Tables in " + name }, { 24 });
		for (auto const& pair : entityBlockMap) {
			theTable.insertRow(pair.first);
		}

		theTable.insertFooter(Helpers::rowsInSet(entityBlockMap.size(), Config::getTimer().elapsed()));
		theTable.show(aStream);
	}


	void Database::describeTable(std::string anEntityName, std::ostream& anOuptut) {
		//"Field","Type","Null","Key","Default","Extra"
		// Build Table
		TableView theTable{ entityHeaderList, entityWidthList };
		std::string theType;
		uint64_t theSize;

		if (entityMap.count(anEntityName) == 0) { throw Errors::unknownTable; }

		Entity& theEntity = *entityMap[anEntityName];
		for (auto theIter = theEntity.getAttributes().begin(); theIter != theEntity.getAttributes().end(); ++theIter) {
			theType = dataTypesWords.at(theIter->getType());
			theSize = theIter->getSize();

			if (0 != theSize) { theType += "(" + std::to_string(theSize) + ")"; } // if varchar

			// Variadically insert values
			theTable.insertRow(theIter->getName(),
				theType,
				boolToWords.at(theIter->isNullable()),
				boolToWords.at(theIter->isPrimaryKey()),
				BasicTypes::toString(theIter->getDefault()),
				theIter->isAutoIncrement() ? "Auto Increment" : "");
		}
		theTable.insertFooter(Helpers::rowsInSet(theEntity.getAttributes().size(), Config::getTimer().elapsed()));
		theTable.show(anOuptut);
	}

	void Database::selectFromTable(SQLQuery& aQuery, std::ostream& anOutput)
	{
		if (!(entityMap.count(aQuery.EntityName) > 0)) { throw Errors::unknownEntity; }

		RowsPtr theLeftRows;
		RowsPtr theRightRows;
		RowsPtr theOutputRows;

		if (aQuery.hasJoin) {
			if (!(entityMap.count(aQuery.join.rightTable) > 0)) { throw Errors::unknownEntity; }
			// if view is up to date and in cache
			if (viewUpdated[aQuery.join.leftTable] && viewUpdated[aQuery.join.rightTable]) {
				if (viewCache.contains());
			}

			

			conditionalLoad(aQuery.join.leftTable, aQuery.whereFilter, theLeftRows);
			conditionalLoad(aQuery.join.rightTable, aQuery.whereFilter, theRightRows);

			applyJoin(theLeftRows, theRightRows, theOutputRows, aQuery);
		}
		else {
			conditionalLoad(aQuery.EntityName, aQuery.whereFilter, theOutputRows);
		}

		// Order by
		if (aQuery.orderNames.size() != 0) {
			sortRows(aQuery.orderNames[0], theOutputRows);
		}
		// Apply Limit
		applyLimit(theOutputRows, anOutput, aQuery);
	}

	void Database::updateRows(SQLQuery& aQuery, KeyValues aReplaceMap, std::ostream& anOuptut)
	{
		uint64_t theRowCount = 0;

		if (entityBlockMap.count(aQuery.EntityName) == 0) { throw Errors::unknownTable; }

		Row theRow;
		storage->iterate([&](Block& aBlock, uint64_t anIndex) {

			theRow.decode(aBlock);
			uint64_t theNextPtr = aBlock.header.nextPtr;

			if (aQuery.whereFilter.matches(theRow.getData()) || !aQuery.hasWhere) {
				//TODO: Factor out has where, but it's quite readable, so I'll keep it for now

				for (auto& thePairs : aReplaceMap) {
					theRow.set(thePairs.first, thePairs.second);
				}
				theRow.encode(aBlock.reset());
				storage->writeBlock(anIndex, aBlock);
				++theRowCount;
			}

			return theNextPtr;

			}, storage->getNextPointer(entityBlockMap[aQuery.EntityName])); // Start at the first row

		viewUpdated[aQuery.EntityName] = false;

		anOuptut << Helpers::QueryOk(theRowCount, Config::getTimer().elapsed());
	}

	void Database::deleteRows(SQLQuery& aQuery, std::ostream& anOuptut)
	{
		uint64_t theRowCount = 0;

		if (entityBlockMap.count(aQuery.EntityName) == 0) { throw Errors::unknownTable; }
		std::string thePrimaryKey = entityMap[aQuery.EntityName]->getPrimaryKey()->getName();

		Row theRow;
		storage->iterate([&](Block& aBlock, uint64_t anIndex) {

			theRow.decode(aBlock);
			uint64_t theNextPtr = aBlock.header.nextPtr;

			if (aQuery.whereFilter.matches(theRow.getData()) || !aQuery.hasWhere) {
				//TODO: Factor out has where, but it's quite readable, so I'll keep it for now
				storage->removeBlock(anIndex);
				indexMap[aQuery.EntityName]->erase(theRow.getData()[thePrimaryKey]);

				++theRowCount;
			}

			return theNextPtr;

			}, storage->getNextPointer(entityBlockMap[aQuery.EntityName])); // Start at the first row

		viewUpdated[aQuery.EntityName] = false;

		anOuptut << Helpers::QueryOk(theRowCount, Config::getTimer().elapsed());
	}

	void Database::showIndices(std::string aTableName, std::string aKeyName, std::ostream& anOutput)
	{
		TableView theTable({ aKeyName, "Block Number" }, { 20, 20 });

		uint64_t theCount = 0;
		indexMap[aTableName]->eachKV([&](const Value& aValue, uint64_t theBlockIndex) {
			theTable.insertRow(BasicTypes::toString(aValue), theBlockIndex);
			++theCount;
			return true;
			});
		theTable.show(anOutput);
		anOutput << Helpers::rowsInSet(theCount, Config::getTimer().elapsed());

	}

	void Database::showIndexes(std::ostream& anOutput)
	{
		TableView theTable({ "table", "fields(s)" }, { 20, 20 });
		size_t theCount = 0;
		for (auto& theIndex : indexMap) {
			theTable.insertRow(theIndex.first, theIndex.second->getKeyName());
			++theCount;
		}
		theTable.show(anOutput);
		anOutput << Helpers::rowsInSet(theCount, Config::getTimer().elapsed());

	}

	void Database::insertRows(std::string aName, std::vector<std::string> aNameList,
		std::vector<std::vector<Value>> aValuesList) {
		if (entityMap.count(aName) > 0) {
			RowsOpt theRows;
			uint64_t theBlockIndex;
			Entity& theEntity = *entityMap[aName];
			Index& theIndex = *indexMap[aName];
			theEntity.buildRows(aNameList, aValuesList, theRows);

			for (auto& aRow : theRows) {
				if (aRow) {
					//write row as block
					// TODO: Refactor block for reset capability
					Block theBlock; // Need to make a new block for writing storage
					aRow.value().entityId = theEntity.getEntityIndex();
					aRow.value().encode(theBlock);
					theBlockIndex = storage->writeNewBlock(theBlock, entityBlockMap[aName]);

					// Add to index
					theIndex.insertKV(aRow.value().getData()[theEntity.getPrimaryKey()->getName()], theBlockIndex);
				}
				else {
					throw Errors::invalidArguments;
				}
			}
			viewUpdated[theEntity.getName()] = false;
		}
	}

	// USE: Call this to dump the db for debug purposes...
	void Database::dump(std::ostream& anOutput) {
		TableView theTable(dbDumpHeaders, dbDumpWidths);
		Block theBlock;

		storage->each([&](const Block& aBlock, uint64_t anIndex) {
			theTable.insertRow(anIndex,
				typeStringMap.at(aBlock.header.type),
				aBlock.header.index,
				aBlock.header.prevPtr,
				aBlock.header.nextPtr);
			return true;
			});

		theTable.show(anOutput);
	}

	uint64_t Database::getBlockCount() {
		return storage->getUsedBlockCount();
	}
}
