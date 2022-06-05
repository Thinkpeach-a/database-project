//
//  Storage.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <stack>
#include <optional>
#include <functional>
#include "BlockIO.hpp"
#include "Errors.hpp"

namespace ECE141 {

	struct CreateDB {}; //tags for db-open modes...
	struct OpenDB {};

	class Storable {
	public:
		virtual void  encode(Block& aBlock) = 0;
		virtual void  decode(Block& aBlock) = 0;

		virtual ~Storable() = default;
	};

	// Visitor Types
	using BlockVisitor = std::function<bool(const Block&, uint64_t)>;
	using BlockIterator = std::function<uint64_t(Block&, uint64_t)>;

	// USE: Our storage manager class...
	class Storage : public BlockIO {
	public:

		//reference to the file to be input
		Storage(std::fstream& aStream, OpenDB);
		Storage(std::fstream& aStream, CreateDB);
		~Storage();


		// Loop through all blocks consecutively in in-file order
		bool each(const BlockVisitor& aVisitor);

		// Iterates until visitor returns out of bounds value
		bool iterate(const BlockIterator& aVisitor, uint64_t aStart);

		uint64_t getPrevPointer(uint64_t aBlockIndex);

		uint64_t getNextPointer(uint64_t aBlockIndex);

		uint64_t writeNewBlock(Block& aBlock, uint64_t aPrevBlockIndex, uint64_t aCurrentIndex = NULL_BLOCK_IND);

		void removeBlock(uint64_t aBlockIndex);

		/// <summary>
		/// Continues to release blocks until aVisitor returns false,
		/// returns number of blocks released.
		/// </summary>
		/// <param name="aVisitor"></param>
		/// <param name="aStartIndex"></param>
		/// <param name="anInclusive"></param>
		/// <returns></returns>
		uint64_t releaseBlockUntil(const BlockVisitor& aVisitor, uint64_t aStartIndex);

	protected:

		uint64_t     getFreeBlockIndex();

		uint64_t	 getFileBlockCount();

		uint64_t     getUsedBlockCount() { return usedBlockCount; }

		void writeMetaBlock(Block& aBlock);

		void updateMetaBlock();



		//-----------------------------------------------------------------------
		// Primitives

		void updatePrevPointer(uint64_t aBlockIndex, uint64_t aPrevPtr);

		void updateNextPointer(uint64_t aBlockIndex, uint64_t aNextPtr);

		void updateBlockType(uint64_t aBlockIndex, BlockType aBlockType);

		//-----------------------------------------------------------------------
		// Data
		uint64_t usedBlockCount;
		uint64_t totalBlockCount;
		uint64_t unusedBlockPtr = NULL_BLOCK_IND;

		friend class Database;
	};

}


#endif /* Storage_hpp */
