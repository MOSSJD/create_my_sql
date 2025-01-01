#include "database.hpp"
#include "my_sql.hpp"
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace me
{

    std::ostream &operator<<(std::ostream &os, const Field &field)
    {
        os << "        " << field.name << ' '
           << field.isNull << ' '
           << dataAbbrTypes.find(field.dataType)->second;
        if (!dataLengths.count(field.dataType))
            os << ' ' << std::to_string(field.dataLength);
        return os;
    }
    Field::Field() = default;
    Field::~Field() = default;
    Field::Field(const std::string &name, const DataType &dataType, const size_t &dataLength, const bool &isNull)
        : name(name),
          dataType(dataType),
          dataLength(dataLength),
          isNull(isNull)
    {
    }

    std::ostream &operator<<(std::ostream &os, const Table &table)
    {
        os << "    " << table.name << "\n    {\n";
        bool first = true;
        for (const auto &[name, field] : table.fields)
        {
            if (!first)
                os << ",\n";
            os << field;
            first = false;
        }
        os << '\n';
        return os;
    }
    const std::string &Table::getName() const
    {
        return name;
    }
    Table::Table() = default;
    Table::Table(const std::string &name) : name(name) {}
    Table::~Table() = default;
    Field &Table::addField(const Field &field)
    {
        fields[field.name] = field;
        return fields[field.name];
    }

    std::ostream &operator<<(std::ostream &os, const Database &database)
    {
        os << database.name << "\n{\n";
        bool first = true;
        for (const auto &[name, table] : database.tables)
        {
            if (!first)
                os << "    },\n";
            os << table;
            first = false;
        }
        os << "    }\n";
        return os;
    }
    Database::Database() = default;
    Database::Database(const std::string &name) : name(name) {}
    Database::~Database() = default;
    Table &Database::addTable(const Table &table)
    {
        tables[table.getName()] = table;
        return tables[table.getName()];
    }
    const std::string &Database::getName() const
    {
        return name;
    }
    Database &DatabaseManager::addDatabase(const Database &database)
    {
        databases[database.getName()] = database;
        return databases[database.getName()];
    }
    DatabaseManager::DatabaseManager(const std::string &metadataFileDir)
        : metadataFileDir(metadataFileDir)
    {
        std::ifstream metadataStream(metadataFileDir);
        if (!metadataStream.is_open())
            throw std::runtime_error("failed to open the metadata file");
        std::string line;
        bool nextDatabase = false;
        while (!getALine(metadataStream, line))
        {
            nextDatabase = false;
            if (!validName(line))
                throw std::runtime_error("database name could only be comprised of alphabets");
            Database &curDatabase = addDatabase(Database(line));
            if (!startWithLeftBrace(metadataStream, line))
                throw std::runtime_error("database could only start with '{'");
            bool nextTable = false;
            bool endOfTables = false;
            // read tables
            while (!getALine(metadataStream, line))
            {
                nextTable = false;
                if (line == "}") // reached the end of the database
                    break;
                else if (line == "},") // indicates the next database
                {
                    nextDatabase = true;
                    break;
                }
                else if (endOfTables)
                    throw std::runtime_error("multiple tables must be seperated by comma");

                if (!validName(line))
                    throw std::runtime_error("table name could only be comprised of alphabets");
                Table &curTable = curDatabase.addTable(Table(line));
                if (!startWithLeftBrace(metadataStream, line))
                    throw std::runtime_error("table could only start with '{'");
                bool nextField = false;
                bool firstField = true;
                while (!getALine(metadataStream, line))
                {
                    bool p_nextField = nextField;
                    nextField = false;
                    if (line == "}") // reached the end of the table
                    {
                        endOfTables = true;
                        break;
                    }
                    else if (line == "},") // indicates the next table
                    {
                        nextTable = true;
                        break;
                    }
                    else if (firstField)
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
                    firstField = false;
                    std::stringstream ss(line);

                    std::string tempName;
                    // run out of field
                    if (ss.eof())
                        throw std::runtime_error("bad field format");
                    ss >> tempName;
                    if (!validName(tempName))
                        throw std::runtime_error("field name could only be comprised of alphabets");

                    std::string tempIsNull;
                    // run out of field
                    if (ss.eof())
                        throw std::runtime_error("bad field format");
                    ss >> tempIsNull;
                    if (tempIsNull.size() != 1 || !(tempIsNull.back() == '1' || tempIsNull.back() == '0'))
                        throw std::runtime_error("'isNull' could only be 0 or 1");

                    // run out of field
                    if (ss.eof())
                        throw std::runtime_error("bad field format");
                    std::string tempDataAbbr;
                    ss >> tempDataAbbr;
                    if (!dataTypeAbbrs.count(tempDataAbbr))
                        throw std::runtime_error("invalid field data type");
                    DataType tempDataType = dataTypeAbbrs.find(tempDataAbbr)->second;

                    size_t tempDataLength;
                    if (dataLengths.count(tempDataType))
                        tempDataLength = dataLengths.find(tempDataType)->second;
                    else
                    {
                        // run out of field
                        if (ss.eof())
                            throw std::runtime_error("bad field format");
                        std::string tempDataLength;
                        ss >> tempDataLength;
                        if (tempDataLength.size() > 10)
                            throw std::length_error("data length too long");
                        tempDataLength = std::stoi(tempDataLength);
                    }
                    // not reading up all of the previous field
                    if (!ss.eof())
                        throw std::runtime_error("bad field format");

                    curTable.addField(Field(tempName, tempDataType, tempDataLength, tempIsNull == "1"));
                }
                if (nextField)
                    throw std::runtime_error("should have the other field");
            }
            if (nextTable)
                throw std::runtime_error("should have the other table");
        }
        if (nextDatabase)
            throw std::runtime_error("should have the other database");
        metadataStream.close();
    }
    DatabaseManager::~DatabaseManager()
    {
        save();
    }
    void DatabaseManager::save()
    {
        // std::ofstream metadataStream(metadataFileDir);
        std::ofstream metadataStream(metadataFileDir);
        bool first = true;
        for (const auto &[name, database] : databases)
        {
            if (!first)
                metadataStream << "},\n";
            metadataStream << database;
            first = false;
        }
        metadataStream << "}\n";
        metadataStream.close();
    }

}