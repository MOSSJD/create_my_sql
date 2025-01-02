#ifndef SHELL
#define SHELL

#include "database.hpp"
#include <set>

namespace me
{

    class Shell
    {
    private:
        static const std::string defaultMetadataDir;
        static const std::string defaultRowdataDir;
        std::string metadataDir;
        std::string rowdataDir;
        me::DatabaseManager databaseManager;

    public:
        static const std::set<std::string> EXITCODES;
        static const std::string DEFAULT_OPEN_FOLDER;

    public:
        Shell();
        Shell(const std::string &metadataDir, const std::string &rowdataDir);
        ~Shell();

    public:
        void run();
    };
}

#endif