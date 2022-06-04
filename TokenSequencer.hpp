#ifndef TokenSequencer_hpp
#define TokenSequencer_hpp

#include <initializer_list>
#include <variant>
#include <functional>

#include "Tokenizer.hpp"


namespace ECE141 {

	class TokenSequencer;

	using TokenVar = std::variant<Keywords, Operators, TokenType>;
	using KwdList = std::initializer_list<Keywords>;
	//using CheckFcn = bool (*) (const Token&);
	using CheckFcn = std::function<bool(const Token&)>;

	enum class CheckCondition { isKeyword, isValue, isOperation, none };

	class TokenSequencer {
	public:	
		
		TokenSequencer(Tokenizer& aTokenizer);
		// Matching Operations
		TokenSequencer& matchNext(KwdList aMatchList);
        TokenSequencer& matchNext(Keywords aKeyword);
		TokenSequencer& hasOpt(Operators anOp);
		TokenSequencer& hasData(std::string aString);
    
		// Type Checking Operations
		TokenSequencer& isIdentifier();
		TokenSequencer& isOpt();
		TokenSequencer& isType(TokenType aType);

		// Get operations
		TokenSequencer& getOpt(Operators& anOpt);
		TokenSequencer& getData(std::string& aName);

		//Advanced Get Operations/ Parse Helper
		TokenSequencer& loopParse(std::vector<std::string>& aValueList, CheckFcn aSkipCondition,
			CheckFcn aStopCondition, CheckFcn aVerifyCondition);

		std::string     getData();
		Keywords        getKeyword();
		int             getValue();

		// Index Iterating Operations
		TokenSequencer& next(int anIndex=1);

		TokenSequencer& skipPast(char aChar);

		TokenSequencer& skipTo(char aChar);

		TokenSequencer& jumpBy(int anIndex);


		/// <summary>
		/// Resets tokenizer to last saved stated. Used to keep track 
		/// of already read tokens.
		/// </summary>
		/// <param name="anIndex"></param>
		/// <returns></returns>
		TokenSequencer& reset(uint64_t anIndex=-1);

		/// <summary>
		/// Conversion to bool for condition checking
		/// </summary>
		operator bool() { return status; }

		//-------------------------------------------------------
		//Save and restore index

		/// <summary>
		/// Stores current index value of tokenizer for future resets.
		/// </summary>
		TokenSequencer& saveCurrentIndex();

		/// <summary>
		/// Read until semicolon, then saves current index.
		/// </summary>
		/// <returns></returns>
		TokenSequencer& skipStatement();

		Tokenizer& tokenizer;

	protected:
		// return as bool
		bool status = true;

		//save state to not reread statements
		uint64_t savedIndex = 0;


		//Conditionals
		//bool isIdentifier(Token& aToken) { return aToken.type == TokenType::identifier; }
		//bool isOpt(Token& aToken) { return aToken.type == TokenType::operators; }
		//bool isKeyword(Token& aToken) { return aToken.type == TokenType::keyword; }

	};
}



#endif // !TokenSequencer_hpp
