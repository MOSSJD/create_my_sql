#include "utils.hpp"
#include <algorithm>

namespace me
{

    bool getALine(std::istream &ifs, std::string &line)
    {
        std::getline(ifs, line);
        auto l_iter = std::find_if(line.begin(), line.end(), [](auto &a)
                                   { return a != ' '; });
        auto r_iter = std::find_if(line.rbegin(), line.rend(), [](auto &a)
                                   { return a != ' '; })
                          .base();
        size_t pos = l_iter - line.begin();
        size_t len = r_iter - l_iter;
        line = line.substr(pos, len);
        std::transform(line.begin(), line.end(), line.begin(), tolower);
        return ifs.eof();
    }
    bool validName(const std::string &line)
    {
        if (!isalpha(line[0]))
            return false;
        return std::all_of(line.begin() + 1, line.end(), isalnum);
    }
    bool startWithLeftBrace(std::istream &ifs, std::string &line)
    {
        return !getALine(ifs, line) && line == "{";
    }
    bool endWithComma(const std::string &line)
    {
        return line.back() == ',';
    }

}