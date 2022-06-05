#ifndef SQLQuery_hpp
#define SQLQuery_hpp

#include "Filters.hpp"
#include "Joins.hpp"


namespace ECE141 {
    class SQLQuery {
    public:
        // Entity Name
        std::string EntityName;

        // Column names to display
        std::vector<std::string> columnNames;

        // Where
        Filters whereFilter;

        // Group By
        std::vector<std::string> groupNames;

        // Order By
        std::vector<std::string> orderNames;

        // Limit
        uint64_t limitStartIndex = 0;
        uint64_t limitCount = 0;

        // Join
        Join join;

        //Delete
        bool hasWhere = false;
        bool hasJoin = false;

        //cache
        std::string selectString = "";
    };
}

#endif // !SQLQuery_hpp
