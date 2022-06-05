//
//  Entity.cpp
//  PA3
//
//  Created by rick gessner on 3/2/22.
//

#include <stdio.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <set>
#include "Entity.hpp"
#include "BasicTypes.hpp"
#include "TableView.hpp"
#include "Storage.hpp"

namespace ECE141 {
	using RowsOpt = std::vector<RowOpt>;
	std::map<DataTypes, char> dataTypesLetter{ {DataTypes::no_type,'N'},
		{DataTypes::bool_type,'B'},
		{DataTypes::datetime_type,'D'},
		{DataTypes::float_type,'F'},
		{DataTypes::int_type,'I'},
		{DataTypes::varchar_type,'V'} };

	//using AttributeValueList = std::vector<uint16_t>;

	//STUDENT: Implement this class...

	Entity::Entity(const std::string aName, uint64_t anId) :
		name(aName), autoincr(0), entityIndex{anId} {}

	Entity::Entity(const Entity& aCopy) {
		*this = aCopy;
	}

	Entity& Entity::operator=(const Entity& aCopy) {
		name = aCopy.name;

		attributeMap = aCopy.attributeMap;
		attributes = aCopy.attributes;

		indexPtr = aCopy.indexPtr;
		entityIndex = entityIndex;
		autoincr = aCopy.autoincr;

		//std::cout << "AutoIncrement: " << autoincr << "\n";
		return *this;
	}

	Entity::~Entity() {
		//std::cout << "~Entity()\n";

	}

	Entity& Entity::addAttribute(const Attribute& anAttribute) {
		std::string theName = anAttribute.getName();
		if (!getAttribute(anAttribute.getName())) {
			attributes.push_back(anAttribute); // For iteration
			attributeMap[theName] = anAttribute; // For quick search
		}
		else {
			throw Errors::invalidAttribute;
		}
		return *this;
	}

	AttributeOpt Entity::getAttribute(const std::string& aName) const {
		if (attributeMap.count(aName) > 0) {
			return attributeMap.at(aName); // Use .at for const key
		}
		return std::nullopt;
	}

	// USE: ask the entity for name of primary key (may not have one...)
	const Attribute* Entity::getPrimaryKey() const {
		for (auto& theAttribute : attributes) {
			if (theAttribute.isAutoIncrement()) {
				return &theAttribute;
			}
		}
		return nullptr;
	}

	void Entity::buildRows(std::vector<std::string>& aNameList,
		std::vector<std::vector<Value>>& aValuesList, RowsOpt& aRows) {
		//TODO: Refactor logic for clean up

		// Check if names from attributes are correct, 
		// then make sure excluded columns are NULLABLE, else return;
		size_t theCount = 0; // Counter to see how many of the attribute names are in aNameList

		Row theTemplateRow{ 0 }; // Builds a row we can use as a base
		Value theNullValue = NullType();

		for (auto& theAttribute : attributes) {
			std::string theAttributeName = theAttribute.getName();
			if (!vectorContains(aNameList, theAttributeName)) { // attribute not in name list
				if (!(theAttribute.isAutoIncrement()) && !(theAttribute.isNullable())) { throw Errors::invalidAttribute; }
				theTemplateRow.set(theAttributeName, theNullValue); // We'll check for autoincrement later
			}
			else { theCount++; } // attribute in name list
		}
		// attribute names mismatch
		if (theCount != aNameList.size()) { throw Errors::invalidAttribute; }

		// Build Rows by Argument
		// Behavior: if auto increment field is not written, then we fill it with autoincrement val, 
		// else do nothing
		std::vector<std::string> theAutoNames;
		getAutoIncrementNames(theAutoNames, aNameList);

		std::string theKey;
		Value theValue;
		std::optional<Value> theTypedValue;
		size_t theArrSize = aNameList.size();

		//bool theValidFlag = true;
		// Add user input values to non NULL fields, auto writes auto increment
		for (auto& theRowValues : aValuesList) {
			for (size_t theIndex = 0; theIndex < theArrSize; ++theIndex) {

				theKey = aNameList[theIndex];
				theValue = theRowValues[theIndex]; // do type validation
				// if correct type
				Attribute theAttribute = getAttribute(theKey).value();
				theTypedValue = getValueFromString(theAttribute, theValue);
				if (theTypedValue) { //TODO: Need to refactor to use map instead of vector
					theTemplateRow.set(theKey, theTypedValue.value());
				}
				else {
					throw Errors::invalidArguments;
				}
			}
			// write autoincrement values

			for (auto& theName : theAutoNames) {
				theTemplateRow.set(theName, ++autoincr);
			}
			aRows.push_back(theTemplateRow);
		}
	}

	void Entity::showQuery(SQLQuery& aQuery, std::ostream& anOutput, Storage* aStorage, uint64_t entityBlockInd) {
	}

	void Entity::encode(Block& aBlock) {
		aBlock.header.index = entityIndex;
		aBlock.header.type = static_cast<char>(BlockType::entity_block);
		aBlock.writeToPayload(name);
		aBlock.writeToPayload(indexPtr);
		aBlock.writeToPayload(autoincr);

		// Write to payload
		aBlock.writeToPayload(static_cast<uint64_t>(attributes.size()));
		for (auto& theAttribute : attributes) {
			theAttribute.encode(aBlock);
		}
	}

	void Entity::decode(Block& aBlock) {

		entityIndex = aBlock.header.index;
		aBlock.readFromPayload(name);
		aBlock.readFromPayload(indexPtr);
		aBlock.readFromPayload(autoincr);

		uint64_t theAttributeCount;
		Attribute theAttribute;

		attributes.clear();
		aBlock.readFromPayload(theAttributeCount);
		for (uint64_t theIndex = 0; theIndex < theAttributeCount; ++theIndex) {
			theAttribute.decode(aBlock);
			attributes.push_back(theAttribute);
			attributeMap[theAttribute.getName()] = theAttribute;
		}
	}

	// TODO: optimize?
	bool Entity::containsAttributeName(std::string& aName) {
		for (auto& theAttribute : attributes) {
			if (aName == theAttribute.getName()) { return true; };
		}
		return false;
	}

	void Entity::getAutoIncrementNames(std::vector<std::string>& aVector, std::vector<std::string>& aNameList) {

		std::string theAttributeName;
		for (auto& theAttribute : attributes) {
			theAttributeName = theAttribute.getName();
			if (theAttribute.isAutoIncrement() && !vectorContains(aNameList, theAttributeName)) {
				aVector.push_back(theAttributeName);
			}
		}
	}

	std::optional<Value> Entity::getValueFromString(const Attribute& anAttribute, Value& aStrValue) {
		DataTypes theType = anAttribute.getType();
		try {
			return BasicTypes::toValue(std::get<std::string>(aStrValue), theType);
		}
		catch (const std::invalid_argument&) {
			return std::nullopt;
		}
		catch (const std::out_of_range&) {
			return std::nullopt;
		}
	}
}
