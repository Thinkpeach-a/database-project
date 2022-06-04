#ifndef FormatString_hpp
#define FormatString_hpp

#include <regex>
#include <string>

namespace ECE141 {

    template<typename ... Args>
    std::string formatString(std::string aStr, Args ...anArgList) {
        // Lambda Fold expression, 
        // adapted from https://stackoverflow.com/questions/7230621/
        // how-can-i-iterate-over-a-packed-variadic-template-argument-list
        std::string theStr = aStr;
        std::regex theFindExpr("\\{(.*?)\\}");

        ([&](auto& anArg)
            {
                theStr = std::regex_replace(theStr, theFindExpr, std::to_string(anArg),
                    std::regex_constants::format_first_only);
            } (anArgList), ...);
        return theStr;
    }
}



#endif // !FormatString_hpp

