//
//  Entity.hpp
//  Assignment3
//
//  Created by rick gessner on 3/18/22.
//  Copyright © 2022 rick gessner. All rights reserved.
//

#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <map>

#include "Attribute.hpp"
#include "Errors.hpp"
#include "BasicTypes.hpp"
#include "Storage.hpp"
#include "Row.hpp"
#include "SQLQuery.hpp"

namespace ECE141 {

	using AttributeOpt = std::optional<Attribute>;
	using AttributeList = std::vector<Attribute>;
	using RowOpt = std::optional<Row>;
	using RowsOpt = std::vector<RowOpt>;


	//------------------------------------------------

	class Entity : public Storable {
	public:

		Entity(const std::string aName = "", uint64_t anId=0);
		Entity(const Entity& aCopy);
		Entity& operator=(const Entity& aCopy);

		~Entity();

		const std::string&      getName() const { return name; }
		Entity&                 updateName(std::string aName) { name = aName; return *this; }
		const AttributeList&    getAttributes() const { return attributes; }
		Entity&                 addAttribute(const Attribute& anAttribute);
		AttributeOpt            getAttribute(const std::string& aName) const;


		const Attribute*        getPrimaryKey() const;

		void                    buildRows(std::vector<std::string>& aNameList,
								std::vector<std::vector<Value>>& aValuesList, RowsOpt& aRows);

		void                    showQuery(SQLQuery& aQuery, std::ostream& anOutput, Storage* aStorage, uint64_t entityBlockInd);

		void                    setIndexPtr(uint64_t aValue) { indexPtr = aValue; }
		void                    setEntityIndex(uint64_t aValue) { entityIndex = aValue; }
		uint64_t                getEntityIndex() { return entityIndex; }
 
		void  encode(Block& aBlock) override;
		void  decode(Block& aBlock) override;

		//


	protected:

		AttributeList   attributes;
		std::map<std::string, Attribute> attributeMap;
		std::string     name;
		int             autoincr;  //auto_increment
		uint64_t        indexPtr = 0;

		uint64_t        entityIndex = 0;

		//-------------------------------------------------
		// Helper Functions

		bool containsAttributeName(std::string& aName);

		// get list of names to auto increment, removes those that exist in aNameList for user override
		void getAutoIncrementNames(std::vector<std::string>& aVector, std::vector<std::string>& aNameList);

		template <class T>
		bool vectorContains(std::vector<T>& aVector, T aValue);

		std::optional<Value> getValueFromString(const Attribute& anAttribute, Value& aStrValue);

		friend class Database;
	};

	template <class T>
	bool Entity::vectorContains(std::vector<T>& aVector, T aValue) {
		for (auto& theValue : aVector) {
			if (aValue == theValue){
				return true;
			}
		}
		return false;
	}



}
#endif /* Entity_hpp */