#include "Directory.h"
#include "Mini_FAT.h"
#include "File_Entry.h"
#include "Parser.h"
#include"CommandHandler.h"
#include <algorithm>
#include <cstring>
#include <cctype>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
CommandHandler::CommandHandler(Directory** currentDirPtr)
    : currentDirectoryPtr(currentDirPtr)
{
    // Basic utility commands first
    commandHelp["cls"] = {
        "Clears all visible output from the console.",
        "Usage:\n"
        "  cls\n\n"
        "Examples:\n"
        "  - Clear the screen: `cls`\n"
    };

    commandHelp["quit"] = {
        "Exits the application and terminates the session.",
        "Usage:\n"
        "  quit\n\n"
        "Examples:\n"
        "  - To exit: `quit`\n"
    };

    // Help command as a user-friendly feature
    commandHelp["help"] = {
        "Displays guidance for available commands.",
        "Usage:\n"
        "  help\n"
        "  help [command]\n\n"
        "Examples:\n"
        "  - To view all commands: `help`\n"
        "  - To view details of a command: `help md`\n"
    };

    // File operations
    commandHelp["touch"] = {
        "Generates an empty text file in the specified path.",
        "Usage:\n"
        "  touch [file_name]\n\n"
        "Examples:\n"
        "  - Create an empty file: `touch myfile.txt`\n"
        "  - Create an empty file in a folder: `touch /docs/myfile.txt`\n"
    };

    commandHelp["write"] = {
        "Opens a file for writing and adds content line by line.",
        "Usage:\n"
        "  write [file_name]\n\n"
        "Examples:\n"
        "  - Write content to a file: `write file1.txt`\n"
    };

    commandHelp["type"] = {
        "Displays the contents of a text file.",
        "Usage:\n"
        "  type [file_name]\n\n"
        "Examples:\n"
        "  - View file content: `type notes.txt`\n"
    };

    commandHelp["del"] = {
        "Removes specified files permanently.",
        "Usage:\n"
        "  del [file_name]+\n\n"
        "Examples:\n"
        "  - Delete a file: `del myfile.txt`\n"
        "  - Delete multiple files: `del file1.txt file2.txt`\n"
    };

    commandHelp["copy"] = {
        "Duplicates a file or directory to a new location.",
        "Usage:\n"
        "  copy [source] [destination]\n\n"
        "Examples:\n"
        "  - Copy a file: `copy file1.txt file2.txt`\n"
        "  - Copy a folder: `copy /myFolder /backupFolder`\n"
    };

    commandHelp["rename"] = {
        "Changes the name of a file or directory.",
        "Usage:\n"
        "  rename [current_name] [new_name]\n\n"
        "Examples:\n"
        "  - Rename a file: `rename old.txt new.txt`\n"
    };

    // Directory management
    commandHelp["md"] = {
        "Creates a new folder at the specified location.",
        "Usage:\n"
        "  md [directory_name]\n\n"
        "Examples:\n"
        "  - Create a folder: `md newFolder`\n"
        "  - Create a folder in a path: `md /path/to/newFolder`\n"
    };

    commandHelp["rd"] = {
        "Deletes one or more empty directories.",
        "Usage:\n"
        "  rd [directory_name]+\n\n"
        "Examples:\n"
        "  - Remove a directory: `rd myDir`\n"
        "  - Remove multiple directories: `rd dir1 dir2 dir3`\n"
    };

    commandHelp["cd"] = {
        "Changes the current working directory.",
        "Usage:\n"
        "  cd [path]\n\n"
        "Examples:\n"
        "  - Change to a specific folder: `cd myFolder`\n"
        "  - Navigate up: `cd ..`\n"
    };

    commandHelp["dir"] = {
        "Shows the list of files and subfolders in a directory.",
        "Usage:\n"
        "  dir\n"
        "  dir [path]\n\n"
        "Examples:\n"
        "  - View current directory: `dir`\n"
        "  - View a specific path: `dir /my/folder`\n"
    };

    commandHelp["import"] = {
        "Transfers a file from your physical machine to the virtual disk.",
        "Usage:\n"
        "  import [source_path] [destination_path]\n\n"
        "Examples:\n"
        "  - Import a file: `import myfile.txt /virtualFolder`\n"
    };

    commandHelp["export"] = {
        "Exports a file from the virtual disk to your machine.",
        "Usage:\n"
        "  export [file_path] [destination_path]\n\n"
        "Examples:\n"
        "  - Export a file: `export virtualFile.txt /downloads`\n"
    };
}
void CommandHandler::executeCommand(const string& input, bool& isRunning)
{
    // clean spaces from the input
    string cleanedInput = input;
    cleanedInput.erase(0, cleanedInput.find_first_not_of(" \t\n")); // clean leading spaces
    cleanedInput.erase(cleanedInput.find_last_not_of(" \t\n") + 1); // clean trailing spaces

    
    // Tokenize the trimmed input
    vector<string> tokens = Tokenizer::tokenize(cleanedInput);

    if (tokens.empty())
    {
        return; // No command entered
    }

    // Parse tokens into command and arguments
    Command parsedcmd = Parser::parse(tokens);

    // Convert command name to lowercase for case-insensitive comparison
    for (char& c : parsedcmd.name)
    {
        c = tolower(c);
    }

    // Now, use cmd.name and cmd.arguments as before
    if (parsedcmd.name == "help")
    {
        if (parsedcmd.arguments.empty())
        {
            processAllCommandsHelp();

        }
        else if (parsedcmd.arguments.size() == 1)
        {
            processOneCommandHelp(parsedcmd.arguments[0]);
        }
        else
        {
            cout << "Error: Invalid syntax for help command.\n";
            cout << "Usage:\n"
                << "  help\n"
                << "  help [command]\n";
        }
    }
    else if (parsedcmd.name == "del")
    {
        if (parsedcmd.arguments.empty())
        {
            cout << "Error: Invalid syntax for del command.\n";
            cout << "Usage: del [file|directory]+ (e.g., del file1.txt dir1 file2.txt)\n";
        }
        else
        {
            processDel(parsedcmd.arguments);
        }
    }
    else if (parsedcmd.name == "copy")
    {
        if (!parsedcmd.arguments.empty() && parsedcmd.arguments.size() <= 2) {
            processCopy(parsedcmd.arguments);
        }
        else
        {
            cout << "Error: Invalid syntax for copy command.\n";
            cout << "Usage:\n";
            cout << "  copy [source]\n";
            cout << "  copy [source] [destination]\n";
        }
    }
    else if (parsedcmd.name == "rename")
    {
        if (parsedcmd.arguments.size() == 2)
        {
            processRename(parsedcmd.arguments);
        }
        else
        {
            cout << "Error: Invalid syntax for rename command.\n";
            cout << "Usage: rename [fileName] [new fileName]\n";
        }
    }
    else if (parsedcmd.name == "type")
    {
        if (!parsedcmd.arguments.empty())
        {
            processType(parsedcmd.arguments);
        }
        else
        {
            cout << "Error: Invalid syntax for type command.\n";
            cout << "Usage: type [file_path]+ (one or more file paths)\n";
        }
    }
    else if (parsedcmd.name == "write")
    {
        if (parsedcmd.arguments.size() == 1)
        {
            processWrite(parsedcmd.arguments[0]);
        }
        else
        {
            cout << "Error: Invalid syntax for write command.\n";
            cout << "Usage: write [file_path] or [file_name]\n";
        }
    }
    else if (parsedcmd.name == "touch")
    {
        if (parsedcmd.arguments.size() == 1)
        {
            processTouch(parsedcmd.arguments[0]);
        }
        else
        {
            cout << "Error: Invalid syntax for touch command.\n";
            cout << "Usage: touch [file_path]\n";
        }
    }
    else if (parsedcmd.name == "dir")
    {
        if (parsedcmd.arguments.empty())
        {
            processDir("");
        }
        else if (parsedcmd.arguments.size() == 1)
        {
            processDir(parsedcmd.arguments[0]);
        }
        else
        {
            cout << "Error: Invalid syntax for dir command.\n";
            cout << "Usage:\n"
                << "  dir\n"
                << "  dir [path]\n";
        }
    }
    else if (parsedcmd.name == "import")
    {
        if (!parsedcmd.arguments.empty())
        {
            processImport(parsedcmd.arguments);
        }
        else
        {
            cout << "Error: Invalid syntax for import command.\n";
            cout << "Usage:\n  import [source]\n  import [source] [destination]\n";
        }
    }
    else if (parsedcmd.name == "md")
    {
        if (parsedcmd.arguments.size() == 1)
        {
            processMd(parsedcmd.arguments[0]);
        }
        else
        {
            cout << "Error: Invalid syntax for md command.\n";
            cout << "Usage: md [directory_name]\n";
        }
    }
    else if (parsedcmd.name == "cd")
    {
        if (parsedcmd.arguments.size() <= 1)
        {
            string path = "";
            if (parsedcmd.arguments.size() == 1)
            {
                path = parsedcmd.arguments[0];
            }
            processCd(path);
        }
        else
        {
            cout << "Error: Invalid syntax for cd command.\n";
            cout << "Usage:\n"
                << "  cd\n"
                << "  cd [directory]\n";
        }
    }
    else if (parsedcmd.name == "cls")
    {
        if (parsedcmd.arguments.empty())
        {
            processCls();
        }
        else
        {
            cout << "Error: Unknown command '" << cleanedInput << "'. Type 'help' to see available commands.\n";
        }
    }
    else if (parsedcmd.name == "export")
    {
        processExport(parsedcmd.arguments);
    }
    else if (parsedcmd.name == "rd")
    {
        if (parsedcmd.arguments.empty())
        {
            cout << "Error: Invalid syntax for rd command.\n";
            cout << "Usage: rd [directory]+\n";
        }
        else
        {
            processRd(parsedcmd.arguments);
        }
    }
    else if (parsedcmd.name == "quit")
    {
        if (parsedcmd.arguments.empty())
        {
            processQuit(isRunning);
        }
        else
        {
            cout << "Error: Incorrect syntax for 'quit' command.\n";
            cout << "Usage: quit\n";
        }
    }
    else
    {
        cout << "Error: Unknown command '" << parsedcmd.name << "'. Type 'help' to see available commands.\n";
    }
}
void CommandHandler::processAllCommandsHelp()
{
    cout << "Here are the commands you can use:\n";
    cout << "=================================\n";

    int commandIndex = 1;
    for (const auto& [commandName, commandDetails] : commandHelp)
    {
        cout << "  " << commandIndex << ". " << commandName
            << " - " << commandDetails.first << "\n";
        commandIndex++;
    }

    cout << "=================================\n";
    cout << "Use 'help [command]' to learn more about a specific command.\n";
}
void CommandHandler::processOneCommandHelp(const string& command)
{
    // Normalize command to lowercase for case-insensitive matching
    string normalizedCommand = command;
    transform(normalizedCommand.begin(), normalizedCommand.end(), normalizedCommand.begin(),
        [](unsigned char c) { return tolower(c); });

    // Search for the command in the help map
    auto commandEntry = commandHelp.find(normalizedCommand);
    if (commandEntry != commandHelp.end())
    {
        cout << "Detailed Help for the '" << command << "' Command:\n";
        cout << commandEntry->second.second << "\n";
    }
    else
    {
       
        cout << "The command '" << command
            << "' is not recognized. Please ensure it is spelled correctly or use 'help' to see the full list of available commands.\n";
    }
}
std::string CommandHandler::toLower(const std::string& input)
{
    // Create a lowercase version of the input string
    std::string lowercaseResult = input;
    std::transform(lowercaseResult.begin(), lowercaseResult.end(), lowercaseResult.begin(),
        [](unsigned char character) { return std::tolower(character); });

    return lowercaseResult;
}
std::string CommandHandler::toUpper(const std::string& input)
{
    // Create an uppercase version of the input string
    std::string uppercaseResult = input;
    std::transform(uppercaseResult.begin(), uppercaseResult.end(), uppercaseResult.begin(),
        [](unsigned char character) { return std::toupper(character); });

    return uppercaseResult;
}
void CommandHandler::processCls()
{
    // On Windows, use system("cls") to clear the screen
    system("cls");
}
void CommandHandler::processMd(const string& dirPath)
{
    // Step 1: Trim spaces from the input
    string trimmedPath = dirPath;
    trimmedPath.erase(0, trimmedPath.find_first_not_of(" \t\r\n")); // Trim leading spaces
    trimmedPath.erase(trimmedPath.find_last_not_of(" \t\r\n") + 1); // Trim trailing spaces

    // Validate input
    if (trimmedPath.empty())
    {
        cout << "Error: Invalid syntax for md command.\n";
        cout << "Usage: md [directory_name]\n";
        return;
    }

    // Step 2: Parse the path into parent path and directory name
    string parentPath;
    string dirName;

    size_t lastSlash = trimmedPath.find_last_of("/\\");
    if (lastSlash == string::npos)
    {
        parentPath = "";
        dirName = trimmedPath; // The directory will be created in the current directory
    }
    else
    {
        parentPath = trimmedPath.substr(0, lastSlash);
        dirName = trimmedPath.substr(lastSlash + 1);
    }

    // Step 3: Navigate to the parent directory
    Directory* parentDir = nullptr;
    parentDir = parentPath.empty() ? *currentDirectoryPtr : navigateToDir(parentPath);

    if (!parentDir)
    {
        cout << "Error: Directory path '" << parentPath << "' does not exist.\n";
        return;
    }

    // Step 4: Check for duplicate directory name
    for (const auto& entry : parentDir->DirOrFiles)
    {
        if (!entry.getIsFile())
        {
            string existingName = entry.getName();
            string newName = dirName;

            // Normalize names for case-insensitive comparison
            std::transform(existingName.begin(), existingName.end(), existingName.begin(),
                [](unsigned char c) { return std::tolower(c); });
            std::transform(newName.begin(), newName.end(), newName.begin(),
                [](unsigned char c) { return std::tolower(c); });

            if (existingName == newName)
            {
                cout << "Error: Directory '" << dirName << "' already exists.\n";
                return;
            }
        }
    }

    // Step 5: Allocate a cluster for the new directory
    int newCluster = Mini_FAT::getAvailableCluster();
    if (newCluster == -1)
    {
        cout << "Error: No available clusters to create directory.\n";
        return;
    }

    // Step 6: Initialize and clean up the directory name
    Mini_FAT::setClusterPointer(newCluster, -1); // Mark cluster as EOF
    string cleanedName = Directory_Entry::cleanTheName(dirName);
    if (cleanedName.empty())
    {
        cout << "Error: Invalid directory name.\n";
        return;
    }

    // Step 7: Create the new directory
    Directory* newDir = new Directory(cleanedName, 0x10, newCluster, parentDir);
    newDir->readDirectory(); // Initialize with '.' and '..'

    // Step 8: Create a Directory_Entry object for the new directory
    Directory_Entry newDirEntry(cleanedName, 0x10, newCluster);
    newDirEntry.subDirectory = newDir; // Associate the entry with the new directory

    // Step 9: Add the new directory entry to the parent directory
    parentDir->DirOrFiles.push_back(newDirEntry);

    // Step 10: Write changes to the parent directory
    parentDir->writeDirectory();

    cout << "Directory '" << cleanedName << "' created successfully.\n";
}
void CommandHandler::processRd(const vector<string>& directories)
{
    if (directories.empty())
    {
        cout << "Error: Invalid syntax for rd command.\n";
        cout << "Usage: rd [directory]+\n";
        return;
    }

    // Iterate over each directory argument
    for (const auto& dirPath : directories)
    {
        // Step 1: Confirm deletion
        cout << "Are you sure you want to delete the directory '" << dirPath << "'? (y/n): ";
        char choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Flush remaining input

        if (choice != 'y' && choice != 'Y')
        {
            cout << "Skipped deletion of '" << dirPath << "'.\n";
            continue;
        }

        // Step 2: Extract parent path and directory name
        string parentPath, dirName;
        size_t lastSlash = dirPath.find_last_of("/\\");
        if (lastSlash == string::npos)
        {
            parentPath = "";  // Directory in the current directory
            dirName = dirPath;
        }
        else
        {
            parentPath = dirPath.substr(0, lastSlash);
            dirName = dirPath.substr(lastSlash + 1);
        }

        // Step 3: Locate the parent directory
        Directory* parentDir = parentPath.empty() ? *currentDirectoryPtr : navigateToDir(parentPath);

        if (!parentDir)
        {
            cout << "Error: Parent directory '" << parentPath << "' does not exist.\n";
            continue;
        }

        // Step 4: Search for the target directory
        int dirIndex = parentDir->searchDirectory(dirName);
        if (dirIndex == -1)
        {
            cout << "Error: Directory '" << dirName << "' does not exist within the specified path.\n";
            continue;
        }

        Directory_Entry dirEntry = parentDir->DirOrFiles[dirIndex];
        if (dirEntry.dir_attr != 0x10) // 0x10 indicates a directory
        {
            cout << "Error: '" << dirName << "' is not a valid directory.\n";
            continue;
        }

        // Step 5: Check if the directory is empty
        if (!dirEntry.subDirectory->isEmpty())
        {
            cout << "Error: Directory '" << dirPath << "' is not empty. Please remove its contents first.\n";
            continue;
        }

        // Step 6: Delete the directory
        delete dirEntry.subDirectory; // Free memory
        parentDir->DirOrFiles.erase(parentDir->DirOrFiles.begin() + dirIndex);
        parentDir->writeDirectory();

        cout << "Directory '" << dirPath << "' was successfully deleted.\n";
    }
}
void CommandHandler::processCd(const string& path)
{
    if (path.empty())
    {
        // Display current directory
        cout << "Current Directory: " << (*currentDirectoryPtr)->getFullPath() << "\n";
        return;
    }

    // Handle special directories '.' and '..'
    if (path == ".")
    {
        // Do nothing
        cout << "Navigating to current directory (no change).\n";
        return;
    }
    else if (path == "..")
    {
        if ((*currentDirectoryPtr)->parent != nullptr)
        {
            *currentDirectoryPtr = (*currentDirectoryPtr)->parent;
            cout << "Changed directory to: " << (*currentDirectoryPtr)->getFullPath() << "\n";
        }
        else
        {
            cout << "Error: Already at the root directory.\n";
        }
        return;
    }

    // Determine if the path is absolute
    bool isAbsolute = false;
    Directory* traversalDir = *currentDirectoryPtr;
    string drive = "";
    size_t startIndex = 0;

    // Check if the path starts with a drive letter, e.g., "C:\"
    if (path.length() >= 3 && isalpha(path[0]) && path[1] == ':' && path[2] == '\\')
    {
        isAbsolute = true;
        drive = path.substr(0, 2); // e.g., "C:"

        // Convert drive to uppercase for case-insensitive comparison
        drive = toUpper(drive);

        // Traverse up to the root directory
        while (traversalDir->parent != nullptr)
        {
            traversalDir = traversalDir->parent;
        }

        // Debugging Statements

        // Verify the drive letter matches (case-insensitive)
        string traversalDrive = toUpper(traversalDir->name.substr(0, 2));
        if (traversalDrive != drive)
        {
            cout << "Error: Drive '" << drive << "' not found.\n";
            return;
        }

        // Update the path to remove the drive part
        // Example: "C:\omar\omar1" becomes "omar\omar1"
        string updatedPath = path.substr(3); // Skip "C:\"
        // Split the updated path
        vector<string> pathComponents;
        string component;
        stringstream ss(updatedPath);
        while (getline(ss, component, '\\'))
        {
            if (!component.empty())
                pathComponents.push_back(component);
        }

        // Start traversing from the root
        for (const auto& dirName : pathComponents)
        {
            if (dirName == ".")
            {
                // Current directory: do nothing
                continue;
            }
            else if (dirName == "..")
            {
                if (traversalDir->parent != nullptr)
                {
                    traversalDir = traversalDir->parent;
                }
                else
                {
                    cout << "Error: Already at the root directory.\n";
                    return;
                }
            }
            else
            {
                // Search for the specified subdirectory
                int dirIndex = traversalDir->searchDirectory(dirName);
                if (dirIndex == -1)
                {
                    cout << "Error: System cannot find the specified folder '" << dirName << "'.\n";
                    return;
                }

                // Get the Directory_Entry object
                Directory_Entry* subDirEntry = &traversalDir->DirOrFiles[dirIndex];
                if (subDirEntry->dir_attr != 0x10) // 0x10 indicates a directory
                {
                    cout << "Error: '" << dirName << "' is not a directory.\n";
                    return;
                }

                // Assign traversalDir to the existing subDirectory
                traversalDir = subDirEntry->subDirectory;
            }
        }

        // Update the current directory pointer to traversalDir
        *currentDirectoryPtr = traversalDir;
        cout << "Changed directory to: " << (*currentDirectoryPtr)->getFullPath() << "\n";
        return;
    }

    // If the path is not absolute, split it normally
    vector<string> pathComponents;
    string componentNonAbsolute;
    stringstream ssNonAbsolute(path);
    while (getline(ssNonAbsolute, componentNonAbsolute, '\\'))
    {
        if (!componentNonAbsolute.empty())
            pathComponents.push_back(componentNonAbsolute);
    }

    // Traverse the path components
    bool errorOccurred = false;
    for (const auto& dirName : pathComponents)
    {
        if (dirName == ".")
        {
            // Current directory: do nothing
            continue;
        }
        else if (dirName == "..")
        {
            // Move to parent directory
            if (traversalDir->parent != nullptr)
            {
                traversalDir = traversalDir->parent;
            }
            else
            {
                cout << "Error: Already at the root directory.\n";
                errorOccurred = true;
                break;
            }
        }
        else
        {
            // Move to specified subdirectory
            int dirIndex = traversalDir->searchDirectory(dirName);
            if (dirIndex == -1)
            {
                cout << "Error: System cannot find the specified folder '" << dirName << "'.\n";
                errorOccurred = true;
                break;
            }

            // Get the Directory_Entry object
            Directory_Entry* subDirEntry = &traversalDir->DirOrFiles[dirIndex];
            if (subDirEntry->dir_attr != 0x10) // 0x10 indicates a directory
            {
                cout << "Error: '" << dirName << "' is not a directory.\n";
                errorOccurred = true;
                break;
            }

            // Assign traversalDir to the existing subDirectory
            traversalDir = subDirEntry->subDirectory;
        }
    }

    if (!errorOccurred)
    {
        // Update the current directory pointer to traversalDir
        *currentDirectoryPtr = traversalDir;
        cout << "Changed directory to: " << (*currentDirectoryPtr)->getFullPath() << "\n";
    }
}
void CommandHandler::processQuit(bool& isRunning)
{
    cout << "\n================================================================================================================\n";
    cout << "                                               Exiting the Shell                                                \n";
    cout << "================================================================================================================\n";
    cout << "                                          Thank you for using the Shell.                                                                               \n";
    cout << "================================================================================================================\n";

    isRunning = false; // Terminate the shell loop
}
File_Entry* CommandHandler::navigateToFile(string& path)
{
    // Split the given path into directory path and file name
    size_t lastBackslash = path.find_last_of('\\');
    if (lastBackslash == string::npos) {
        cout << "Error: The provided file path is invalid. Please ensure the format is correct.\n";
        return nullptr;
    }

    string dirPath = path.substr(0, lastBackslash);
    string fileName = path.substr(lastBackslash + 1);

    if (fileName.empty()) {
        cout << "Error: The file name is missing. Please provide a valid file name.\n";
        return nullptr;
    }

    // Navigate to the target directory
    Directory* targetDir = navigateToDir(dirPath);
    if (targetDir == nullptr) {
        // The navigateToDir function already prints an appropriate error message
        return nullptr;
    }

    // Search for the file in the directory
    int fileIndex = targetDir->searchDirectory(fileName);
    if (fileIndex == -1) {
        cout << "Error: The file '" << fileName << "' could not be found in the directory '"
            << targetDir->getFullPath() << "'.\n";
        return nullptr;
    }

    Directory_Entry& fileEntry = targetDir->DirOrFiles[fileIndex];
    if (fileEntry.dir_attr == 0x10) { // 0x10 indicates the entry is a directory
        cout << "Error: The specified entry '" << fileName << "' is a directory, not a file.\n";
        return nullptr;
    }

    // Create a File_Entry object and read its content
    File_Entry* file = new File_Entry(fileEntry, targetDir);
    file->readFileContent();

    cout << "Success: The file '" << fileName << "' has been located and its content loaded.\n";
    return file;
}
Directory* CommandHandler::navigateToDir(const std::string& path) {
    // Make a local copy of the input path and normalize path separators to '\\'
    std::string normalizedPath = path;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '/', '\\');

    // Split the path by '\\'
    std::vector<std::string> pathComponents;
    std::stringstream pathStream(normalizedPath);
    std::string component;

    while (std::getline(pathStream, component, '\\')) {
        if (!component.empty()) {
            pathComponents.push_back(component);
        }
    }

    // Handle empty path error
    if (pathComponents.empty()) {
        std::cout << "Error: The provided path is empty. Please specify a valid path.\n";
        return nullptr;
    }

    // Start traversal at the current directory
    Directory* currentDir = *currentDirectoryPtr;

    // Handle root directory navigation (e.g., "C:")
    std::string rootDrive = toUpper(currentDir->getDrive()) + ":";
    if (toUpper(pathComponents[0]) == toUpper(rootDrive)) {
        while (currentDir->parent != nullptr) {
            currentDir = currentDir->parent; // Move to the root directory
        }
        pathComponents.erase(pathComponents.begin()); // Remove the root drive from path components
    }

    // Traverse the directory path
    for (const auto& dirName : pathComponents) {
        int dirIndex = currentDir->searchDirectory(dirName);

        // Handle directory not found
        if (dirIndex == -1) {
            std::cout << "Error: The directory '" << dirName
                << "' was not found in '" << currentDir->getFullPath() << "'.\n";
            return nullptr;
        }

        Directory_Entry& entry = currentDir->DirOrFiles[dirIndex];

        // Handle invalid directory entry
        if (entry.dir_attr != 0x10) { // 0x10 represents a directory
            std::cout << "Error: '" << dirName << "' is not a directory. Ensure you provide a valid directory path.\n";
            return nullptr;
        }

        // Move to the subdirectory
        currentDir = entry.subDirectory;
        if (!currentDir) {
            std::cout << "Error: Unable to access the subdirectory '" << dirName << "'.\n";
            return nullptr;
        }

        // Notify successful navigation
        std::cout << "Navigated to: " << currentDir->getFullPath() << "\n";
    }

    return currentDir; // Return the final directory after traversal
}
void CommandHandler::processDir(const std::string& path) {
    // Identify the target directory based on the input path
    Directory* targetDir = *currentDirectoryPtr;

    // Handle special cases: "." (current directory) and ".." (parent directory)
    if (path == ".") {
        // Stay in the current directory
    }
    else if (path == "..") {
        if (targetDir->parent != nullptr) {
            targetDir = targetDir->parent;
        }
        else {
            std::cout << "Error: You are already at the root directory and cannot go higher.\n";
            return;
        }
    }
    else if (!path.empty()) {
        Directory* resolvedDir = navigateToDir(path);
        if (!resolvedDir) {
            // Error already displayed by navigateToDir
            return;
        }
        targetDir = resolvedDir;
    }

    // Display the directory header
    std::cout << "Contents of Directory: " << targetDir->getFullPath() << "\n\n";

    // Separate and classify directory contents
    std::vector<Directory_Entry> directories, files;
    int dirCount = 0, fileCount = 0;
    long long totalSize = 0; // Sum of file sizes

    for (const auto& entry : targetDir->DirOrFiles) {
        if (entry.dir_attr == 0x10) { // Directory
            if (entry.getName() == "." || entry.getName() == "..") {
                continue; // Skip special directories
            }
            directories.push_back(entry);
        }
        else { // File
            files.push_back(entry);
        }
    }

    // Sort directories and files alphabetically
    std::sort(directories.begin(), directories.end(), [](const Directory_Entry& a, const Directory_Entry& b) {
        return a.getName() < b.getName();
        });
    std::sort(files.begin(), files.end(), [](const Directory_Entry& a, const Directory_Entry& b) {
        return a.getName() < b.getName();
        });

    // Define formatting for display
    const int nameWidth = 40;
    const int sizeWidth = 15;

    // Display table headers
    std::cout << std::left << std::setw(nameWidth) << "Name"
        << std::right << std::setw(sizeWidth) << "Size\n";
    std::cout << std::left << std::setw(nameWidth) << "----"
        << std::right << std::setw(sizeWidth) << "----\n";

    // Display directories
    for (const auto& d : directories) {
        std::string name = d.getName();
        if (name.empty()) {
            name = "<Unnamed Directory>";
        }
        std::cout << std::left << std::setw(nameWidth) << (name + " <DIR>")
            << std::right << std::setw(sizeWidth) << "-" << "\n";
        dirCount++;
    }

    // Display files
    for (const auto& f : files) {
        std::string name = f.getName();
        if (name.empty()) {
            name = "<Unnamed File>";
        }
        int size = f.getSize();
        std::cout << std::left << std::setw(nameWidth) << name
            << std::right << std::setw(sizeWidth) << size << " bytes\n";
        fileCount++;
        totalSize += size;
    }

    // Calculate free space using FAT information
    long long freeClusters = Mini_FAT::getFreeClusters();
    long long clusterSize = Mini_FAT::getClusterSize();
    long long freeSpace = freeClusters * clusterSize;

    // Display summary
    std::cout << "\nSummary:\n"
        << fileCount << " File(s)\t" << totalSize << " bytes\n"
        << dirCount << " Dir(s)\t" << freeSpace << " bytes free\n";
}
void CommandHandler::processTouch(const std::string& filePath) {
    // Trim leading and trailing whitespace from the input
    std::string trimmedPath = filePath;
    trimmedPath.erase(0, trimmedPath.find_first_not_of(" \t\r\n")); // Trim leading spaces
    trimmedPath.erase(trimmedPath.find_last_not_of(" \t\r\n") + 1); // Trim trailing spaces

    // Validate if the input is empty
    if (trimmedPath.empty()) {
        std::cout << "Error: Missing file path.\n"
            << "Usage: touch [file_path]\n";
        return;
    }

    // Split the path into the parent directory and file name
    std::string parentPath, fileName;
    size_t lastSlash = trimmedPath.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        parentPath = ""; // Current directory
        fileName = trimmedPath;
    }
    else {
        parentPath = trimmedPath.substr(0, lastSlash);
        fileName = trimmedPath.substr(lastSlash + 1);
    }

    // Validate the file name
    if (!isValidFileName(fileName)) {
        std::cout << "Error: The file name '" << fileName << "' is invalid.\n";
        return;
    }

    // Automatically append ".txt" if the file name lacks a valid extension
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == fileName.length() - 1) {
        fileName += ".txt";
    }

    // Navigate to the parent directory
    Directory* parentDir = nullptr;
    if (parentPath.empty()) {
        parentDir = *currentDirectoryPtr; // Default to current directory
    }
    else {
        parentDir = navigateToDir(parentPath);
        if (parentDir == nullptr) {
            std::cout << "Error: The directory '" << parentPath << "' does not exist.\n";
            return;
        }
    }

    // Check if a file with the same name already exists (case-insensitive)
    for (const auto& entry : parentDir->DirOrFiles) {
        std::string existingName = entry.getName();
        std::string newName = fileName;

        // Convert both names to lowercase for case-insensitive comparison
        std::transform(existingName.begin(), existingName.end(), existingName.begin(),
            [](unsigned char c) { return std::tolower(c); });
        std::transform(newName.begin(), newName.end(), newName.begin(),
            [](unsigned char c) { return std::tolower(c); });

        if (existingName == newName) {
            std::cout << "Error: A file named '" << fileName << "' already exists in this directory.\n";
            return;
        }
    }

    // Create a new file entry with the specified name and mark it as a file
    Directory_Entry newFileEntry(fileName, 0x00, /*firstCluster=*/0);
    newFileEntry.setIsFile(true); // Explicitly mark as a file

    // Add the new file entry to the parent directory
    parentDir->DirOrFiles.push_back(newFileEntry);

    // Save the updated directory contents to the virtual disk
    parentDir->writeDirectory();

    // Confirmation message
    std::cout << "File '" << newFileEntry.getName() << "' created successfully in '"
        << parentDir->getFullPath() << "'.\n";
}
void CommandHandler::processWrite(const string& filePath) {
    // 1. Parse the path to separate parent path and file name
    string parentPath;
    string fileName;

    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash == string::npos) {
        // If no slash found, the file resides in the current directory
        parentPath = "";
        fileName = filePath;
    }
    else {
        // Split the path into parent directory and file name
        parentPath = filePath.substr(0, lastSlash);
        fileName = filePath.substr(lastSlash + 1);
    }

    // 2. Validate file name
    if (!isValidFileName(fileName)) {
        cout << "Error: '" << fileName << "' is not a valid file name.\n";
        return;
    }

    // 3. Navigate to the parent directory
    Directory* parentDir = nullptr;
    if (parentPath.empty()) {
        // Use current directory if no parent path is specified
        parentDir = *currentDirectoryPtr;
    }
    else {
        // Resolve the specified directory path
        parentDir = navigateToDir(parentPath);
    }

    if (parentDir == nullptr) {
        // If parent directory cannot be resolved
        cout << "Error: Directory path '" << parentPath << "' does not exist.\n";
        return;
    }

    // 4. Search for the file in the parent directory
    bool fileFound = false;
    string lowerFileName = toLower(fileName); // Convert file name to lowercase for case-insensitive matching

    for (auto& entry : parentDir->DirOrFiles) { // Iterate through directory entries
        if (toLower(entry.getName()) == lowerFileName) { // Compare file names in lowercase
            if (!entry.getIsFile()) {
                // If the entry is a directory, not a file
                cout << "Error: '" << fileName << "' is a directory, not a file.\n";
                return;
            }

            // 5. Prompt user for input
            cout << "Enter text to write to '" << fileName << "'. Type 'END' on a new line to finish:\n";

            string line;
            string newContent;
            while (true) {
                getline(cin, line); // Read user input line by line
                if (line == "END") // Exit loop if user types 'END'
                    break;
                newContent += line + "\n"; // Append line to the file content
            }

            // 6. Update the content
            entry.setContent(newContent); // Update the content of the file entry

            // 7. Persist changes to disk
            parentDir->writeDirectory(); // Write directory changes to the virtual disk

            cout << "Content successfully written to '" << fileName << "'.\n";
            fileFound = true; // Mark the file as found and processed
            break;
        }
    }

    if (!fileFound) {
        // If no matching file is found in the parent directory
        cout << "Error: File '" << fileName << "' does not exist.\n";
    }
}
bool CommandHandler::isValidFileName(const std::string& name) {
    // 1. Validate the length of the name
    // Files and directories must have names between 1 and 11 characters (8.3 format or directory name).
    if (name.empty() || name.length() > 11) {
        return false; // Name is either too short or exceeds the allowed length.
    }

    // 2. Check for invalid characters
    // Characters such as \, *, ?, ", <, >, | are not allowed in file/directory names.
    const std::string invalidChars = R"(/\*?"<>|)";
    for (char c : name) {
        if (invalidChars.find(c) != std::string::npos) {
            return false; // Name contains invalid characters.
        }
    }

    // 3. Prevent names starting with a dot
    // File or directory names cannot begin with a dot (e.g., ".hidden" is not allowed).
    if (name[0] == '.') {
        return false; // Leading dot is not allowed.
    }

    // 4. Enforce 8.3 naming convention for files
    // Files should have a base name (up to 8 characters) and an optional extension (up to 3 characters).
    size_t dotPos = name.find_last_of('.');
    if (dotPos != std::string::npos) {
        // Split the name into base and extension
        std::string base = name.substr(0, dotPos);
        std::string ext = name.substr(dotPos + 1);

        // Check if base and extension are valid
        if (base.empty() || ext.empty() || base.length() > 8 || ext.length() > 3) {
            return false; // Either base or extension violates the 8.3 format.
        }
    }
    else {
        // 5. Handle directories without an extension
        // Directories are allowed up to 11 characters with no extension.
        if (name.length() > 11) {
            return false; // Directory name exceeds the allowed limit.
        }
    }

    // If all checks are passed, the name is valid
    return true;
}
void CommandHandler::processType(const vector<string>& filePaths) {
    // Validate input: Ensure at least one file path is provided
    if (filePaths.empty()) {
        cout << "Error: No file paths provided.\n";
        cout << "Usage: type [file_path]+ (one or more file paths)\n";
        return;
    }

    // Iterate through each provided file path
    for (const string& filePath : filePaths) {
        // Step 1: Parse the file path into parent path and file name
        string parentPath;
        string fileName;

        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash == string::npos) {
            // If no slash found, assume the file is in the current directory
            parentPath = "";
            fileName = filePath;
        }
        else {
            // Split the path into parent directory and file name
            parentPath = filePath.substr(0, lastSlash);
            fileName = filePath.substr(lastSlash + 1);
        }

        // Step 2: Navigate to the parent directory
        Directory* parentDir = nullptr;
        if (parentPath.empty()) {
            // Use current directory if no parent path is provided
            parentDir = *currentDirectoryPtr;
        }
        else {
            // Resolve the directory path
            parentDir = navigateToDir(parentPath);
        }

        // Check if the parent directory exists
        if (parentDir == nullptr) {
            cout << "Error: Directory path '" << parentPath << "' does not exist.\n";
            continue; // Proceed to the next file path
        }

        // Step 3: Search for the file in the parent directory
        bool fileFound = false;
        string lowerFileName = toLower(fileName); // Convert to lowercase for case-insensitive search

        for (const auto& entry : parentDir->DirOrFiles) {
            if (toLower(entry.getName()) == lowerFileName) {
                if (!entry.getIsFile()) {
                    // If the entry is a directory, not a file
                    cout << "Error: '" << fileName << "' is a directory, not a file.\n";
                    fileFound = true; // Mark as found to avoid general error message
                    break;
                }

                // Step 4: File found, display its content
                cout << "Content of '" << fileName << "':\n";
                cout << entry.getContent() << "\n"; // Assuming `getContent()` retrieves file content
                fileFound = true; // Mark as processed
                break;
            }
        }

        // Step 5: Handle case where the file is not found
        if (!fileFound) {
            cout << "Error: File '" << fileName << "' does not exist.\n";
        }
    }
}
void CommandHandler::processDel(const vector<string>& targets) {
    // Validate input: Ensure at least one target is provided
    if (targets.empty()) {
        cout << "Error: No targets specified for deletion.\n";
        cout << "Usage: del [file|directory]+\n";
        return;
    }

    // Iterate through each target for deletion
    for (const auto& target : targets) {
        Directory* parentDir = nullptr;
        Directory_Entry* dirEntry = nullptr;
        string entryName;
        string dirPath;

        // Check if the target is a full path or relative
        bool isFullPath = (target.find(":\\") != string::npos || target[0] == '\\');

        if (isFullPath) {
            // Parse full path into directory path and entry name
            size_t lastSlash = target.find_last_of("\\");
            if (lastSlash == string::npos || lastSlash == target.length() - 1) {
                cout << "Error: Invalid path '" << target << "'.\n";
                continue;
            }

            dirPath = target.substr(0, lastSlash);
            entryName = target.substr(lastSlash + 1);

            // Navigate to the parent directory
            parentDir = navigateToDir(dirPath);
            if (!parentDir) {
                cout << "Error: Directory path '" << dirPath << "' does not exist.\n";
                continue;
            }
        }
        else {
            // Assume the target is in the current directory
            parentDir = *currentDirectoryPtr;
            entryName = target;
        }

        // Search for the entry in the parent directory
        int entryIndex = parentDir->searchDirectory(entryName);
        if (entryIndex == -1) {
            cout << "Error: '" << entryName << "' does not exist in '" << parentDir->getFullPath() << "'.\n";
            continue;
        }

        dirEntry = &parentDir->DirOrFiles[entryIndex];

        if (dirEntry->dir_attr == 0x10) { // Directory
            cout << "Are you sure you want to delete all files in the directory '"
                << dirEntry->getName() << "'? (y/n): ";
            char confirmation;
            cin >> confirmation;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (tolower(confirmation) == 'y') {
                // Navigate to the directory
                string fullPath = parentDir->getFullPath() + "\\" + dirEntry->getName();
                Directory* targetDir = navigateToDir(fullPath);

                if (!targetDir) {
                    cout << "Error: Could not access the directory '" << dirEntry->getName() << "'.\n";
                    continue;
                }

                // Delete all files in the directory
                for (auto it = targetDir->DirOrFiles.begin(); it != targetDir->DirOrFiles.end();) {
                    if (it->dir_attr != 0x10) { // Only process files
                        string fileName = it->getName();
                        cout << "Are you sure you want to delete the file '" << fileName << "'? (y/n): ";
                        cin >> confirmation;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');

                        if (tolower(confirmation) == 'y') {
                            File_Entry file(*it, targetDir);
                            file.deleteFile();
                            cout << "File '" << fileName << "' deleted successfully.\n";
                            it = targetDir->DirOrFiles.erase(it); // Remove from directory
                        }
                        else {
                            ++it;
                        }
                    }
                    else {
                        ++it; // Skip subdirectories
                    }
                }

                // Persist changes and notify user
                targetDir->writeDirectory();
                cout << "All files in the directory '" << dirEntry->getName() << "' have been processed.\n";
            }
            else {
                cout << "Skipped deletion of files in directory '" << dirEntry->getName() << "'.\n";
            }
        }
        else { // File
            string fileName = dirEntry->getName();
            cout << "Are you sure you want to delete the file '" << fileName << "'? (y/n): ";
            char confirmation;
            cin >> confirmation;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (tolower(confirmation) == 'y') {
                // Delete the file
                File_Entry file(*dirEntry, parentDir);
                file.deleteFile();
                cout << "File '" << fileName << "' deleted successfully.\n";

                // Remove the file entry and persist changes
                parentDir->DirOrFiles.erase(parentDir->DirOrFiles.begin() + entryIndex);
                parentDir->writeDirectory();
            }
            else {
                cout << "Skipped deletion of '" << fileName << "'.\n";
            }
        }
    }
}
void CommandHandler::processRename(const vector<string>& args) {
    // Step 1: Validate input arguments
    if (args.size() != 2) {
        cout << "Error: Invalid syntax for the rename command.\n";
        cout << "Usage: rename [fileName or fullPath] [new fileName]\n";
        return;
    }

    string filePath = args[0];      // The current file name or full path
    string newFileName = args[1];   // The new name for the file

    // Step 2: Ensure newFileName is a valid file name (not a path)
    if (newFileName.find("\\") != string::npos || newFileName.find(":") != string::npos) {
        cout << "Error: The new file name must be a valid file name without a path.\n";
        return;
    }

    Directory* targetDir = nullptr; // Directory containing the file to be renamed
    string fileName;                // The name of the file to rename

    // Step 3: Determine if filePath is a full path or a relative name
    if (filePath.find(":\\") != string::npos || filePath[0] == '\\') {
        // Full path case
        string path = filePath.substr(0, filePath.find_last_of("\\"));
        targetDir = navigateToDir(path);

        // Validate if the directory exists
        if (!targetDir) {
            cout << "Error: The file path '" << filePath << "' does not exist.\n";
            return;
        }

        fileName = filePath.substr(filePath.find_last_of("\\") + 1); // Extract the file name
    }
    else {
        // Relative path case (use the current directory)
        targetDir = *currentDirectoryPtr;
        fileName = filePath;
    }

    // Step 4: Search for the file in the target directory
    int fileIndex = targetDir->searchDirectory(fileName);
    if (fileIndex == -1) {
        cout << "Error: The file '" << fileName << "' does not exist in the directory.\n";
        return;
    }

    Directory_Entry& fileEntry = targetDir->DirOrFiles[fileIndex];

    // Step 5: Validate that the entry is a file, not a directory
    if (fileEntry.dir_attr == 0x10) { // 0x10 indicates a directory
        cout << "Error: '" << fileName << "' is a directory. Use 'rd' to rename directories.\n";
        return;
    }

    // Step 6: Check for duplicate file names in the directory
    for (const auto& entry : targetDir->DirOrFiles) {
        if (entry.getName() == newFileName) {
            cout << "Error: A file named '" << newFileName << "' already exists in the directory.\n";
            return;
        }
    }

    // Step 7: Rename the file
    fileEntry.assignDir_Name(newFileName); // Update the name in the directory entry
    targetDir->writeDirectory();          // Persist changes to disk

    // Step 8: Confirm success
    cout << "File '" << fileName << "' has been renamed to '" << newFileName << "' successfully.\n";
}
void CommandHandler::processCopy(const vector<string>& args)
{
    // **Case (1): Type copy alone**
    if (args.empty())
    {
        cout << "Error: Invalid syntax for copy command.\n";
        cout << "Usage: copy [source] [destination]\n";
        return;
    }

    string sourcePath = args[0];
    string destinationPath = args.size() > 1 ? args[1] : "";

    // **Parse the Source Path**
    Directory* sourceDir = nullptr;
    string sourceName;
    size_t lastSlash = sourcePath.find_last_of("/\\");

    if (lastSlash == string::npos)
    {
        // Source is in the current directory
        sourceDir = *currentDirectoryPtr;
        sourceName = sourcePath;
    }
    else
    {
        // Source path includes directories
        string sourceParentPath = sourcePath.substr(0, lastSlash);
        sourceName = sourcePath.substr(lastSlash + 1);
        sourceDir = navigateToDir(sourceParentPath);
    }

    // **Check if Source Directory Exists**
    if (!sourceDir)
    {
        // **Case (5): Full path does not exist**
        cout << "Error: Source path '" << sourcePath << "' does not exist.\n";
        return;
    }

    // **Search for the Source Entry**
    int sourceIndex = sourceDir->searchDirectory(sourceName);
    if (sourceIndex == -1)
    {
        // **Case (2): Source file does not exist**
        cout << "Error: Source '" << sourceName << "' does not exist.\n";
        return;
    }

    Directory_Entry& sourceEntry = sourceDir->DirOrFiles[sourceIndex];

    // **Handle File Copying**
    if (sourceEntry.dir_attr == 0x00) // 0x00 indicates a file
    {
        // **Determine Destination Directory and File Name**
        Directory* destinationDir = nullptr;
        string destFileName;

        if (destinationPath.empty())
        {
            // **Default Destination: Current Directory with Same Name**
            destinationDir = *currentDirectoryPtr;
            destFileName = sourceName;
            if (sourceDir == destinationDir && sourceName == destFileName)
            {
                cout << "The file cannot be copied onto itself.\n";
                cout << "0 file(s) copied.\n";
                return;
            }
        }

        else
        {
            // **Check if Destination Path is Absolute**
            bool isAbsolute = false;
            if (destinationPath.size() >= 3 && destinationPath[1] == ':' &&
                (destinationPath[2] == '\\' || destinationPath[2] == '/'))
            {
                isAbsolute = true;
            }

            if (isAbsolute)
            {
                // **Destination Path is Absolute: Extract File Name**
                size_t destLastSlash = destinationPath.find_last_of("/\\");
                if (destLastSlash == string::npos)
                {
                    // **Invalid Absolute Path (No File Name)**
                    cout << "Error: Invalid destination path.\n";
                    cout << "0 file(s) copied.\n";
                    return;
                }
                destFileName = destinationPath.substr(destLastSlash + 1);
                destinationDir = *currentDirectoryPtr; // Copy to Current Directory
            }
            else
            {
                // **Destination Path is Relative**
                size_t destLastSlash = destinationPath.find_last_of("/\\");
                if (destLastSlash == string::npos)
                {
                    // **Destination is in Current Directory**
                    destinationDir = *currentDirectoryPtr;
                    destFileName = destinationPath;
                }
                else
                {
                    // **Destination Includes Directory Path**
                    string destParentPath = destinationPath.substr(0, destLastSlash);
                    destFileName = destinationPath.substr(destLastSlash + 1);
                    destinationDir = navigateToDir(destParentPath);
                }
            }
        }

        // **Check if Destination Directory Exists**
        if (!destinationDir)
        {
            // **Case (5): Destination Directory Not Found**
            cout << "Error: Destination directory does not exist.\n";
            cout << "0 file(s) copied.\n";
            return;
        }

        // **Check if Destination is a Directory**
        int destIndex = destinationDir->searchDirectory(destFileName);
        bool destIsDirectory = false;

        if (destIndex != -1)
        {
            // **Destination Exists**
            if (destinationDir->DirOrFiles[destIndex].dir_attr == 0x10)
            {
                // **Destination is a Directory**
                destIsDirectory = true;
                destinationDir = destinationDir->DirOrFiles[destIndex].subDirectory;
                destFileName = sourceName; // Copy with Same Name into Destination Directory
            }
        }

        if (destIsDirectory)
        {
            // **Case (13): Copy File into Destination Directory**
            // **Check if File Already Exists in Destination Directory**
            int existingIndex = destinationDir->searchDirectory(sourceName);
            if (existingIndex != -1)
            {
                // **Case (14): Destination File Exists - Prompt for Overwrite**
                cout << "Error: File with the name '" << sourceName << "' already exists in the destination directory.\n";
                cout << "Do you want to overwrite it? (y/n): ";
                char choice;
                cin >> choice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (tolower(choice) != 'y')
                {
                    cout << "Copy operation canceled for '" << sourceName << "'.\n";
                    cout << "0 file(s) copied.\n";
                    return;
                }

                // **Overwrite Existing File**
                Directory_Entry& existingEntry = destinationDir->DirOrFiles[existingIndex];
                existingEntry = sourceEntry; // Assuming Shallow Copy is Sufficient
                cout << "File '" << sourceName << "' overwritten successfully in the destination directory.\n";
                cout << "1 file(s) copied.\n";
                return;
            }

            // **Destination File Does Not Exist - Proceed to Copy**
            Directory_Entry newFileEntry = sourceEntry;
            newFileEntry.assignDir_Name(sourceName); // Assign Same Name

            if (!destinationDir->canAddEntry(newFileEntry))
            {
                // **Case (6): Not Enough Space**
                cout << "Error: Not enough space to copy file '" << sourceName << "'.\n";
                cout << "0 file(s) copied.\n";
                return;
            }

            destinationDir->addEntry(newFileEntry);
            cout << "File '" << sourceName << "' copied successfully to the destination directory.\n";
            cout << "1 file(s) copied.\n";
            return;
        }
        else
        {
            // **Destination is a File or Intended to be a File**
            // **Check for Self-Copy**
            if (sourcePath == destinationPath)
            {
                // **Case (3) & (4): Self-Copy Detected**
                cout << "Error: The file cannot be copied onto itself.\n";
                cout << "0 file(s) copied.\n";
                return;
            }

            // **Check if Destination File Exists**
            if (destinationDir->searchDirectory(destFileName) != -1)
            {
                // **Case (14): Destination File Exists - Prompt for Overwrite**
                cout << "Error: File with the name '" << destFileName << "' already exists in the destination directory.\n";
                cout << "Do you want to overwrite it? (y/n): ";
                char choice;
                cin >> choice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (tolower(choice) != 'y')
                {
                    cout << "Copy operation canceled for '" << destFileName << "'.\n";
                    cout << "0 file(s) copied.\n";
                    return;
                }

                // **Overwrite Existing File**
                Directory_Entry& existingEntry = destinationDir->DirOrFiles[destIndex];
                existingEntry = sourceEntry; // Assuming Shallow Copy is Sufficient
                cout << "File '" << destFileName << "' overwritten successfully.\n";
                cout << "1 file(s) copied.\n";
                return;
            }

            // **Destination File Does Not Exist - Proceed to Copy**
            Directory_Entry newFileEntry = sourceEntry;
            newFileEntry.assignDir_Name(destFileName); // Assign New File Name

            if (!destinationDir->canAddEntry(newFileEntry))
            {
                // **Case (6): Not Enough Space**
                cout << "Error: Not enough space to copy file '" << sourceName << "'.\n";
                cout << "0 file(s) copied.\n";
                return;
            }

            destinationDir->addEntry(newFileEntry);
            cout << "File '" << sourceName << "' copied successfully as '" << destFileName << "'.\n";
            cout << "1 file(s) copied.\n";
            return;
        }
    }

    // **Handle Directory Copying**
    if (sourceEntry.dir_attr == 0x10) // 0x10 indicates a directory
    {
        // **Case (7) (BONUS): Copy All Files from Source Directory to Current Directory**
        // **Case (10): Copy All Files from Source Directory to Existing Destination Directory**
        // **Case (14): Handle Overwrite Scenarios**

        Directory* destinationDir = nullptr;
        string destDirName;

        if (destinationPath.empty())
        {
            // **Case (7) (BONUS): No Destination Path Provided - Copy to Current Directory**
            destinationDir = *currentDirectoryPtr;
        }
        else
        {
            // **Destination Path Provided - Determine if Directory Exists**
            size_t destLastSlash = destinationPath.find_last_of("/\\");
            if (destLastSlash == string::npos)
            {
                // **Destination is in Current Directory**
                destinationDir = *currentDirectoryPtr;
                destDirName = destinationPath;
            }
            else
            {
                // **Destination Includes Directory Path**
                string destParentPath = destinationPath.substr(0, destLastSlash);
                destDirName = destinationPath.substr(destLastSlash + 1);
                destinationDir = navigateToDir(destParentPath);
            }
        }

        if (!destinationDir)
        {
            // **Case (9): Destination Directory Not Found**
            cout << "Error: Destination directory '" << destinationPath << "' does not exist.\n";
            return;
        }

        // **Check if Destination Path is an Existing Directory**
        bool destIsDirectory = false;
        if (!destDirName.empty())
        {
            int destIndex = destinationDir->searchDirectory(destDirName);
            if (destIndex != -1 && destinationDir->DirOrFiles[destIndex].dir_attr == 0x10)
            {
                // **Destination is an Existing Directory**
                destIsDirectory = true;
                destinationDir = destinationDir->DirOrFiles[destIndex].subDirectory;
            }
            else if (destIndex != -1 && destinationDir->DirOrFiles[destIndex].dir_attr != 0x10)
            {
                // **Destination Exists but is Not a Directory**
                cout << "Error: Destination path '" << destinationPath << "' is not a directory.\n";
                return;
            }
            else
            {
                // **Destination Directory Does Not Exist**
                cout << "Error: Destination directory '" << destinationPath << "' does not exist.\n";
                return;
            }
        }

        // **Iterate Through Source Directory Entries and Copy Files**
        int filesCopied = 0;
        for (const auto& entry : sourceEntry.subDirectory->DirOrFiles)
        {
            if (entry.dir_attr == 0x00) // Only Copy Files
            {
                string srcFileName = entry.getName();
                int destIndex = destinationDir->searchDirectory(srcFileName);

                if (destIndex != -1)
                {
                    // **Case (14): Destination File Exists - Prompt for Overwrite**
                    cout << "Error: File with the name '" << srcFileName << "' already exists in the destination directory.\n";
                    cout << "Do you want to overwrite it? (y/n): ";
                    char choice;
                    cin >> choice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (tolower(choice) != 'y')
                    {
                        cout << "Copy operation skipped for '" << srcFileName << "'.\n";
                        continue;
                    }

                    // **Overwrite Existing File**
                    Directory_Entry& existingEntry = destinationDir->DirOrFiles[destIndex];
                    existingEntry = entry; // Assuming Shallow Copy is Sufficient
                    cout << "File '" << srcFileName << "' overwritten successfully in destination directory.\n";
                    filesCopied++;
                    continue;
                }

                // **Destination File Does Not Exist - Proceed to Copy**
                Directory_Entry newFileEntry = entry;
                newFileEntry.assignDir_Name(srcFileName); // Assign Same Name

                if (!destinationDir->canAddEntry(newFileEntry))
                {
                    // **Case (6): Not Enough Space**
                    cout << "Error: Not enough space to copy file '" << srcFileName << "'.\n";
                    continue;
                }

                destinationDir->addEntry(newFileEntry);
                cout << "File '" << srcFileName << "' copied successfully to destination directory.\n";
                filesCopied++;
            }
            // **Note**: Skipping subdirectories as per initial requirements
        }

        // **Output Summary of Copied Files**
        cout << filesCopied << " file(s) copied from directory '" << sourceName << "'.\n";
        return;
    }

    // **Unsupported Entry Type**
    cout << "Error: Unsupported entry type for '" << sourceName << "'.\n";
}
void CommandHandler::processImport(const std::vector<std::string>& args) {
    // Check for correct number of arguments
    if (args.empty() || args.size() > 2) {
        // Syntax error
        std::cout << "Error: Invalid syntax for import command.\n";
        std::cout << "Usage:\n  import [source]\n  import [source] [destination]\n";
        return;
    }

    std::string source = args[0];
    std::string destination = (args.size() == 2) ? args[1] : "";

    // Resolve the source path using filesystem library
    fs::path sourcePath(source);
    if (!sourcePath.is_absolute()) {
        // First, check in the current directory of the virtual disk
        fs::path currentDirPath = fs::path((*currentDirectoryPtr)->getFullPath());
        fs::path potentialPath = currentDirPath / sourcePath;
        if (fs::exists(potentialPath)) {
            sourcePath = potentialPath;
        }
        else {
            // If not found, check in the debug folder
            // Adjust the debug folder path as per your project's structure
            fs::path debugFolder = "C:\\Users\\omara\\Desktop\\OSOS-master\\OSOS-master\\shell\\x64\\Debug\\";
            potentialPath = debugFolder / sourcePath;
            if (fs::exists(potentialPath)) {
                sourcePath = potentialPath;
            }
            else {
                // Source file or directory not found in both locations
                std::cout << "Error: Source file or directory '" << source << "' does not exist.\n";
                return;
            }
        }
    }

    // Proceed if the source exists
    if (!fs::exists(sourcePath)) {
        std::cout << "Error: Source file or directory '" << source << "' does not exist.\n";
        return;
    }

    // Initialize import counter
    int importedFileCount = 0;

    // Handle if source is a directory: import all .txt files
    if (fs::is_directory(sourcePath)) {
        Directory* targetDir = *currentDirectoryPtr; // Start with current directory

        // Determine the target directory based on destination
        if (!destination.empty()) {
            fs::path destPath(destination);
            // Normalize destination path by removing trailing slashes
            destPath = destPath.lexically_normal();

            if (destPath.is_absolute()) {
                if (fs::exists(destPath)) {
                    if (fs::is_directory(destPath)) {
                        // Destination is an existing directory: navigate to it
                        Directory* destDir = navigateToDir(destPath.string());
                        if (destDir == nullptr) {
                            std::cout << "Error: Destination directory '" << destination << "' does not exist or is not correctly linked.\n";
                            return;
                        }
                        targetDir = destDir;
                    }
                    else {
                        std::cout << "Error: Destination path '" << destination << "' is not a directory.\n";
                        return;
                    }
                }
                else {
                    // Destination directory does not exist: need to create it
                    // Extract parent path and directory name
                    fs::path parentPath = destPath.parent_path();
                    std::string dirName = destPath.filename().string();
                    if (dirName.empty()) {
                        std::cout << "Error: Destination directory name is empty.\n";
                        return;
                    }

                    Directory* parentDir = nullptr;
                    if (parentPath.empty()) {
                        // Destination is directly under the current directory
                        parentDir = *currentDirectoryPtr;
                    }
                    else {
                        // Navigate to the parent directory
                        parentDir = navigateToDir(parentPath.string());
                        if (parentDir == nullptr) {
                            std::cout << "Error: Parent directory '" << parentPath.string() << "' does not exist.\n";
                            return;
                        }
                    }

                    // Check if directory with the same name already exists in parent
                    bool dirExists = false;
                    Directory* existingDir = nullptr;
                    for (const auto& entry : parentDir->DirOrFiles) {
                        if (!entry.getIsFile() && toLower(entry.getName()) == toLower(dirName)) {
                            dirExists = true;
                            existingDir = entry.subDirectory;
                            break;
                        }
                    }

                    if (dirExists && existingDir != nullptr) {
                        std::cout << "Directory '" << destination << "' already exists.\n";
                        targetDir = existingDir;
                    }
                    else {
                        // Create the directory
                        char dir_attr = 0x10;               // Directory attribute
                        int dir_firstCluster = 0;           // First cluster (use 0 if not applicable)
                        Directory* newDir = new Directory(dirName, dir_attr, dir_firstCluster, parentDir);

                        // Create a new Directory_Entry for the new directory
                        Directory_Entry newDirEntry(dirName, 0x10, 0);
                        newDirEntry.setIsFile(false);          // Mark as directory
                        newDirEntry.subDirectory = newDir;     // Link the Directory object

                        // Add the new directory entry to the parent directory
                        parentDir->addEntry(newDirEntry);
                        parentDir->writeDirectory();           // Persist changes

                        targetDir = newDir;                    // Set the target directory to the newly created directory
                        std::cout << "Directory '" << destination << "' created successfully.\n";
                    }
                }
            }
            else {
                // Destination is a relative directory name (e.g., "omar")
                std::string dirName = destPath.string();
                if (dirName.empty()) {
                    std::cout << "Error: Destination directory name is empty.\n";
                    return;
                }

                // Check if the directory already exists in the current directory
                bool dirExists = false;
                Directory* existingDir = nullptr;
                for (const auto& entry : targetDir->DirOrFiles) {
                    if (!entry.getIsFile() && toLower(entry.getName()) == toLower(dirName)) {
                        dirExists = true;
                        existingDir = entry.subDirectory;
                        break;
                    }
                }

                if (dirExists && existingDir != nullptr) {
                    std::cout << "Directory '" << destination << "' already exists.\n";
                    targetDir = existingDir;
                }
                else {
                    // Create the directory
                    char dir_attr = 0x10;               // Directory attribute
                    int dir_firstCluster = 0;           // First cluster (use 0 if not applicable)
                    Directory* newDir = new Directory(dirName, dir_attr, dir_firstCluster, targetDir);

                    // Create a new Directory_Entry for the new directory
                    Directory_Entry newDirEntry(dirName, 0x10, 0);
                    newDirEntry.setIsFile(false);          // Mark as directory
                    newDirEntry.subDirectory = newDir;     // Link the Directory object

                    // Add the new directory entry to the target directory
                    targetDir->addEntry(newDirEntry);
                    targetDir->writeDirectory();           // Persist changes

                    targetDir = newDir;                    // Set the target directory to the newly created directory
                    std::cout << "Directory '" << destination << "' created successfully.\n";
                }
            }
        }

        // Iterate over the source directory and import .txt files
        for (const auto& entry : fs::directory_iterator(sourcePath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::string fileName = entry.path().filename().string();

                // Validate file name
                const std::string invalidChars = "\\/:*?\"<>|";
                bool isValid = true;
                for (char c : fileName) {
                    if (invalidChars.find(c) != std::string::npos) {
                        isValid = false;
                        break;
                    }
                }
                if (fileName.empty() || !isValid) {
                    // Assign a default name if necessary
                    std::string extension = entry.path().extension().string();
                    if (extension.empty()) {
                        std::cout << "Error: File '" << entry.path().string() << "' has no valid name or extension. Skipping import.\n";
                        continue;
                    }
                    fileName = "<No Name>" + extension;
                    std::cout << "Warning: File name was empty or invalid. Assigned default name '" << fileName << "'.\n";
                }

                // Check if the file already exists in the target directory
                bool fileExists = false;
                int existingFileIndex = -1;
                for (size_t i = 0; i < targetDir->DirOrFiles.size(); ++i) {
                    // Case-insensitive comparison
                    if (toLower(targetDir->DirOrFiles[i].getName()) == toLower(fileName)) {
                        fileExists = true;
                        existingFileIndex = static_cast<int>(i);
                        break;
                    }
                }

                if (fileExists) {
                    // Prompt to overwrite
                    std::cout << "File '" << fileName << "' already exists. Do you want to overwrite it? (yes/no): ";
                    std::string userChoice;
                    std::getline(std::cin, userChoice);
                    std::transform(userChoice.begin(), userChoice.end(), userChoice.begin(), ::tolower);
                    if (userChoice != "yes") {
                        std::cout << "Skipped importing '" << fileName << "'.\n";
                        continue;
                    }
                }

                // Read the content of the source file
                std::ifstream inputFile(entry.path(), std::ios::binary);
                if (!inputFile.is_open()) {
                    std::cout << "Error: Unable to open source file '" << entry.path().string() << "'. Skipping import.\n";
                    continue;
                }
                std::string fileContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
                inputFile.close();

                // Debug Statement

                if (fileExists && existingFileIndex != -1) {
                    // Overwrite the existing file's content
                    Directory_Entry& existingEntry = targetDir->DirOrFiles[existingFileIndex];
                    existingEntry.setContent(fileContent);
                    existingEntry.setIsFile(true);      // Ensure it's marked as a file
                    existingEntry.dir_attr = 0x00;      // Ensure dir_attr is set correctly
                    targetDir->writeDirectory();
                    std::cout << "File '" << fileName << "' overwritten and imported successfully.\n";
                }
                else {
                    // Create a new file entry
                    Directory_Entry newFile(fileName, 0x00, static_cast<int>(fileContent.size())); // attr=0x00 for file
                    newFile.setIsFile(true);                      // Mark as file
                    newFile.dir_attr = 0x00;                      // Ensure dir_attr is set correctly
                    newFile.setContent(fileContent);              // Set file content and update size
                    targetDir->addEntry(newFile);                 // Add to directory
                    targetDir->writeDirectory();                   // Write changes
                    std::cout << "File '" << fileName << "' imported successfully.\n";
                    importedFileCount++;
                }
            }
        }

        // Display the total number of files imported
        std::cout << "Total files imported: " << importedFileCount << "\n";
    }
}
void CommandHandler::processExport(const std::vector<std::string>& args) {
    if (args.size() < 1 || args.size() > 2) {
        std::cout << "Error: Invalid syntax for export command.\n";
        std::cout << "Usage: export [source_file_or_directory] [destination_file_or_directory]\n";
        return;
    }

    std::string sourcePath = args[0];
    std::string destinationPath = (args.size() == 2) ? args[1] : fs::current_path().string();

    Directory* currentDir = *currentDirectoryPtr;
    Directory_Entry* sourceEntry = nullptr;

    bool isSourceAbsolutePath = (sourcePath.length() >= 3 && isalpha(sourcePath[0]) && sourcePath[1] == ':' && (sourcePath[2] == '\\' || sourcePath[2] == '/'));

    // Resolve source path
    if (isSourceAbsolutePath) {
        std::string dirPath = sourcePath.substr(0, sourcePath.find_last_of("\\/"));
        std::string entryName = sourcePath.substr(sourcePath.find_last_of("\\/") + 1);

        Directory* resolvedDir = navigateToDir(dirPath);
        if (!resolvedDir) {
            std::cout << "Error: Directory '" << dirPath << "' does not exist.\n";
            return;
        }

        int entryIndex = resolvedDir->searchDirectory(entryName);
        if (entryIndex == -1) {
            std::cout << "Error: File or directory '" << entryName << "' does not exist in '" << dirPath << "'.\n";
            return;
        }

        sourceEntry = &resolvedDir->DirOrFiles[entryIndex];
    }
    else {
        int entryIndex = currentDir->searchDirectory(sourcePath);
        if (entryIndex == -1) {
            std::cout << "Error: File or directory '" << sourcePath << "' does not exist in the current directory.\n";
            return;
        }

        sourceEntry = &currentDir->DirOrFiles[entryIndex];
    }

    int exportedFiles = 0; // Counter for exported files

    // Check if source is a directory
    if (sourceEntry->dir_attr == 0x10) { // Directory
        Directory sourceDir(sourceEntry->getName(), sourceEntry->dir_attr, sourceEntry->dir_firstCluster, currentDir);
        sourceDir.readDirectory();

        for (const auto& entry : sourceDir.DirOrFiles) {
            if (entry.dir_attr != 0x10) { // Export files only
                File_Entry file(entry, &sourceDir);
                file.readFileContent();

                std::string destinationFilePath = (fs::path(destinationPath) / entry.getName()).string();

                // Check for overwrite
                if (fs::exists(destinationFilePath)) {
                    std::cout << "File '" << destinationFilePath << "' already exists. Overwrite? (yes/no): ";
                    std::string choice;
                    std::getline(std::cin, choice);
                    std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
                    if (choice != "yes") {
                        std::cout << "Skipping '" << entry.getName() << "'.\n";
                        continue;
                    }
                }

                std::ofstream outFile(destinationFilePath, std::ios::binary);
                if (!outFile.is_open()) {
                    std::cout << "Error: Unable to open destination file '" << destinationFilePath << "'.\n";
                    continue;
                }

                outFile.write(file.content.c_str(), file.content.size());
                outFile.close();

                exportedFiles++;
            }
        }

        std::cout << "Total files exported from '" << sourceDir.getFullPath() << "': " << exportedFiles << "\n";
        return;
    }

    // If source is a single file
    if (sourceEntry->dir_attr != 0x10) {
        File_Entry file(*sourceEntry, currentDir);
        file.readFileContent();

        std::string destinationFilePath = destinationPath;
        if (fs::is_directory(destinationPath)) {
            destinationFilePath = (fs::path(destinationPath) / sourceEntry->getName()).string();
        }

        // Check for overwrite
        if (fs::exists(destinationFilePath)) {
            std::cout << "File '" << destinationFilePath << "' already exists. Overwrite? (yes/no): ";
            std::string choice;
            std::getline(std::cin, choice);
            std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
            if (choice != "yes") {
                std::cout << "Export canceled for '" << sourceEntry->getName() << "'.\n";
                return;
            }
        }

        std::ofstream outFile(destinationFilePath, std::ios::binary);
        if (!outFile.is_open()) {
            std::cout << "Error: Unable to open destination file '" << destinationFilePath << "'.\n";
            return;
        }

        outFile.write(file.content.c_str(), file.content.size());
        outFile.close();

        exportedFiles++;
        std::cout << "File '" << sourceEntry->getName() << "' exported successfully to '" << destinationFilePath << "'.\n";
        std::cout << "Total files exported: " << exportedFiles << "\n";
        return;
    }
}
