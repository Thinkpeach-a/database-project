//
//  BlockIO.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "BlockIO.hpp"
#include <cstring>
#include <sstream>
#include <bitset>
#include <cmath>
#include <filesystem>


namespace ECE141 {

	namespace fs = std::filesystem;

	Block::Block(BlockType aType, uint64_t anIndex, uint64_t aPrevPtr, uint64_t aNextPtr)
		: header(aType, anIndex, aPrevPtr, aNextPtr, 0) {}

	Block::Block(BlockHeader aHeader) : header{ aHeader } {}

	Block::Block(const Block& aCopy) {
		*this = aCopy;
	}

	Block& Block::operator=(const Block& aCopy) {
		std::memcpy(payload, aCopy.payload, kPayloadSize);
		header = aCopy.header;
		return *this;
	}

	void Block::writeToPayload(std::string aString) {
		uint64_t theSize = aString.length() + 1; //we want to store null character as well
		uint64_t theTotalSize = theSize + header.storedSize;
		if (theTotalSize > kPayloadSize) { throw Errors::writeError; }
		copyPayloadArray(aString.c_str(), payload + header.storedSize, theSize);

		header.storedSize = theTotalSize;

	}

	Block& Block::readFromPayload(std::string& aStr) {
		aStr.clear();

		char currentChar = payload[payloadCounter];

		// Reads until null termination character
		while (currentChar != '\0' && payloadCounter < kPayloadSize - 1) {
			aStr += currentChar;
			++payloadCounter;
			currentChar = payload[payloadCounter];
		}
		if (currentChar == '\0') {
			++payloadCounter;
		}
		return *this;
	}

	//TODO: Optimize with STL?
	void Block::fillRemainingPayload() {
		for (uint64_t theIndex = header.storedSize; theIndex < kPayloadSize; ++theIndex) {
			payload[theIndex] = '\0';
		}
		header.storedSize = kPayloadSize;
	}

	//TODO: Optimize with STL?
	void Block::copyPayloadArray(const char* aFromArr, char* aDestArr, uint64_t aCopyLen) {
		for (uint64_t theIndex = 0; theIndex < aCopyLen; ++theIndex) {
			aDestArr[theIndex] = aFromArr[theIndex];
		}
	}

	BlockHeader& BlockHeader::operator<<(std::fstream& aStream) {

		aStream.read(&type, CHAR_LEN).
			read(reinterpret_cast<char*>(&index), UINT64_LEN).
			read(reinterpret_cast<char*>(&prevPtr), UINT64_LEN).
			read(reinterpret_cast<char*>(&nextPtr), UINT64_LEN).
			read(reinterpret_cast<char*>(&storedSize), UINT64_LEN);
		if (!aStream.good()) { throw Errors::readError; }

		return *this;
	}

	//---------------------------------------------------

	BlockIO::BlockIO(std::fstream& aStream) : stream(aStream) {}
	// USE: write data a given block (after seek)
	void BlockIO::writeBlock(uint64_t aBlockNum, Block& aBlock) {

		stream.seekp(aBlockNum * kBlockSize, std::ios::beg);
		stream << aBlock;

		if (!stream.good()) { throw Errors::writeError; }

		stream.flush();
	}

	// USE: write data a given block (after seek) ---------------------------------------
	void BlockIO::readBlock(uint64_t aBlockNum, Block& aBlock) {
		stream.seekg(aBlockNum * kBlockSize, std::ios::beg);

		aBlock.header << stream;

		stream.read(reinterpret_cast<char*>(&(aBlock.payload)), kPayloadSize * CHAR_LEN);
		stream.flush();
		if (!stream.good()) { throw Errors::readError; }
	}

	std::ostream& operator<<(std::ostream& aStream, BlockHeader aHeader) {

		aStream.write(reinterpret_cast<char*>(&aHeader.type), CHAR_LEN).
			write(reinterpret_cast<char*>(&aHeader.index), UINT64_LEN).
			write(reinterpret_cast<char*>(&aHeader.prevPtr), UINT64_LEN).
			write(reinterpret_cast<char*>(&aHeader.nextPtr), UINT64_LEN).
			write(reinterpret_cast<char*>(&aHeader.storedSize), UINT64_LEN).flush();

		if (!aStream.good()) { throw Errors::writeError; }

		aStream.flush();
		return aStream;
	}

	std::ostream& operator<<(std::ostream& aStream, Block aBlock)
	{
		aStream << aBlock.header;
		aStream.write((char*)&aBlock.payload, kPayloadSize);

		if (!aStream.good()) { throw Errors::writeError; }

		return aStream;
	}
}
