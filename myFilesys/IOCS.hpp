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
    
    void mount(std::string volumeName);
    void create(std::string volumeName, size_t size);
    void unmount();
    
    FILE* createFile(std::string path);
    FILE* openFile(std::string path);
    void closeFile(std::string path);
    bool writeFile(std::string sourcePath, std::string destPath);
    void deleteFile(std::string path);
    
    void createDirectory(std::string path);
    
private:
    FILE* createInode(std::string path, bool isDirectory);
};

#endif /* IOCS_hpp */
