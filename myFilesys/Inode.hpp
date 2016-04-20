//
//  Inode.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef Inode_hpp
#define Inode_hpp

#include <stdio.h>
#include <ctime>
#include <string>
#include "IndirectBlock.hpp"
#include "Constants.h"


class Inode {
    
public:
    Inode();
    Inode(unsigned int number);
    // file metadata
    unsigned int number;
    unsigned int links;
    unsigned int blocks;
    char isDir;
    
    std::time_t cTime;
    std::time_t mTime;
    std::time_t aTime;
    
    /* Locations of file data/directory entry blocks.
     * Follows ext2 filesystem standard:
     * Pointers 1 to 12 point to direct blocks, pointer 13 points to an indirect block,
     * pointer 14 points to a doubly indirect block, and pointer 15 points to a trebly indirect block.
     */
    blocknum_t directBlocks[12];
    blocknum_t indirectBlock;
    blocknum_t dIndirectBlock;
    blocknum_t tIndirectBlock;
    
    // read from disk
    FILE* open(std::string name);
    // write to disk
    void write();
    blocknum_t* blockPtrWithNumber(unsigned int number);
    blocknum_t* nextFreeBlockPtr();
    void setBlockPtr(blocknum_t blocknum, unsigned int withNumber);
    bool writeData(FILE* fromFile);
    void readData();
    void deleteData(bool shouldDeleteEntry);
    
private:
    int32_t _lastBlockSize;
};

#endif /* Inode_hpp */
