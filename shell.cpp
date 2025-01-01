#include "shell.hpp"

namespace me
{

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