#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "File_Entry.h"
#include "Parser.h"
#include "Tokenizer.h"
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for Directory class
class Directory;

class CommandHandler {
public:
    // Constructor accepts a pointer to the pointer of the current directory
    CommandHandler(Directory** currentDirPtr);

    // Execute the input command
    void executeCommand(const std::string& input, bool& isRunning);
    std::string toLower(const std::string& s);
    std::string toUpper(const std::string& s);

private:
    // Command-specific handlers
    void processAllCommandsHelp();
    void processOneCommandHelp(const std::string& command);
    void processCls();
    void processMd(const std::string& dirname);
    void processRd(const std::vector<std::string>& directories);
    void processCd(const std::string& dirname);
    
    void processQuit(bool& isRunning);
    
    void processDir(const std::string& path);
    void processTouch(const std::string& filePath);
    void processWrite(const std::string& filePath);
    void processType(const std::vector<std::string>& filePaths);
    void processDel(const std::vector<std::string>& targets);
    void processRename(const std::vector<std::string>& args);
    void processCopy(const std::vector<std::string>& args);
    void processExport(const std::vector<std::string>& args);
    void processImport(const std::vector<std::string>& args);

    // Helper methods
    Directory* navigateToDir(const std::string& path);
    File_Entry* navigateToFile(std::string& path);
    bool isValidFileName(const std::string& name);

    // Member variables
    std::unordered_map<std::string, std::pair<std::string, std::string>> commandHelp; // Updated name
    Directory** currentDirectoryPtr;
};

#endif // COMMANDHANDLER_H

