//
//  main.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include "IOCS.hpp"

void shell();
void printPrompt();
char* getCmd();
void parseCmd(char *cmd, std::vector<std::string> &args);
void executeCommand(std::vector<std::string> args);
std::vector<std::string> &tokenizePrompt(std::string &s, char delim, std::vector<std::string> &elems);

void create(std::vector<std::string> args);
void mount(std::vector<std::string> args);
void unmount(std::vector<std::string> args);
void createFile(std::vector<std::string> args);
void openFile(std::vector<std::string> args);
void closeFile(std::vector<std::string> args);
void writeFile(std::vector<std::string> args);
void readFile(std::vector<std::string> args);
void deleteFile(std::vector<std::string> args);
void createDirectory(std::vector<std::string> args);
void quit(std::vector<std::string> args);
void list(std::vector<std::string> args);

std::vector<std::string> builtin_str = { "create", "mount", "unmount", "touch", "o", "c", "w", "r", "rm", "mkdir", "quit", "ls"};
void (*builtin_func[]) (std::vector<std::string>) = { &create, &mount, &unmount, &createFile, &openFile, &closeFile, &writeFile,
    &readFile, &deleteFile, &createDirectory, &quit, &list};

int main(int argc, const char * argv[]) {
    shell();
    return 0;
}

std::vector<std::string> &tokenizePrompt(std::string &s, char delim, std::vector<std::string> &elems) {
    // Remove newline character
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    
    std::stringstream ss(s);
    std::string item;
    
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

/*
 * Main logic of the shell. Will process batch commands if batchfile is not NULL.
 * Parameters:      FILE* - pointer to a file containing batch commands, can be NULL
 */
void shell() {
    char *cmd = NULL;
    
    while (1) {
        std::vector<std::string> args;
        
        printPrompt();
        
        cmd = getCmd();
        if (*cmd == '\0')
            break; // stop instance of shell if command is null character, or if using a batchfile and reach EOF
        
        parseCmd(cmd, args);
        
        executeCommand(args);
        
        free(cmd);
    }
}

/*
 * Gets a command line from stdin or a batchfile
 * Parameters:      FILE* - pointer to a file containing batch commands, can be NULL
 * Returns:         char* - a C string containing the command
 */
char* getCmd() {
    char *cmd = NULL;
    size_t nBytes = 0;
    
    while (!cmd)
        getline(&cmd, &nBytes, stdin); // get line from either stdin
    
    return cmd;
}

/*
 * Separates commands into an array of arguments
 * Parameters:      char* - pointer to a file containing batch commands, can be NULL
 *                  char** - array of C strings containing arguments
 */
void parseCmd(char *cmd, std::vector<std::string> &args) {
    if (!cmd)
        return;
    std::string cmdStr(cmd);
    tokenizePrompt(cmdStr, ' ', args);
}

void printPrompt() {
    std::cout<< "myFileSys Utility: ";
}

void executeCommand(std::vector<std::string> args) {
    if (args.size() == 0) {
        std::cout << "\n";
        return;
    }
    
    for (int i = 0; i < builtin_str.size(); i++) {
        if (args[0] == builtin_str[i]) { // compare first argument against list of internal commands
            builtin_func[i](args); // execute that corresponding function
            return;
        }
    }
    
    std::cout << "Invalid command.\n";
}

void create(std::vector<std::string> args) {
    if (args.size() >= 3) {
        size_t size = std::stoi(args[2]); // In MB
        size *= 1000000;
        size = size/BLOCK_SIZE + size%BLOCK_SIZE;
        IOCS::sharedInstance->create(args[1], size);
    } else {
        std::cout << "Expected 2 arguments. Recieved " << args.size()-1 << ".\n";
    }
    
}

void mount(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->mount(args[1]);
    } else {
        std::cout << "Expected 1 arguments. Recieved " << args.size()-1 << ".\n";
    }
}

void unmount(std::vector<std::string> args) {
    IOCS::sharedInstance->unmount();
}

void createFile(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->createFile(args[1]);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}

void openFile(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->openFile(args[1]);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}

void closeFile(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->closeFile(args[1]);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}

void writeFile(std::vector<std::string> args) {
    if (args.size() >= 3) {
        IOCS::sharedInstance->writeFile(args[1], args[2]);
    } else {
        std::cout << "Expected 2 arguments. Recieved " << args.size()-1 << ".\n";
    }
}

void readFile(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->readFile(args[1]);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}

void deleteFile(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->deleteFile(args[1], NULL);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}

void createDirectory(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->createDirectory(args[1]);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}

void quit(std::vector<std::string> args) {
    if (IOCS::sharedInstance->volumeFile != NULL) {
        IOCS::sharedInstance->unmount();
    }
    exit(0);
}

void list(std::vector<std::string> args) {
    if (args.size() >= 2) {
        IOCS::sharedInstance->list(args[1]);
    } else if (args.size() > 0) {
        std::string empty = "";
        IOCS::sharedInstance->list(empty);
    } else {
        std::cout << "Expected 1 argument. Recieved " << args.size()-1 << ".\n";
    }
}