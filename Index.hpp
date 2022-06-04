//
//  Index.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/2/21.
//

#ifndef Index_hpp
#define Index_hpp

#include <stdio.h>
#include <map>
#include <vector>

#include <functional>
#include "Storage.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Row.hpp"



namespace ECE141 {


	enum class IndexType { intKey = 0, strKey };

	//using IndexKey = std::variant<std::string, uint32_t>; // I'm not quite sure what it's supposed to be, a Value?
	using Rows = std::vector<Row>;

	using IndexVisitor =
		std::function<bool(const Value&, uint64_t)>;

	struct CreateIndex{};
	struct LoadIndex{};

	struct Index : public Storable, BlockIterator {
		//Index() : storage{ nullptr }, type(DataTypes::no_type), blockNum{ 0 }, changed{ false }, entityId{ 0 }{};

		Index(Storage* aStorage, CreateIndex,std::string aKeyName, uint64_t aBlockNum = 0,
			DataTypes aType = DataTypes::int_type);

		Index(Storage* aStorage, LoadIndex, std::string aKeyName, uint64_t aBlockNum = 0,
			DataTypes aType = DataTypes::int_type);

		Index(const Index& aCopy) {
			storage = aCopy.storage;
			blockNum = aCopy.blockNum;
			changed = aCopy.changed;
			entityId = aCopy.entityId;
			type = aCopy.type;
		}

		~Index();

		void loadIndexMap();

		class ValueProxy {
		public:
			Index& index;
			Value  key;
			DataTypes type;

			ValueProxy(Index& anIndex, int aKey)
				: index(anIndex), key(aKey), type(DataTypes::int_type) {}

			ValueProxy(Index& anIndex, const std::string& aKey)
				: index(anIndex), key(aKey), type(DataTypes::varchar_type) {}

			ValueProxy& operator= (uint32_t aValue) {
				index.setKeyValue(key, aValue);
				return *this;
			}

			operator IntOpt() { return index.valueAt(key); }
		}; //value proxy

		ValueProxy operator[](const std::string& aKey) {
			return ValueProxy(*this, aKey);
		}

		ValueProxy operator[](uint32_t aKey) {
			return ValueProxy(*this, aKey);
		}

		uint64_t getBlockNum() const { return blockNum; }
		Index& setBlockNum(uint32_t aBlockNum) {
			blockNum = aBlockNum;
			return *this;
		}

		bool    isChanged() { return changed; }
		Index& setChanged(bool aChanged) {
			changed = aChanged; return *this;
		}

		//StorageInfo getStorageInfo(size_t aSize) {
		//  //student complete...
		//}

		IntOpt valueAt(Value& aKey) {
			return exists(aKey) ? data[aKey] : (IntOpt)(std::nullopt);
		}

		bool setKeyValue(Value& aKey, uint32_t aValue) {
			data[aKey] = aValue;
			return changed = true; //side-effect intended!
		}

		StatusResult erase(const Value& aValue) {
			Value theValue = aValue;
			if (exists(theValue)) {
				data.erase(theValue);
			}

			return StatusResult{ Errors::noError };
		}


		StatusResult erase(const std::string& aKey) {

			Value theValue = BasicTypes::toValue(aKey, DataTypes::varchar_type);
			if (exists(theValue)) {
				data.erase(theValue);
			}

			return StatusResult{ Errors::noError };
		}

		StatusResult erase(int aKey) {

			Value theValue = aKey;
			if (exists(theValue)) {
				data.erase(theValue);
			}
			return StatusResult{ Errors::noError };
		}

		size_t getSize() { return data.size(); }

		bool exists(Value& aKey) {
			return data.count(aKey);
		}

		void encode(Block& aBlock) override;

		void decode(Block& aBlock) override;

		// Default encode and decode don't really work very well if the number of indices overflow a single block 
		// Might want to use alternate interface
		//void encodeData() { // no overload
		//	Block theBlock;
		//	uint64_t thePrevBlockInd = blockNum;

		//	for (auto& theDataItr : data) {
		//		// while block is not full

		//		thePrevBlockInd = storage->writeNewBlock(theBlock, thePrevBlockInd);
		//	}
		//}

		//void decodeData() {
		//	//student implement...
		//	//return StatusResult{ Errors::noError };
		//}

		bool each(BlockVisitor aVisitor) {
			Block theBlock;
			for (auto thePair : data) {
				storage->readBlock(thePair.second, theBlock);
				if (!aVisitor(theBlock, thePair.second)) { return false; }
			}
			return true;
		}

		//visit index values (key, value)...
		bool eachKV(IndexVisitor aCall) {
			for (auto& thePair : data) {
				if (!aCall(thePair.first, thePair.second)) {
					return false;
				}
			}
			return true;
		}

		bool insertKV(Value& aValue, uint64_t aBlockCount) {
			if (!exists(aValue)) {
				data[aValue] = aBlockCount;
				changed = true;
				return true;
			}
			return false;
		}

		std::string getKeyName() { return name; }

	protected:

		Storage*                      storage;
		std::map<Value, uint64_t>     data;
		DataTypes                     type;
		std::string                   name;
		bool                          changed;
		uint64_t                      blockNum; //where index storage begins
		uint64_t                      entityId;
	}; //index

	using IndexMap = std::map<std::string, std::shared_ptr<Index>>;

}


#endif /* Index_hpp */
