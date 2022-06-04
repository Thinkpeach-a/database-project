//
//  Tokenizer.hpp
//  Database
//
//  Created by rick gessner on 3/19/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef Tokenizer_hpp
#define Tokenizer_hpp

#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include "keywords.hpp"
#include "Scanner.hpp"

namespace ECE141 {

  using parseCallback = bool(char aChar);
  
  enum class TokenType {
    function, identifier, keyword, number, operators, timedate, punctuation, string, unknown
  };
  
  //-----------------
  
  struct Token {
    
    Token& operator=(const Token &aCopy) {
      type=aCopy.type;
      keyword=aCopy.keyword;
      data=aCopy.data;
      return *this;
    }
    
    TokenType   type=TokenType::unknown;
    Keywords    keyword=Keywords::unknown_kw;
    Operators   op=Operators::unknown_op;
    std::string data = "";
  };
 
  //-----------------
  
  class Tokenizer : public Scanner {
  public:
    Tokenizer(std::istream &anInputStream);
    
    StatusResult  tokenize();
    Token&        tokenAt(uint64_t anOffset);

    Token&        current();
    bool          more() {return index<size()-1;}
    bool          next(int anOffset=1);
    Token&        peek(int anOffset=1);
    void          restart() {index=0;}
    void          jumpTo(uint64_t anIndex) { index = anIndex; }
    uint64_t        size() {return tokens.size();}
    uint64_t        remaining() {return index<size() ? size()-index-1 :0;}

                          //these might consume a token...    
    bool          skipTo(Keywords aKeyword);
    bool          skipTo(TokenType aTokenType);

    bool          skipIf(Keywords aKeyword);
    bool          skipIf(Operators anOperator);
    bool          skipIf(TokenType aTokenType);
    bool          skipIf(char aChar);

    void          dump(); //utility
    uint64_t        getIndex() { return index; }

  protected:

    std::vector<Token>    tokens;    
    uint64_t                index;
  };
  
}

#endif /* Tokenizer_hpp */

