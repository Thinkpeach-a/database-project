//
//  Attribute.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include "keywords.hpp"
#include "BasicTypes.hpp"
#include "Storage.hpp"
namespace ECE141 {

	class Attribute : public Storable {
	protected:
		std::string   name;
		DataTypes     type;
		uint16_t      size; //max=1000
		bool          autoIncrement;
		bool          primary;
		bool          nullable;
		bool          hasDefault;
		Value         defaultVal = NullType();
		//Others?

	public:

		/// <summary>
		/// Attribute in the table.
		/// </summary>
		/// <param name="aType"></param>
		Attribute(DataTypes aType = DataTypes::no_type);
		Attribute(std::string aName, DataTypes aType, uint16_t aSize = 0);
		Attribute(const Attribute& aCopy);
		~Attribute();

		void encode(Block& aBlock);
		void decode(Block& aBlock);

		Attribute& setName(std::string aName);
		Attribute& setDataType(DataTypes aType);
		Attribute& setSize(int aSize);
		Attribute& setAutoIncrement(bool anAuto);
		Attribute& setPrimaryKey(bool anAuto);
		Attribute& setNullable(bool aNullable);
		Attribute& setDefault(bool aDefault, std::string aValue="");

		bool                isValid(); //is this Entity valid?

		const std::string&  getName() const { return name; }
		DataTypes           getType() const { return type; }
		uint16_t            getSize() const { return size; }
		Value               getDefault() const { return defaultVal; }
		

		bool                isPrimaryKey() const { return primary; }
		bool                isNullable() const { return nullable; }
		bool                isAutoIncrement() const { return autoIncrement; }
		
	};

	using AttributeOpt = std::optional<Attribute>;
	using AttributeList = std::vector<Attribute>;

}


#endif /* Attribute_hpp */
