#ifndef UTILS
#define UTILS

#include <map>
#include <iostream>

namespace me
{

    enum class DataType
    {
        INT,
        DOUBLE,
        CHAR,
        VARCHAR,
        BOOL
    };
    const std::map<std::string, DataType> dataTypeAbbrs{
        {"i", DataType::INT},
        {"d", DataType::DOUBLE},
        {"c", DataType::CHAR},
        {"vc", DataType::VARCHAR},
        {"b", DataType::BOOL}};
    const std::map<DataType, std::string> dataAbbrTypes{
        {DataType::INT, "i"},
        {DataType::DOUBLE, "d"},
        {DataType::CHAR, "c"},
        {DataType::VARCHAR, "vc"},
        {DataType::BOOL, "b"}};
    const std::map<DataType, size_t> dataLengths{
        {DataType::INT, 4},
        {DataType::DOUBLE, 8},
        {DataType::CHAR, 1},
        {DataType::BOOL, 1}};

    bool getALine(std::istream &ifs, std::string &line);
    bool validName(const std::string &line);
    bool startWithLeftBrace(std::istream &ifs, std::string &line);
    bool endWithComma(const std::string &line);

}

#endif