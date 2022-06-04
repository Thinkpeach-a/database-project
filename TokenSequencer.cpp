#include "TokenSequencer.hpp"

#include <vector>

namespace ECE141 {
	//when initialize it, store the tokenizer and
    //the current index
	TokenSequencer::TokenSequencer(Tokenizer& aTokenizer) : tokenizer{aTokenizer} {
		savedIndex = tokenizer.getIndex();
	}
    
   //whether the next match the next keyword list
	TokenSequencer& TokenSequencer::matchNext(KwdList aMatchList) {
	
		if (status) {
			std::vector<Keywords> theMatchVec{ aMatchList };

			uint64_t theIndex = 0;
			uint64_t theVecLen = theMatchVec.size();
			while (status && tokenizer.more() && theIndex < theVecLen) {
				if (!(theMatchVec[theIndex] == tokenizer.current().keyword)) {
					status = false;
					break;
				}
				++theIndex;
				status = this->next();
			}
		}
		
		return *this;
	}
    TokenSequencer& TokenSequencer::matchNext(Keywords akeyword){
        if(status){
            if(!(akeyword==tokenizer.current().keyword)){
                status=false;
            }else{
                status=this->next();
            }
        }
        
        return *this;
     }
    
	TokenSequencer& TokenSequencer::hasOpt(Operators anOp) {
		if (status) {
			if (!(anOp == tokenizer.current().op)) {
				status = false;
			}
		}
		return *this;
	}

	TokenSequencer& TokenSequencer::hasData(std::string aString) {
		if (status) {
			if (!(aString == tokenizer.current().data)) {
				status = false;
			}
		}
		return *this;
	}

	TokenSequencer& TokenSequencer::isIdentifier() {
		if (status) {
			if (!(tokenizer.current().type == TokenType::identifier)) {
				status = false;
			}
		}
		return *this;

	}

	TokenSequencer& TokenSequencer::isOpt() {
		if (status) {
			if (!(tokenizer.current().type == TokenType::operators)) {
				status = false;
			}
		}

		return *this;
	}

	TokenSequencer& TokenSequencer::isType(TokenType aType){
		if (!(tokenizer.current().type == aType)) {
			status = false;
		}
		return *this;
	}

	TokenSequencer& TokenSequencer::getOpt(Operators& anOpt){
		if (status) {
			anOpt = tokenizer.current().op;
		}
		
		return *this;
	}

	TokenSequencer& TokenSequencer::getData(std::string& aName) {
		if (status) {
			aName = tokenizer.current().data;
		}

		return *this;
	}

	TokenSequencer& TokenSequencer::loopParse(std::vector<std::string>& aValueList, CheckFcn aSkipCondition,
		CheckFcn aStopCondition, CheckFcn aVerifyCondition) {
		Token theCurrentToken = tokenizer.current();

		while (!(aStopCondition(theCurrentToken))) {
			// Skip if specific condition
			if (aSkipCondition(theCurrentToken)) {}
			// Else Push back
			else if (aVerifyCondition(theCurrentToken)) {
				aValueList.push_back(theCurrentToken.data);
			}
			// Iterate tokenizer
			if (tokenizer.more()) {
				tokenizer.next();
				theCurrentToken = tokenizer.current();
			}
			else { break; }
		}
		return *this;
	}

	std::string TokenSequencer::getData()
	{
		return tokenizer.current().data;
	}

	Keywords TokenSequencer::getKeyword()
	{
		return tokenizer.current().keyword;
	}

	int TokenSequencer::getValue()
	{
		return std::stoi(getData());
	}
    
	TokenSequencer& TokenSequencer::next(int anIndex) {
		//if there are no more words remaining
        if (tokenizer.remaining() == 0) {
			status = false;
		}
		else if (!tokenizer.next(anIndex)) {
			status = false;
		}
		return *this;
	}

	TokenSequencer& TokenSequencer::skipPast(char aChar)
	{
		if (status) {
			while (next()) {
				auto theToken = tokenizer.current();
				if (theToken.type == TokenType::punctuation) {
					if (theToken.data[0] == aChar) {
						next();//skip it...
						return *this;
					}
				}
			}
			status = false;
		}
		return *this;
	}

	TokenSequencer& TokenSequencer::jumpBy(int anIndex) {
		if (status) {
			status = tokenizer.next(anIndex);
		}
		return *this;
	}

	TokenSequencer& TokenSequencer::reset(uint64_t anIndex) {
		if (anIndex >= 0 && anIndex < tokenizer.size() && anIndex < savedIndex) {
			savedIndex = anIndex;
		}
		status = true;
		tokenizer.jumpTo(savedIndex);
		return *this;
	}

	TokenSequencer& TokenSequencer::saveCurrentIndex(){
		savedIndex = tokenizer.getIndex();
		return *this;
	}

	TokenSequencer& TokenSequencer::skipStatement(){
		return reset().skipPast(';').saveCurrentIndex();
	}

}
