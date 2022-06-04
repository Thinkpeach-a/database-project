//
//  AboutUs.hpp
//  About your team members...
//
//  Created by rick gessner on 3/23/22.
//

#ifndef AboutMe_hpp
#define AboutMe_hpp

#include <stdio.h>
#include <string>
#include <optional>
#include <vector>

namespace ECE141 {

    using StringOpt = std::optional<std::string>;

    class AboutUs {
    private:
        std::vector<std::string> nameVector = { "James Chen", "Yida Zou" };
        std::vector<std::string> gitVector = { "jpchen01", "Thinkpeach-a" };

    public:
        AboutUs() = default;
        ~AboutUs() = default;

        size_t getTeamSize() { return 2; } //STUDENT: update this...

        StringOpt getName(size_t anIndex) const {
            //return name of student anIndex N (or nullopt)
            if (anIndex >= nameVector.size())
                return std::nullopt;
            return nameVector[anIndex];
        }
        StringOpt getGithubUsername(size_t anIndex) const {
            //return github username of student anIndex N (or nullopt)
            if (anIndex >= gitVector.size())
                return std::nullopt;
            return gitVector[anIndex];
        }

    };
}

#endif /* about_me */
