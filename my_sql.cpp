#include "my_sql.hpp"

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
        {DataType::BOOL, 1},
    };

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
    bool startWithLeftBrace(std::ifstream &ifs, std::string &line)
    {
        return !getALine(ifs, line) && line == "{";
    }
    bool endWithComma(const std::string &line)
    {
        return line.back() == ',';
    }

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
    DatabaseManager::DatabaseManager() = delete;
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
        std::ofstream metadataStream("out.txt");
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

    const std::set<std::string> Shell::EXITCODES = {"exit", "quit"};
    const std::string Shell::DEFAULT_OPEN_FOLDER = "";
    Shell::Shell() : Shell(defaultMetadataDir) {}
    Shell::Shell(const std::string &metadataDir)
        : metadataDir(metadataDir),
          databaseManager(metadataDir)
    {
    }
    Shell::~Shell() = default;
    void Shell::run()
    {
        std::cout << "This is the sql application. Please enter your instruction:\n";

        std::string line;
        // Main circulation
        while (true)
        {
            getALine(std::cin, line);
            // exit code
            if (EXITCODES.find(line) != EXITCODES.end())
            {
                std::cout << "bye.\n";
                break;
            }
        }
    }
    const std::string Shell::defaultMetadataDir = "userdata/databases.txt";

}