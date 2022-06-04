//
//  Storage.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//


#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include "Storage.hpp"
#include "Config.hpp"

#include "Helpers.hpp"

namespace ECE141 {


	// USE: ctor ---------------------------------------
	Storage::Storage(std::fstream& aStream, OpenDB) : BlockIO(aStream) {
		// Load data
		totalBlockCount = getFileBlockCount();

		// Load data from header
		aStream.seekg(0, std::ios::beg);
		Block theBlock;
		readBlock(0, theBlock);

		// Metablock will be used to store these values.
		usedBlockCount = theBlock.header.prevPtr;
		unusedBlockPtr = theBlock.header.nextPtr;
		updateMetaBlock();
	}

	Storage::Storage(std::fstream& aStream, CreateDB) : BlockIO(aStream) {
		// Load data
		Block theBlock;

		totalBlockCount = 1;
		usedBlockCount = 0;
		unusedBlockPtr = NULL_BLOCK_IND;

		writeMetaBlock(theBlock);
	}

	// USE: dtor ---------------------------------------
	Storage::~Storage() {
		updateMetaBlock();
	}

	bool Storage::each(const BlockVisitor& aVisitor) {
		Block theBlock;
		for (uint64_t i = 0; i < totalBlockCount; i++) {
			readBlock(i, theBlock);
			if (!aVisitor(theBlock, i)) break;
		}
		return true;
	}

	bool Storage::iterate(const BlockIterator& aVisitor, uint64_t aStart) {
		while (aStart < totalBlockCount && aStart != NULL_BLOCK_IND) {
			Block theBlock;	

			readBlock(aStart, theBlock);
			aStart = aVisitor(theBlock, aStart);

		}
		return true;
	}

	uint64_t Storage::getFreeBlockIndex() {
		if (unusedBlockPtr != NULL_BLOCK_IND) {
			uint64_t theFreeBlock = unusedBlockPtr;

			Block theBlock;
			readBlock(theFreeBlock, theBlock);

			// update free block ptr
			unusedBlockPtr = theBlock.header.nextPtr;

			updateMetaBlock();
			return theFreeBlock;
		}
		else {
			// gives index of next empty part of the file,
			// increments blockCount afterwards
			return totalBlockCount++;
		}
	}

	uint64_t Storage::getFileBlockCount() {
		// Save current pos
		uint64_t theCurrentPos = stream.tellg();

		// Find start count
		stream.seekg(0, std::ios::beg);
		uint64_t begin = stream.tellg();

		// Find end count
		stream.seekg(0, std::ios::end);
		uint64_t end = stream.tellg();

		return static_cast<uint64_t>(std::ceil((end - begin) / kBlockSize));
	}

	void Storage::writeMetaBlock(Block& aBlock) {
		StatusResult theResult;
		aBlock.updateType(BlockType::meta_block);
		aBlock.header.prevPtr = usedBlockCount;
		aBlock.header.nextPtr = unusedBlockPtr;
		writeBlock(0, aBlock);
	}

	void Storage::updateMetaBlock() {
		StatusResult theResult;

		Block theBlock;
		readBlock(0, theBlock);

		theBlock.header.prevPtr = usedBlockCount;
		theBlock.header.nextPtr = unusedBlockPtr;
		writeBlock(0, theBlock);
	}

	uint64_t Storage::writeNewBlock(Block& aBlock, uint64_t aPrevBlockIndex, uint64_t aCurrentIndex) {
		if (NULL_BLOCK_IND == aCurrentIndex){ aCurrentIndex = getFreeBlockIndex(); }
		
		Block thePrevBlock;
		// Connect prev to current and current to prev.next

		if (aPrevBlockIndex != NULL_BLOCK_IND && aCurrentIndex < totalBlockCount) {

			Block thePrevBlock;
			readBlock(aPrevBlockIndex, thePrevBlock);
			uint64_t theNextBlockIndex = thePrevBlock.header.nextPtr;

			// Update prev pointer of next item in chain if it exists
			if (theNextBlockIndex != NULL_BLOCK_IND) {
				updatePrevPointer(theNextBlockIndex, aCurrentIndex);
			}
			// Update next ptr of prev block
			thePrevBlock.header.nextPtr = aCurrentIndex;
			writeBlock(aPrevBlockIndex, thePrevBlock);

			// Update current block
			aBlock.header.prevPtr = aPrevBlockIndex;
			aBlock.header.nextPtr = theNextBlockIndex;
			writeBlock(aCurrentIndex, aBlock);

			++usedBlockCount;
			updateMetaBlock();
		}
		// Prev block is NULLPTR
		else if (aPrevBlockIndex == NULL_BLOCK_IND) {
			aBlock.header.prevPtr = NULL_BLOCK_IND;
			aBlock.header.nextPtr = NULL_BLOCK_IND;
			writeBlock(aCurrentIndex, aBlock);

			++usedBlockCount;
		}
		return aCurrentIndex;
	}

