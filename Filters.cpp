#include "Filters.hpp"
#include <string>
#include <limits>
#include "keywords.hpp"
#include "Helpers.hpp"
#include "Entity.hpp"
#include "Attribute.hpp"
#include "Compare.hpp"
#include <stack>
namespace ECE141 {

    using Comparitor = bool (*)(Value& aLHS, Value& aRHS);
    bool isComparable(Operand& anOperand) {
        if (TokenType::identifier == anOperand.ttype) {
            if (DataTypes::bool_type != anOperand.dtype || DataTypes::no_type != anOperand.dtype) {
                return true;
            }
        }
        //if it is not attribute name: then is always true
        else {
            return true;
        }
        return false;
    }
    /*	bool equals(Value& aLHS, Value& aRHS) {
            bool theResult = false;

            std::visit([&](auto const& aLeft) {
                std::visit([&](auto const& aRight) {
                    theResult = isEqual(aLeft, aRight);
                    }, aRHS);
                }, aLHS);
            return theResult;
        }*/

        /*	static std::map<Operators, Comparitor> comparitors{
              {Operators::equal_op, equals},
              //STUDENT: Add more for other operators...
            };

            bool Expression::operator()(KeyValues& aList) {
                Value theLHS{ lhs.value };
                Value theRHS{ rhs.value };

                if (TokenType::identifier == lhs.ttype) {
                    theLHS = aList[lhs.name]; //get row value
                }

                if (TokenType::identifier == rhs.ttype) {
                    theRHS = aList[rhs.name]; //get row value
                }

                return comparitors.count(op)
                    ? comparitors[op](theLHS, theRHS) : false;
            }*/
    bool Equals(Operand& aLeft, Operand& aRight) {
        if (TokenType::identifier == aLeft.ttype && TokenType::identifier == aRight.ttype) {
            if (aLeft.dtype == aRight.dtype) {
                // if they are both no_type, return true
                if (aLeft.dtype == DataTypes::no_type) {
                    return true;
                }
                else {
                    return isEqual(aLeft.value, aRight.value);
                }
            }

        }
        else if (TokenType::identifier == aLeft.ttype) {
            if (aLeft.dtype != DataTypes::no_type) {
                return isEqual(aLeft.value, aRight.value);
            }
        }
        else if (TokenType::identifier == aRight.ttype) {
            if (aRight.dtype != DataTypes::no_type) {
                return isEqual(aLeft.value, aRight.value);
            }
        }
        return false;
    }



    bool isLarger(Operand& aLeft, Operand& aRight) {
        if (TokenType::identifier == aLeft.ttype && TokenType::identifier == aRight.ttype) {
            if (aLeft.dtype == aRight.dtype) {
                // decide whether the two variables are comparable
                if (isComparable(aLeft)) {
                    return aLeft.value > aRight.value;
                }

            }

        }
        else if (TokenType::identifier == aLeft.ttype) {
            if (isComparable(aLeft)) {
                return aLeft.value > aRight.value;
            }
        }
        else if (TokenType::identifier == aRight.ttype) {
            if (isComparable(aRight)) {
                return aLeft.value > aRight.value;
            }
        }
        return false;
    }

