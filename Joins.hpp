//
//  Join.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/21/21.
//

#ifndef Join_h
#define Join_h

#include <string>
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "keywords.hpp"
#include "Row.hpp"
#include "Filters.hpp"

namespace ECE141 {

  class Join  {
  public:
    //Join(const std::string &aLeftTable, const std::string &aLeftValue,const std::string &aRightTable,
    //    std::string &aRightValue, Keywords aType)
    //    : leftTable(aLeftTable), leftValue(aLeftValue),
    //      rightTable(aRightTable), rightValue(aRightValue),
    //      joinType(aType) {}
            
    std::string leftTable;
    std::string leftValue;

    std::string rightTable;
    std::string rightValue;

    Keywords    joinType;  //left, right, etc...
    //Expression  expr;
  };

  using JoinList = std::vector<Join>;

}

#endif /* Join_h */
