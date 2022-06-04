//
//  Attribute.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include <iostream>
#include "Attribute.hpp"

namespace ECE141 {

	Attribute::Attribute(DataTypes aType)
		: type(aType), size(0), autoIncrement(0), primary(0), nullable(1), hasDefault(0) {}

	Attribute::Attribute(std::string aName, DataTypes aType, uint16_t aSize) {
		name = aName;
		type = aType;
		size = aSize;
		autoIncrement = 0;
		primary = 0;
		nullable = 1;
		hasDefault = 0;
	}

	Attribute::Attribute(const Attribute& aCopy) {
		name = aCopy.name;
		type = aCopy.type;
		size = aCopy.size;
		autoIncrement = aCopy.autoIncrement;
		primary = aCopy.primary;
		nullable = aCopy.nullable;
        hasDefault=aCopy.hasDefault;
        defaultVal=aCopy.defaultVal;
	}

	Attribute::~Attribute() {
	}

	void Attribute::encode(Block& aBlock) {
		aBlock.writeToPayload(name);
		aBlock.writeToPayload(type);
		aBlock.writeToPayload(size);
		aBlock.writeToPayload(autoIncrement);
		aBlock.writeToPayload(primary);
		aBlock.writeToPayload(nullable);
		aBlock.writeToPayload(primary);
		aBlock.writeToPayload(hasDefault);
		aBlock.writeToPayload(defaultVal);
	}

	void Attribute::decode(Block& aBlock) {
		aBlock.readFromPayload(name);
		aBlock.readFromPayload(type);
		aBlock.readFromPayload(size);
		aBlock.readFromPayload(autoIncrement);
		aBlock.readFromPayload(primary);
		aBlock.readFromPayload(nullable);
		aBlock.readFromPayload(primary);
		aBlock.readFromPayload(hasDefault);
		aBlock.readFromPayload(defaultVal);
	}

	Attribute& Attribute::setName(std::string aName) {
		name = aName;
		return *this;
	}

	Attribute& Attribute::setDataType(DataTypes aType) {
		type = aType;
		return *this;
	}

	Attribute& Attribute::setSize(int aSize) {
		size = aSize; return *this;
	}

	Attribute& Attribute::setAutoIncrement(bool anAuto) {
		autoIncrement = anAuto; return *this;
	}

	Attribute& Attribute::setPrimaryKey(bool aPrimary) {
		primary = aPrimary; return *this;
	}

	Attribute& Attribute::setNullable(bool aNullable) {
		nullable = aNullable; return *this;
	}

	Attribute& Attribute::setDefault(bool aDefault, std::string aValue) {
		hasDefault = aDefault; defaultVal = aValue; return *this;
	}

	bool Attribute::isValid() {
		return true;
	}
}
