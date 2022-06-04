//
//  Row.cpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//


#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "BasicTypes.hpp"
#include "Row.hpp"


namespace ECE141 {

	Row::Row(uint64_t anEntityID) : entityId{anEntityID} {}
	Row::Row(const Row& aRow) { *this = aRow; }

	Row::~Row() {}

	Row& Row::operator=(const Row& aRow) { 
		entityId = aRow.entityId;
        data.insert(aRow.data.begin(),aRow.data.end());
		return *this; 
	}
	bool Row::operator==(Row& aCopy) const { 
		return false;
	}

	//STUDENT: What other methods do you require?

	Row& Row::set(const std::string& aKey, const Value& aValue) {
		data[aKey] = aValue;
		return *this;
	}

	void Row::encode(Block& aBlock) {

		aBlock.header.index = entityId;
		aBlock.header.type = static_cast<char>(BlockType::data_block);

		aBlock.writeToPayload(entityId);
		aBlock.writeToPayload(static_cast<uint64_t>(data.size()));
		for (auto theMapIter : data) {
			aBlock.writeToPayload(theMapIter.first);
			aBlock.writeToPayload(static_cast<uint16_t>(theMapIter.second.index()));
			aBlock.writeToPayload(BasicTypes::toString(theMapIter.second));
		}
	}

	void Row::decode(Block& aBlock) {
		uint64_t theMapSize;
		std::string theName;
		uint16_t    theVarIndex;
		std::string theValueStr;

		aBlock.readFromPayload(entityId);
		aBlock.readFromPayload(theMapSize);

		aBlock.header.index = entityId;

		for (uint16_t theIndex = 0; theIndex < theMapSize; ++theIndex) {
			aBlock.readFromPayload(theName);
			aBlock.readFromPayload(theVarIndex);
			aBlock.readFromPayload(theValueStr);

			set(theName, BasicTypes::toValue(theValueStr, valueToType[theVarIndex]));
		}
	}

}
