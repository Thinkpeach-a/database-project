//
//  Filters.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/4/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include "Errors.hpp"
#include "Tokenizer.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {

    class Row;
    class Entity;

    struct Operand {
        Operand() = default;
        Operand(std::string& aName, TokenType aType, Value& aValue, size_t anId = 0)
            : ttype(aType), dtype(DataTypes::varchar_type), name(aName),
            value(aValue), entityId(anId) {}

        TokenType   ttype = TokenType::unknown; //is it a field, or const (#, string)...
        DataTypes   dtype = DataTypes::no_type;
        std::string name = "";  //attr name
        Value       value = NullType();
        size_t      entityId = 0;
    };

    //---------------------------------------------------

    class Expression {
    public:
        Operand     lhs = Operand();  //id
        Operand     rhs = Operand();  //usually a constant; maybe a field...
        Operators   op = Operators::unknown_op;   //=     //users.id=books.author_id
        Logical     logic = Logical::unknown_op; //and, or, not...
        Parentheses paren = Parentheses::no_paren;
        bool isParenthese = false;
        bool isLogical = false;

        Expression(Operand& aLHSOperand, Operators anOp,
            Operand& aRHSOperand)
            : lhs(aLHSOperand), rhs(aRHSOperand),
            op(anOp), logic(Logical::no_op) {}

        Expression(const Logical& aLogic) :isLogical(true), logic(aLogic) {}
        Expression(const Parentheses& aParen) :isParenthese(true), paren(aParen) {}
        Expression() {}

        bool operator()(KeyValues& aList);

    };

    using Expressions = std::vector<std::shared_ptr<Expression> >;

    //---------------------------------------------------

    class Filters {
    public:

        Filters();
        Filters(const Filters& aFilters);
        ~Filters();

        size_t        getCount() const { return expressions.size(); }
        bool          matches(KeyValues& aList) const;
        Filters& add(Expression* anExpression);

        StatusResult  parse(Tokenizer& aTokenizer, Entity& anEntity);

    protected:
        Expressions   expressions;
    };

}

#endif /* Filters_h */