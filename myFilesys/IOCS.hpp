//
//  IOCS.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef IOCS_hpp
#define IOCS_hpp

#include <map>
#include "VCB.hpp"
#include "CatalogFile.hpp"

/**
 * I/O Control System
 */
class IOCS {
    
public:
    IOCS();
    static IOCS *sharedInstance;
    VCB *volumeControlBlock;
    FILE* volumeFile;
    size_t volumeSize;
    CatalogFile* catalogFileInode;
    FSManager* freeSpaceManager;
    Inode* rootDirInode;
    std::map<std::string, Inode*> fileTable;
    
    void create(std::string volumeName, size_t size);
    void mount(std::string volumeName);
    void unmount();
    
    void createFile(std::string path);
    FILE* openFile(std::string path);
    void closeFile(std::string path);
    bool writeFile(std::string sourcePath, std::string destPath);
    void readFile(std::string path);
    void deleteFile(std::string path, Inode* dirInode);
    
    void createDirectory(std::string path);
    void list(std::string path);
    
private:
    void createInode(std::string path, bool isDirectory);
};

#endif /* IOCS_hpp */
