#include "Index.hpp"
#include <iostream>

namespace ECE141 {

	Index::Index(Storage* aStorage, CreateIndex, std::string aKeyName, uint64_t aBlockNum,
		DataTypes aType)
		: storage(aStorage), type(aType), blockNum(aBlockNum), name(aKeyName) {
		Block theBlock(BlockType::index_block, 0, NULL_BLOCK_IND, NULL_BLOCK_IND);

		encode(theBlock);
		storage->writeBlock(blockNum, theBlock);

		changed = false;
		entityId = 0;
	}

	Index::Index(Storage* aStorage, LoadIndex, std::string aKeyName, uint64_t aBlockNum,
		DataTypes aType)
		: storage(aStorage), type(aType), blockNum(aBlockNum), name(aKeyName) {
		
		Block theBlock;
		storage->readBlock(aBlockNum, theBlock);
		decode(theBlock);

		loadIndexMap();

		changed = false;
		entityId = 0;
	}

	static const size_t KV_DATA_LEN = sizeof(uint16_t) + sizeof(uint64_t) + 51; //50 is max VarCharLen + 1 for \0 for string

	static const size_t INDEX_LOOP_COUNT = kPayloadSize / KV_DATA_LEN;

	Index::~Index()
	{
		if (changed) {
			//Erase previous index blocks
			storage->releaseBlockUntil([&](const Block& aBlock, uint64_t anIndex) {
				return aBlock.header.nextPtr != NULL_BLOCK_IND;
				},
				storage->getNextPointer(blockNum));
			//Write Blocks to storage
			size_t theCounter = 0;
			uint64_t theCurrentBlockIndex = blockNum;

			Block theBlock(BlockType::index_block, entityId);
			//std::cout << "\n Indices Saved " << data.size() << "\n";
			for (auto& theKVPair : data) {
				//Data: uint16_t ValueType + Value as string + uint64_t blockNumber
				
				++theCounter;
				if (theCounter > INDEX_LOOP_COUNT) {
					theCurrentBlockIndex = storage->writeNewBlock(theBlock, theCurrentBlockIndex);
					theCounter = 0;
					theBlock.reset();
				}
				theBlock.writeToPayload(static_cast<uint16_t>(theKVPair.first.index()));
				theBlock.writeToPayload(BasicTypes::toString(theKVPair.first));
				theBlock.writeToPayload(theKVPair.second);
			}
			if (theBlock.header.storedSize != 0) { // Write final block
				theCurrentBlockIndex = storage->writeNewBlock(theBlock, theCurrentBlockIndex);
			}
		}
	}

	void Index::loadIndexMap()
	{
		size_t theCounter = 0;
		size_t theNextBlockInd = storage->getNextPointer(blockNum);

		uint16_t theValueIndex;
		uint64_t theEntityBlockIndex;
		std::string theValueStr;

		Block theBlock;

		size_t theTempCounter = 0; // Remove later

		while (NULL_BLOCK_IND != theNextBlockInd) {
			storage->readBlock(theNextBlockInd, theBlock);
			for (size_t theIndex = 0; theIndex < INDEX_LOOP_COUNT; ++theIndex) {
				theBlock.readFromPayload(theValueIndex);
				theBlock.readFromPayload(theValueStr);
				theBlock.readFromPayload(theEntityBlockIndex);
				
				if (theValueStr == "") { break; }
				// add to map
				++theTempCounter;
				data[BasicTypes::toValue(theValueStr, valueToType[theValueIndex])] = theEntityBlockIndex;
			}
			theNextBlockInd = theBlock.header.nextPtr;
		}
		//std::cout << "\n Indices Loaded " << data.size() << "\n";
	}

	void Index::encode(Block& aBlock)
	{
		//Store data map
		aBlock.writeToPayload(type);
		aBlock.writeToPayload(name);
		aBlock.writeToPayload(blockNum);
		aBlock.writeToPayload(entityId);
	}

	void Index::decode(Block& aBlock)
	{
		//Load Data
		aBlock.readFromPayload(type);
		aBlock.readFromPayload(name);
		aBlock.readFromPayload(blockNum);
		aBlock.readFromPayload(entityId);
	}
}



