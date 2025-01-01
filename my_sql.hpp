#ifndef MY_SQL
#define MY_SQL

#include <iostream>
#include <set>
#include <functional>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>
#include <string>

namespace me
{

    enum class DataType;
    const std::map<std::string, DataType> dataTypeAbbrs;
    const std::map<DataType, std::string> dataAbbrTypes;
    const std::map<DataType, size_t> dataLengths;

    bool getALine(std::istream &ifs, std::string &line);
    bool validName(const std::string &line);
    bool startWithLeftBrace(std::ifstream &ifs, std::string &line);
    bool endWithComma(const std::string &line);

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
        DatabaseManager();
        DatabaseManager(const std::string &metadataFileDir);
        ~DatabaseManager();

    public:
        void save();
    };

    class Shell
    {
    private:
        static const std::string defaultMetadataDir;
        std::string metadataDir;
        me::DatabaseManager databaseManager;

    public:
        static const std::set<std::string> EXITCODES;
        static const std::string DEFAULT_OPEN_FOLDER;

    public:
        Shell();
        Shell(const std::string &metadataDir);
        ~Shell();

    public:
        void run();
    };
}

#endif