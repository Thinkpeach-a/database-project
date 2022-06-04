//
//  BasicTypes.hpp
//  RGAssignement1
//
//  Created by rick gessner on 3/9/21.
//

#ifndef BasicTypes_h
#define BasicTypes_h

#include <string>
#include <variant>
#include <map>
#include <optional>
#include <iostream>
#include <vector>

namespace ECE141 {

	enum class DataTypes {
		no_type = 'N', bool_type = 'B', datetime_type = 'D',
		float_type = 'F', int_type = 'I', varchar_type = 'V',
	};

	struct NullType {
		template<typename T>
		bool operator==(const T& aType) { return false; }

		bool operator==(const NullType& aType) { return true; }

		template<typename T>
		bool operator<(const T& aType) { return true; }

		template<typename T>
		bool operator>(const T& aType) { return false; }

		operator bool() { return false; }
		operator int() {return int(-1); }
	};

	//std::ostream& operator<<(std::ostream& anOutput, NullType) {
	//	return anOutput << int(-1);
	//}

	template<typename T>
	bool operator==(const T& aType, const NullType& aNull) { return false; }

	template<typename T>
	bool operator<(const T& aType, const NullType& aNull) { return false; }

	template<typename T>
	bool operator>(const T& aType, const NullType& aNull) { return true; }



	using StringList = std::vector<std::string>;
	using StringMap = std::map<std::string, std::string>;
	using StringOpt = std::optional<std::string>;
	using IntOpt = std::optional<uint64_t>;
	using Value = std::variant<bool, int, double, uint64_t, std::string, NullType>;
	using KeyValues = std::map<const std::string, Value>;

	//Comparisons for Value
	//TODO: Fix the templates. Templates really were not working with the variant types
	//especially with our custom type. I'm probably missing something.

	//bool operator>(const std::string& aStr, const Value& aVal) {
	//	return aStr > BasicTypes::toString(aVal);
	//}

	//bool operator==(const std::string& aStr, const Value& aVal) {
	//	return aStr == BasicTypes::toString(aVal);
	//}

	//bool operator<(const std::string& aStr, const Value& aVal) {
	//	return !(aStr > aVal || aStr == aVal);
	//}

	//bool operator!=(const std::string& aStr, const Value& aVal) {
	//	return !(aStr == aVal);
	//}

	//bool operator>=(const std::string& aStr, const Value& aVal) {
	//	return true;
	//}

	//bool operator<=(const std::string& aStr, const Value& aVal) {
	//	return !(aStr > aVal);
	//}

	//bool operator>(const std::string& aStr, const Value& aVal) {
	//	return true;
	//}

	//// Reverse order
	//bool operator<(const Value& aVal, const std::string& aStr) {
	//	return true;
	//}

	//bool operator==(const Value& aVal, const std::string& aStr) {
	//	return true;
	//}

	//bool operator!=(const Value& aVal, const std::string& aStr) {
	//	return true;
	//}

	//bool operator>=(const Value& aVal, const std::string& aStr) {
	//	return true;
	//}

	//bool operator<=(const Value& aVal, const std::string& aStr) {
	//	return true;
	//}

	//// Value & Value

	//bool operator<(const Value& aVal1, const Value& aVal2) {
	//	return true;
	//}

	//bool operator==(const Value& aVal1, const Value& aVal2) {
	//	return true;
	//}

	//bool operator!=(const Value& aVal1, const Value& aVal2) {
	//	return true;
	//}

	//bool operator>=(const Value& aVal1, const Value& aVal2) {
	//	return true;
	//}

	//bool operator<=(const Value& aVal1, const Value& aVal2) {
	//	return true;
	//}

	static inline std::map<size_t, DataTypes> valueToType{
		{0, DataTypes::bool_type},
		{1, DataTypes::int_type},
		{2, DataTypes::float_type},
		{3, DataTypes::datetime_type},
		{4, DataTypes::varchar_type},
		{5, DataTypes::no_type}
	};

	static inline std::map<DataTypes, size_t> typeToValue{
		{DataTypes::bool_type, 0},
		{DataTypes::int_type, 1},
		{DataTypes::float_type, 2},
		{DataTypes::datetime_type, 3},
		{DataTypes::varchar_type, 4},
		{DataTypes::no_type, 5}
	};

	// Helper functions for Value Type
	namespace BasicTypes {
		static Value toValue(const std::string& aValue, DataTypes aType) {

			Value theValue;
			//no_type = 'N', bool_type = 'B', datetime_type = 'D',
			//	float_type = 'F', int_type = 'I', varchar_type = 'V',
			switch (aType) {
			case DataTypes::bool_type:
				if ("true" == aValue) { theValue = true; }
				else if ("false" == aValue) { theValue = false; }
				else {
					theValue = static_cast<bool>(std::stoi(aValue));
				}
				break;
			case DataTypes::datetime_type:
				theValue = static_cast<uint64_t>(std::stoll(aValue));
				break;
			case DataTypes::float_type:
				theValue = static_cast<double>(std::stof(aValue));
				break;
			case DataTypes::int_type:
				theValue = static_cast<int>(std::stoi(aValue));
				break;
			case DataTypes::varchar_type:
				theValue = aValue;
				break;
			case DataTypes::no_type:
				theValue = NullType();
			}
			return theValue;
		}

		// Find more flexible implementation
		static std::string toString(const Value aValue) {
			//bool, int, double, uint64_t, std::string
			size_t typeIndex = aValue.index();

			std::string theStr;
			switch (typeIndex) {
			case 0: //bool
				theStr = std::get<bool>(aValue) ? "true" : "false";
				break;
			case 1: //int
				theStr = std::to_string(std::get<int>(aValue));
				break;
			case 2: //double
				theStr = std::to_string(std::get<double>(aValue));
				break;
			case 3: //uint64_t
				theStr = std::to_string(std::get<uint64_t>(aValue));
				break;
			case 4: //string
				theStr = std::get<std::string>(aValue);
				break;
			case 5:
				theStr = "NULL";
				break;
			default:
				theStr = "NULL";

			}
			return theStr;
		}
	}

}
#endif /* BasicTypes_h */
