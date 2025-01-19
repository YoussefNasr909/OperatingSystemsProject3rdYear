#include "Virtual_Disk.h"
#include "Mini_FAT.h"
#include "Directory.h"
#include "Directory_Entry.h"
#include "File_Entry.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "CommandHandler.h"
#include "Converter.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main()
{
    // Path to the virtual disk file
    string diskPath = "virtual_disk.bin";

    // Step 1: Initialize or open the virtual disk and FAT system
    Mini_FAT::initialize_Or_Open_FileSystem(diskPath);

    // Step 2: Create the root directory "C:\" and initialize its contents
    Directory* rootDir = new Directory("C:", 0x10, 0, nullptr); // Create root directory with default values
    rootDir->name = "C:"; // Assign the name "C:" to the root directory
    rootDir->readDirectory(); // Load directory entries from the virtual disk

    // Step 3: Set the current working directory to the root
    Directory* currentDir = rootDir;

    // Step 4: Initialize the command handler
    CommandHandler cmdHandler(&currentDir);

    // Step 5: Display the welcome message
    cout << "  =========================================================================================================" << endl;
    cout << "                                          Welcome to OUR Shell                                 " << endl;
    cout << "  ---------------------------------------------------------------------------------------------------------" << endl;
    cout << "                                              Developed by:                                                   " << endl;
    cout << "                                             - Abdelrahman Naser                                            " << endl;
    cout << "                                             - Youssef Nasr                                                 " << endl;
    cout << "                                             - Abdelrahman Yasser                                           " << endl;
    cout << "  ---------------------------------------------------------------------------------------------------------" << endl;
    cout << "                                   Type 'help' to view available commands.                                  " << endl;
    cout << "  =========================================================================================================" << endl;

    // Step 6: Start the shell loop
    bool isRunning = true; // Flag to determine if the shell is active
    while (isRunning)
    {
        // Prompt the user with the current directory path
        string input;
        cout << currentDir->getFullPath() << " >> "; // Display the full path of the current directory
        getline(cin, input); // Read user input
        cmdHandler.executeCommand(input, isRunning); // Process the command through the command handler
    }

    // Step 7: Clean up resources before exiting
    Mini_FAT::CloseTheSystem(); // Save any pending changes and close the virtual disk
    delete rootDir; // Release memory allocated for the root directory

    return 0; // Exit the program
}
