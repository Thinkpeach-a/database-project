//
//  BlockIO.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef BlockIO_hpp
#define BlockIO_hpp

#include <iostream>
#include <fstream>
#include <functional>
#include <utility>
#include <cstring>
#include <map>

#include "Helpers.hpp"
#include "Errors.hpp"
#include "LRUCache.hpp"

namespace ECE141 {

	enum class BlockType {
		data_block = 'D',
		free_block = 'F',
		meta_block = 'M',
		entity_block = 'E',
		index_block = 'I',
		unknown_block = 'U',
	};

	// Max size_t value
	static const uint64_t NULL_BLOCK_IND = -1;

	//a small header that describes the block...
	struct BlockHeader {

		BlockHeader(BlockType aType = BlockType::data_block, uint64_t anIndex = NULL_BLOCK_IND, uint64_t aPrevPtr = NULL_BLOCK_IND,
			uint64_t aNextPtr = 0, uint64_t aStoredSize = 0)
			: type(static_cast<char>(aType)), index{ anIndex }, prevPtr{ aPrevPtr },
			nextPtr{ aNextPtr }, storedSize{ aStoredSize } {}

		BlockHeader(const BlockHeader& aCopy) {
			*this = aCopy;
		}

		void empty() {
			type = static_cast<char>(BlockType::free_block);
		}

		BlockHeader& operator=(const BlockHeader& aCopy) {
			type = aCopy.type;
			index = aCopy.index;
			prevPtr = aCopy.prevPtr;
			nextPtr = aCopy.nextPtr;
			storedSize = aCopy.storedSize;

			return *this;
		}

		BlockHeader& operator<<(std::fstream& aStream);

		char   type;     //char version of block type
		uint64_t index;
		uint64_t prevPtr;     //pointers for navigation
		uint64_t nextPtr;    //0  if the nextPtr points to null
		uint64_t storedSize;  //size of bytes used


		enum class HeaderDataType {
			type = sizeof(char),
			index = type + sizeof(index),
			prev_ptr = index + sizeof(prevPtr),
			next_ptr = prev_ptr + sizeof(nextPtr),
			stored_size = next_ptr + sizeof(storedSize)
		};

		friend bool inline operator==(BlockHeader& aHeader, BlockHeader& aSecHeader);
	};

	inline bool operator==(BlockHeader& aHeader, BlockHeader& aSecHeader) {
		return (static_cast<char>(aSecHeader.type) == static_cast<char>(aHeader.type) &&
			aSecHeader.index == aHeader.index &&
			aSecHeader.prevPtr == aHeader.prevPtr &&
			aSecHeader.nextPtr == aHeader.nextPtr &&
			aSecHeader.storedSize == aHeader.storedSize);
	}

	const uint64_t CHAR_LEN = sizeof(char);
	const uint64_t UINT64_LEN = sizeof(uint64_t);

	std::ostream& operator<<(std::ostream& aStream, BlockHeader aHeader);

	const uint64_t kBlockSize = 1024;
	const uint64_t kBlockHeaderSize = CHAR_LEN + 4 * UINT64_LEN;
	const uint64_t kPayloadSize = kBlockSize - kBlockHeaderSize;

	//block .................
	class Block {
	public:
		// Construct with Header Data
		Block(BlockType aType, uint64_t anIndex, uint64_t aPrevPtr=NULL_BLOCK_IND, uint64_t aNextPtr=NULL_BLOCK_IND);

		// Construct from Header
		Block(BlockHeader aHeader);
		Block() : header{} {};

		Block(const Block& aCopy);

		Block& operator=(const Block& aCopy);

		/// <summary>
		/// Writes basic type to payload. Helper function to construct blocks.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="anObject"></param>
		/// <returns></returns>
		template <typename T>
		void writeToPayload(T anObject);

		void writeToPayload(std::string aString);

		// Fills remaining payload with Nulls to make sure it's fully padded.
		inline void fillRemainingPayload();


		inline void updateType(BlockType aType) { header.type = static_cast<char>(aType); }

		// Resets the block indices for reading/writing
		Block& reset() { header.storedSize = 0; payloadCounter = 0; return *this; }

		//StatusResult getPayload(std::ostream& aStream);
		//StatusResult writePayload(std::string aDataString);

		/// <summary>
		/// Reads data from payload of a specific type, initializes a counter for reading. 
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="anType"></param>
		/// <returns></returns>
		template <typename T>
		Block& readFromPayload(T& aType);

		/// <summary>
		/// Reads until first null character
		/// </summary>
		/// <param name="aStr"></param>
		/// <returns></returns>
		Block& readFromPayload(std::string& aStr);


		// Data members, TODO: Maybe refactor to encapsulate?
		BlockHeader   header;
		char          payload[kPayloadSize] = "";

		// Helps with reading
		uint64_t payloadCounter = 0;

	protected:

		friend class BlockIO;
		friend std::ostream& operator<<(std::ostream& aStream, Block aBlock);

		// Helps with copying from char array to char array
		inline void copyPayloadArray(const char* aFromArr,
			char* aDestArr, uint64_t aCopyLen);
	};

	template <typename T>
	void Block::writeToPayload(T anObject) {

		uint64_t theSize = sizeof(anObject);
		uint64_t theTotalSize = header.storedSize + theSize;

		if (theTotalSize > kPayloadSize) { throw Errors::writeError; }
		// Convert data to bytes (char), then copy it to payload
		std::memcpy(payload + header.storedSize,
			reinterpret_cast<char*>(&anObject), theSize);
		// updates stored size.
		header.storedSize = theTotalSize;

	}

	template <typename T>
	Block& Block::readFromPayload(T& aType) {
		uint64_t theSize = sizeof(aType);
		uint64_t theTotalSize = theSize + payloadCounter;

		if (theTotalSize > kPayloadSize) { throw Errors::readError; }

		std::memcpy(reinterpret_cast<char*>(&aType), payload + payloadCounter, theSize);

		payloadCounter = theTotalSize;
		return *this;
	}

	std::ostream& operator<<(std::ostream& aStream, Block aBlock);

	class BlockIO {
	public:

		BlockIO(std::fstream& aStream);
		virtual ~BlockIO() {}

		uint64_t getFileSize();

		virtual void  readBlock(uint64_t aBlockNumber, Block& aBlock);
		virtual void  writeBlock(uint64_t aBlockNumber, Block& aBlock);

	protected:
		// Reference to .db files
		std::fstream& stream;

		std::unique_ptr<LRUCache<uint64_t, Block>> cache;
	};

}

#endif /* BlockIO_hpp */
