#ifndef DATABASE
#define DATABASE

#include "utils.hpp"
#include <iostream>
#include <map>
#include <string>

namespace me
{

    struct Field
    {
        std::string name;
        DataType dataType;
        size_t dataLength;
        bool isNull;
        friend std::ostream &operator<<(std::ostream &os, const Field &field);
        Field();
        ~Field();
        Field(const std::string &name, const DataType &dataType, const size_t &dataLength, const bool &isNull);
    };

    class Table
    {
    private:
        std::string name;
        std::map<std::string, Field> fields;

    public:
        friend std::ostream &operator<<(std::ostream &os, const Table &table);
        const std::string &getName() const;

    public:
        Table();
        Table(const std::string &name);
        ~Table();
        Field &addField(const Field &field);
    };

    class Database
    {
    private:
        std::string name;
        std::map<std::string, Table> tables;

    private:
        friend std::ostream &operator<<(std::ostream &os, const Database &database);

    public:
        Database();
        Database(const std::string &name);
        ~Database();

    public:
        Table &addTable(const Table &table);
        const std::string &getName() const;
    };
    class DatabaseManager
    {
    private:
        std::map<std::string, Database> databases;
        const std::string metadataFileDir;

    public:
        Database &addDatabase(const Database &database);
        DatabaseManager() = delete;
        DatabaseManager(const std::string &metadataFileDir);
        ~DatabaseManager();

    public:
        void save();
    };

}

#endif