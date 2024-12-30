#ifndef __DATABASE_H__
#define __DATABASE_H__

#include<map>
#include<string>

namespace my{

enum class DataType
{
    INT, DOUBLE, CHAR, VARCHAR, BOOL
};

struct Field
{
    DataType dataType;
    size_t dataLength;
    bool isNull;
    Field() = delete;
};

struct Table
{
    std::map<std::string,Field> fields;
    Table() = delete;
};
struct Database
{
    std::map<std::string,Table> tables;
    Database() = delete;
};

}

#endif