#ifndef SQLStatements_hpp
#define SQLStatements_hpp

#include <string>
#include <initializer_list>
#include <map>
#include <set>

#include "Statement.hpp"
#include "TokenSequencer.hpp"
#include "Entity.hpp"
#include "BasicTypes.hpp"
#include "ParseHelper.hpp"

namespace ECE141 {

	class SQLStatement : public Statement{
	public:
		SQLStatement(StatementType aType = StatementType::unknown_type) : Statement{aType} {}
       
		virtual ~SQLStatement() {};

		// Lists with pre-determined keywords for each statement type.
		static const std::initializer_list<Keywords> createTablesList;
		static const std::initializer_list<Keywords> describeTableList;
		static const std::initializer_list<Keywords> dropTableList;
		static const std::initializer_list<Keywords> showTablesList;
        static const std::initializer_list<Keywords> insertRowList;
        
		static inline std::map <Keywords, DataTypes> typeMap{
			{Keywords::boolean_kw, DataTypes::bool_type},
			{Keywords::float_kw, DataTypes::float_type},
			{Keywords::integer_kw, DataTypes::int_type},
			{Keywords::datetime_kw, DataTypes::datetime_type},
			{Keywords::varchar_kw, DataTypes::varchar_type},
		};
	};
      
	class CreateTableStatement : public SQLStatement {
	public:
		CreateTableStatement() : SQLStatement{ StatementType::create_tb } {}

		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
       
		Entity& getEntity() { return entity; }
	protected:
		void parseAttributes(TokenSequencer& aSequencer, Entity& anEntity);
		void parseConstraints(TokenSequencer& aSequencer, Attribute& anAttribute);

		Entity entity{""};
	};
   
	class DropTableStatement : public SQLStatement {
	public:
		DropTableStatement() : SQLStatement{ StatementType::drop_tb } {}
	
		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;

		std::string getTableName() { return tableName; }
	protected:
		std::string tableName = "";
	};

	class DescribeTableStatement : public SQLStatement {
	public:
		DescribeTableStatement() : SQLStatement{ StatementType::describe_tb } {}
		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
		std::string getTableName() { return tableName; }
	protected:
		std::string tableName = "";
	
	};

	class ShowTablesStatement : public Statement {
	public:
		ShowTablesStatement() : Statement{ StatementType::show_tbs} {}
		static bool recognize(TokenSequencer& aSequencer);

		void parse(TokenSequencer& aSequencer) override;
	};
    
    class InsertRowStatement : public SQLStatement{
    public:
        InsertRowStatement() : SQLStatement{ StatementType::insert_row}{}
        static bool recognize(TokenSequencer& aSequencer);
        void parse(TokenSequencer& aSequencer) override;
        //parse the actual field name
        bool parseAttributesNames(TokenSequencer& aSequencer,std::vector<std::string>& attributesNames);
        //parse the values
        bool parseAttributeValues(TokenSequencer& aSequencer,std::vector<Value>& rowValues);
        
        std::vector<std::string>& getNamesVector(){
            return theAttributeNames;
        }
        std::vector<std::vector<Value>>& getRowsVector(){
            return theAttributeValues;
        }
        std::string getTableName(){
            return tableName;
        }
    protected:
        //store the attribute names;
        std::vector<std::string> theAttributeNames;
        //store the actual values of the relative attribute;
        std::vector<std::vector<Value>> theAttributeValues;
        //get the actual table name
        std::string tableName;
    };
    
	
}
#endif // SQLStatements_hpp
