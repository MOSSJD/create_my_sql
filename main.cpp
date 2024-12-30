#include <iostream>
#include <set>
#include <functional>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>
#include "my_sql.hpp"
#include <fstream>
#include <iostream>
#include <string>

enum class DataType
{
    INT, DOUBLE, CHAR, VARCHAR, BOOL
};
const std::map<std::string, DataType> dataTypeAbbrs
{
    {"i", DataType::INT},
    {"d", DataType::DOUBLE},
    {"c", DataType::CHAR},
    {"vc", DataType::VARCHAR},
    {"b", DataType::BOOL}
};
const std::map<DataType, std::string> dataAbbrTypes
{
    {DataType::INT, "i"},
    {DataType::DOUBLE, "d"},
    {DataType::CHAR, "c"},
    {DataType::VARCHAR, "vc"},
    {DataType::BOOL, "b"}
};
const std::map<DataType, size_t> dataLengths
{
    {DataType::INT, 4},
    {DataType::DOUBLE, 8},
    {DataType::CHAR, 1},
    {DataType::BOOL, 1},
};
struct Field
{
    std::string name;
    DataType dataType;
    size_t dataLength;
    bool isNull;
    Field() = delete;
    Field(const std::string &name) : name(name) {}
    void print()
    {
        std::cout << "        "
                  << name << ' ' 
                  << isNull << ' ' 
                  << dataAbbrTypes.find(dataType)->second;
        if (!dataLengths.count(dataType))
            std::cout << ' ' << std::to_string(dataLength);
    }
};
struct Database;
struct Table
{
    std::string name;
    std::set<Field*> fields;
    Database *const database;
    Table() = delete;
    Table(const std::string &name, Database *const database)
        : name(name),
          database(database)
    {}
    ~Table()
    {
        for (auto &i : fields)
            delete i;
    }
    void print()
    {
        std::cout << "    " << name 
                  << "\n    {\n";
        for (const auto &i : fields)
        {
            i->print();
            if (i != *fields.rbegin())
                std::cout << ',';
            std::cout << '\n';
        }
        std::cout << "    }";
    }
};
struct Database
{
    std::string name;
    std::set<Table*> tables;
    Database() = delete;
    Database(const std::string &name) : name(name) {}
    ~Database()
    {
        for (auto &i : tables)
            delete i;
    }
    void print()
    {
        std::cout << name << "\n{\n";
        for (const auto &i : tables)
        {
            i->print();
            if (i != *tables.rbegin())
                std::cout << ',';
            std::cout << '\n';
        }
        std::cout << "}";
    }
};
int main()
{
    std::string line;
    const std::string DEFAULT_OPEN_FOLDER = "";

    // Read metadata
    std::set<Database*> databases;
    std::ifstream metadataStream("userdata/databases.txt");
    Database *tempDatabase = nullptr;
    auto getALine = [](std::ifstream &ifs, std::string &line) -> bool
    {
        std::getline(ifs, line);
        auto l_iter = std::find_if(line.begin(), line.end(), [](auto &a){ return a != ' '; });
        auto r_iter = std::find_if(line.rbegin(), line.rend(), [](auto &a){ return a != ' '; }).base();
        size_t pos = l_iter - line.begin();
        size_t len = r_iter - l_iter;
        line = line.substr(pos, len);
        std::transform(line.begin(), line.end(), line.begin(), tolower);
        return ifs.eof();
    };
    auto validName = [](const std::string &line) -> bool
    {
        if (!isalpha(line[0])) return false;
        return std::all_of(line.begin() + 1, line.end(), isalnum);
    };
    auto startWithLeftBrace = [getALine](std::ifstream &ifs, std::string &line) -> bool
    {
        return !getALine(ifs, line) && line == "{";
    };
    auto endWithComma = [](const std::string &line) -> bool
    {
        return line.back() == ',';
    };
    // read databases
    bool nextDatabase = false;
    while (!getALine(metadataStream, line))
    {
        nextDatabase = false;
        if (!validName(line))
            throw std::runtime_error("database name could only be comprised of alphabets");
        tempDatabase = new Database(line);
        if (!startWithLeftBrace(metadataStream, line))
            throw std::runtime_error("database could only start with '{'");
        Table *tempTable = nullptr;
        bool nextTable = false;
        // read tables
        while (!getALine(metadataStream, line))
        {
            nextTable = false;
            if (line == "}") // reached the end of the database
                break;
            else if (endWithComma(line))
            {
                if (line == "},") // indicates the next database
                {
                    nextDatabase = true;   
                    break;
                }
                else // table body end with a comma
                    throw std::runtime_error("table body end with a comma");
            }
            if (!validName(line))
                throw std::runtime_error("table name could only be comprised of alphabets");
            tempTable = new Table(line, tempDatabase);
            if (!startWithLeftBrace(metadataStream, line))
                throw std::runtime_error("table could only start with '{'");
            Field *tempField = nullptr;
            bool nextField = false;
            bool first = true;
            while (!getALine(metadataStream, line))
            {
                bool p_nextField = nextField;
                nextField = false;
                if (line == "}") // reached the end of the table
                    break;
                else if (line == "},") // indicates the next table
                {
                    nextTable = true;
                    break;
                }
                else if (first)
                {
                    if (endWithComma(line))
                    {
                        line.pop_back();
                        nextField = true;
                    }
                }
                else if (endWithComma(line)) // Is a field and not the first
                {
                    line.pop_back();
                    nextField = true;
                }
                else if (!p_nextField)
                    throw std::runtime_error("multiple field must be seperated by comma");
                first = false;
                std::stringstream ss(line);

                std::string tempName;
                // run out of field
                if (ss.eof())
                    throw std::runtime_error("bad field format");
                ss >> tempName;
                if (!validName(tempName))
                    throw std::runtime_error("field name could only be comprised of alphabets");
                tempField = new Field(tempName);

                std::string tempIsNull;
                // run out of field
                if (ss.eof())
                    throw std::runtime_error("bad field format");
                ss >> tempIsNull;
                if (tempIsNull.size() != 1 || !(tempIsNull.back() == '1' || tempIsNull.back() == '0'))
                    throw std::runtime_error("'isNull' could only be 0 or 1");
                tempField->isNull = tempIsNull == "0" ? false : true;
                
                // run out of field
                if (ss.eof())
                    throw std::runtime_error("bad field format");
                std::string tempDataAbbr;
                ss >> tempDataAbbr;
                if (!dataTypeAbbrs.count(tempDataAbbr))
                    throw std::runtime_error("invalid field data type");
                tempField->dataType = dataTypeAbbrs.find(tempDataAbbr)->second;

                if (dataLengths.count(tempField->dataType))
                    tempField->dataLength = dataLengths.find(tempField->dataType)->second;
                else
                {
                    // run out of field
                    if (ss.eof())
                        throw std::runtime_error("bad field format");
                    std::string tempDataLength;
                    ss >> tempDataLength;
                    if (tempDataLength.size() > 10)
                        throw std::length_error("data length too long");
                    tempField->dataLength = std::stoi(tempDataLength);
                }
                // not reading up all of the previous field
                if (!ss.eof())
                    throw std::runtime_error("bad field format");
                tempTable->fields.insert(tempField);
            }
            if (nextField)
                throw std::runtime_error("should have the other field");
            tempDatabase->tables.insert(tempTable);
        }
        if (nextTable)
            throw std::runtime_error("should have the other table");
        databases.insert(tempDatabase);
    }
    if (nextDatabase)
        throw std::runtime_error("should have the other database");

    auto printDatabases = [databases]()
    {
        for (const auto &i : databases)
        {
            i->print();
            if (i != *databases.rbegin())
                std::cout << ',';
            std::cout << '\n';
        }
    };
    printDatabases();

    const std::set<std::string> EXITCODES = {"exit", "quit"};
    std::cout << "This is the sql application. Please enter your instruction:\n";

    // Main circulation
    while (true)
    {
        // input
        std::getline(std::cin, line);
        // preprocessing
        std::transform(line.begin(), line.end(), line.begin(), tolower);
        int l = 0, r = line.size() - 1;
        while (l < line.size() && line[l] == ' ') l++;
        while (r >= l && line[r] == ' ') r--;
        line = line.substr(l, r - l + 1);
        // exit code
        if (EXITCODES.find(line) != EXITCODES.end())
        {
            std::cout << "bye.\n";
            break;
        }
        
    }
    return 0;
}