	void Storage::removeBlock(uint64_t aBlockIndex) {
		Block theBlock;

		// Guaranteed to have a prevPtr, but nextPtr coult be null
		readBlock(aBlockIndex, theBlock);

		// Update nextPtr of prev Block
		updateNextPointer(theBlock.header.prevPtr, theBlock.header.nextPtr);
		if (theBlock.header.nextPtr != NULL_BLOCK_IND) {
			// Update prev ptr of next block
			updatePrevPointer(theBlock.header.nextPtr, theBlock.header.prevPtr);
		}

		// Insert the Current block into the null list
		theBlock.header.type = static_cast<char>(BlockType::free_block);
		writeNewBlock(theBlock, unusedBlockPtr, aBlockIndex);
		unusedBlockPtr = aBlockIndex;

		--usedBlockCount;
	}

	void Storage::updatePrevPointer(uint64_t aBlockIndex, uint64_t aPrevPtr) {

		Block thePrevBlock;

		readBlock(aBlockIndex, thePrevBlock);

		thePrevBlock.header.prevPtr = aPrevPtr;
		writeBlock(aBlockIndex, thePrevBlock);

	}

	void Storage::updateNextPointer(uint64_t aBlockIndex, uint64_t aNextPtr) {
		Block thePrevBlock;

		readBlock(aBlockIndex, thePrevBlock);

		thePrevBlock.header.nextPtr = aNextPtr;
		writeBlock(aBlockIndex, thePrevBlock);
	}

	uint64_t Storage::releaseBlockUntil(const BlockVisitor& aVisitor, uint64_t aStartIndex) {
		Block theBlock;

		uint64_t theReleasedCount = 0;

		if (NULL_BLOCK_IND == aStartIndex) { return 0; }

		uint64_t thePrevIndex = getPrevPointer(aStartIndex);
		uint64_t theCurrentIndex = aStartIndex;

		readBlock(aStartIndex, theBlock);


		while (theCurrentIndex < totalBlockCount && theCurrentIndex != NULL_BLOCK_IND) {
			// TODO: Possible Bug
			readBlock(theCurrentIndex, theBlock);
			if (aVisitor(theBlock, theCurrentIndex)) {
				updateBlockType(theCurrentIndex, BlockType::free_block);
				theCurrentIndex = theBlock.header.nextPtr;
				++theReleasedCount;
			}
			else { break; }
		}

		if (theReleasedCount > 0) {

			// Connect Surrounding Blocks
			if (theCurrentIndex != NULL_BLOCK_IND) { //prevPtr.next = theCurrentIndex
				if (thePrevIndex != NULL_BLOCK_IND) { //don't set
					updateNextPointer(thePrevIndex, theCurrentIndex);
					updatePrevPointer(theCurrentIndex, thePrevIndex);
				}
				else {
					updatePrevPointer(theCurrentIndex, NULL_BLOCK_IND);
				}
			}
			else { // prevPtr.Next = NULL
				if (thePrevIndex != NULL_BLOCK_IND) {
					updateNextPointer(thePrevIndex, NULL_BLOCK_IND);
				}// else nothing to link
			}

			// insert list to unused block list
			if (NULL_BLOCK_IND == unusedBlockPtr) {
				if (theCurrentIndex != NULL_BLOCK_IND) { // unused == NULL, current != NULL
					updateNextPointer(getPrevPointer(theCurrentIndex), NULL_BLOCK_IND);
				}
			}
			else { // 
				if (theCurrentIndex != NULL_BLOCK_IND) {
					updateNextPointer(getPrevPointer(theCurrentIndex), unusedBlockPtr); // could refactor
					updatePrevPointer(unusedBlockPtr, getPrevPointer(theCurrentIndex));
				}
				else { //unused not NULL, currentIndex is NULL, need block data
					updateNextPointer(getNextPointer(theBlock.header.prevPtr), unusedBlockPtr);
					updatePrevPointer(unusedBlockPtr, getNextPointer(theBlock.header.prevPtr)); // refactor into a join block function
				}
			}
			unusedBlockPtr = aStartIndex;
		}
		return theReleasedCount;
	}

	uint64_t Storage::getPrevPointer(uint64_t aBlockIndex) {
		Block theBlock;
		readBlock(aBlockIndex, theBlock);
		return theBlock.header.prevPtr;
	}

	uint64_t Storage::getNextPointer(uint64_t aBlockIndex) {
		Block theBlock;
		readBlock(aBlockIndex, theBlock);
		return theBlock.header.nextPtr;
	}

	void Storage::updateBlockType(uint64_t aBlockIndex, BlockType aBlockType) {
		StatusResult theResult;
		Block theBlock;
		readBlock(aBlockIndex, theBlock);

		theBlock.header.type = static_cast<char>(aBlockType);
		writeBlock(aBlockIndex, theBlock);
	}
}