    bool isSmaller(Operand& aLeft, Operand& aRight) {
        if (isComparable(aLeft) && isComparable(aRight)) {
            return !(isLarger(aLeft, aRight) || Equals(aLeft, aRight));
        }
        return false;
    }
    //calculate the expression bool value
    bool calculate(Expression& anExpression) {

        if (Operators::notequal_op == anExpression.op) {
            return !Equals(anExpression.lhs, anExpression.rhs);
        }
        else if (Operators::lt_op == anExpression.op) {
            return isSmaller(anExpression.lhs, anExpression.rhs);
        }
        else if (Operators::gt_op == anExpression.op) {
            return isLarger(anExpression.lhs, anExpression.rhs);
        }
        else if (Operators::gte_op == anExpression.op) {
            return Equals(anExpression.lhs, anExpression.rhs) || isLarger(anExpression.lhs, anExpression.rhs);
        }
        else if (Operators::lte_op == anExpression.op) {
            return Equals(anExpression.lhs, anExpression.rhs) || isSmaller(anExpression.lhs, anExpression.rhs);
        }
        else if (Operators::equal_op==anExpression.op){
            return Equals(anExpression.lhs, anExpression.rhs);
        }
        return false;
    }
    //get the result of two expressions
    bool calcuTwoExpressions(bool leftExp, bool rightExp, Logical& aLogic) {
        if (Logical::and_op == aLogic) {
            return leftExp && rightExp;
        }
        else if (Logical::or_op == aLogic) {
            return leftExp || rightExp;
        }
        return true;
    }
    //calculate the result for a series of expressions
    bool calcuExpressions(std::stack<std::shared_ptr<Expression>>& theExpressions, std::stack<bool>& theExpResults) {
        Logical currLogic;
        size_t variableCount = 0;
        bool leftValue = true; bool rightValue = false;
        while (!theExpressions.empty() && !theExpressions.top()->isParenthese) {
            if (theExpressions.top()->isLogical) {
                currLogic = theExpressions.top()->logic;
                theExpressions.pop();
            }
            else {// A and B : find out right or left needs to be assigned with value
                if (1 == variableCount) {
                    rightValue = theExpResults.top();
                    theExpResults.pop();
                    theExpressions.pop();
                    leftValue = calcuTwoExpressions(leftValue, rightValue, currLogic);

                }
                else {
                    leftValue = theExpResults.top();
                    theExpressions.pop();
                    theExpResults.pop();
                    variableCount++;
                }
            }
            if (theExpressions.empty()) { break; }
        }
        if(!theExpressions.empty()){theExpressions.pop();}
        theExpressions.push(std::shared_ptr<Expression>(new Expression()));
        theExpResults.push(leftValue);
        return leftValue;
    }


    //--------------------------------------------------------------

    Filters::Filters() {}

    Filters::Filters(const Filters& aCopy) {
    }

    Filters::~Filters() {
        //no need to delete expressions, they're unique_ptrs!
    }

    Filters& Filters::add(Expression* anExpression) {
        expressions.push_back(std::shared_ptr<Expression>(anExpression));
        return *this;
    }

    //compare expressions to row; return true if matches
    bool Filters::matches(KeyValues& aList) const {

        size_t theExpressionSize = expressions.size();
        if (theExpressionSize == 0) { return true; } // no requirements
        
        //one for the expressions eg: C and (A and B)
        std::stack<std::shared_ptr<Expression>> theExpressions;
        //bool stack for the result of the expression, eg: true false
        std::stack<bool> theExpResults;
        for (int64_t index = theExpressionSize - 1; index >= 0; --index) {
            //pop the element out when meeting "("
            if (expressions[index]->isParenthese) {
                if (Parentheses::right_paren == expressions[index]->paren) {
                    theExpressions.push(expressions[index]);
                }
                else {
                    //stop until we meet the ')'
                    calcuExpressions(theExpressions, theExpResults);
                }
            }
            //push the element when is a <left op right> expression
            else {
                //if is and nor not
                if (expressions[index]->isLogical) {
                    if (Logical::not_op == expressions[index]->logic) {
                        //convert the neareat bool type element
                        bool theLogic = theExpResults.top();
                        theExpResults.pop();
                        theExpResults.push(!theLogic);
                    }
                    else {
                        theExpressions.push(expressions[index]);
                        //theExpResults.push(calculate(*expressions[index]));
                    }
                }
                //if it is actual expressions get the value from row and calculate the value, then push it in two stack
                else {
                    //if the operand has row name, get the value from the list and calculate value of rhs and lhs
                    if (TokenType::identifier == expressions[index]->rhs.ttype) {
                        expressions[index]->rhs.value = aList[expressions[index]->rhs.name];
                    }
                    if (TokenType::identifier == expressions[index]->lhs.ttype) {
                        expressions[index]->lhs.value = aList[expressions[index]->lhs.name];
                    }
                    theExpressions.push(expressions[index]);
                    theExpResults.push(calculate(*expressions[index]));
                }
            }
        }
        bool theReturn = calcuExpressions(theExpressions, theExpResults);
        return theReturn;
    }

    //help parse the logical operator:
    StatusResult parseLogical(Tokenizer& aTokenizer, Logical& aLogic) {
        StatusResult theResult{ noError };
        if ("and" == aTokenizer.current().data) {
            aLogic = Logical::and_op;
        }
        else if ("or" == aTokenizer.current().data) {
            aLogic = Logical::or_op;
        }
        else if ("not" == aTokenizer.current().data) {
            aLogic = Logical::not_op;
        }
        else { theResult = Errors::unknownType; }
        return theResult;
    }
    //where operand is field, number, string...
    StatusResult parseOperand(Tokenizer& aTokenizer,
        Entity& anEntity, Operand& anOperand) {
        StatusResult theResult{ noError };
        Token& theToken = aTokenizer.current();
        if (TokenType::identifier == theToken.type) {
            if (anEntity.getAttribute(theToken.data)) {
                anOperand.ttype = theToken.type;
                anOperand.name = theToken.data; //hang on to name...
                anOperand.entityId = 0;//Helpers::hashString(theToken.data.c_str());
                anOperand.dtype = anEntity.getAttribute(theToken.data)->getType();
            }
            else {
                anOperand.ttype = TokenType::string;
                anOperand.dtype = DataTypes::varchar_type;
                anOperand.value = theToken.data;
            }
        }
        else if (TokenType::number == theToken.type) {
            anOperand.ttype = TokenType::number;
            anOperand.dtype = DataTypes::int_type;
            if (theToken.data.find('.') != std::string::npos) {
                anOperand.dtype = DataTypes::float_type;
                anOperand.value = std::stof(theToken.data);
            }
            else anOperand.value = std::stoi(theToken.data);
        }
        else theResult.error = syntaxError;
        if (theResult) aTokenizer.next();
        return theResult;
    }

    //STUDENT: Add validation here...
    bool validateOperands(Operand& aLHS, Operand& aRHS, Entity& anEntity) {
        if (TokenType::identifier == aLHS.ttype || TokenType::string == aLHS.ttype) { // Verify: Added string type for comparison
            //whether we can find the name
            if (!anEntity.getAttribute(aLHS.name)) { // replace aLHS.name with BasicTypes::toString(aLHS.value)
                return false;
            }
            return true;
        }
        else if (TokenType::identifier == aRHS.ttype) {
            if (!anEntity.getAttribute(aRHS.name)) {
                return false;
            }
            //STUDENT: Add code for validation as necessary
            return true;
        }
        return false;
    }

    //STUDENT: This starting point code may need adaptation...
    StatusResult Filters::parse(Tokenizer& aTokenizer, Entity& anEntity) {
        StatusResult  theResult{ noError };

        while (theResult && (0 < aTokenizer.remaining())) {
            Operand theLHS, theRHS;
            Token& theToken = aTokenizer.current();
            Logical theLogic;
            //parse logical keyword
            if (parseLogical(aTokenizer, theLogic)) {
                add(new Expression(theLogic));
                aTokenizer.next();
                continue;
            }
            //if the tokentype is punctuation: initialize the expression with "()"
            if (theToken.type != TokenType::identifier) {
                if (theToken.type == TokenType::punctuation) {
                    if ("(" == theToken.data) {
                        add(new Expression(Parentheses::left_paren));
                    }
                    if (")" == theToken.data) {
                        add(new Expression(Parentheses::right_paren));
                    }
                    aTokenizer.next();
                    continue;
                }
                else {
                    return theResult;
                }
            }
            
            if ((theResult = parseOperand(aTokenizer, anEntity, theLHS))) {
                Token& theToken = aTokenizer.current();
                if (theToken.type == TokenType::operators) {
                    Operators theOp = Helpers::toOperator(theToken.data);
                    aTokenizer.next();
                    if ((theResult = parseOperand(aTokenizer, anEntity, theRHS))) {
                        if (validateOperands(theLHS, theRHS, anEntity)) {
                            add(new Expression(theLHS, theOp, theRHS));
                            if (aTokenizer.skipIf(semicolon)) {
                                break;
                            }
                        }
                        else theResult.error = syntaxError;
                    }
                }
            }
            else theResult.error = syntaxError;
        }
        return theResult;
    }

}
