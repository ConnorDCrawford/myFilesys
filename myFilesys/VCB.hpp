//
//  VCB.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef VCB_hpp
#define VCB_hpp

#include <iostream>
#include "Constants.h"

class IOCS;

/**
 * Volume Control Block
 */

class VCB {
    
public:
    VCB();
    VCB(int magic, int blockSize, int volumeSize, std::string name);
    
    // a "magic" number used to check if disk is in myFilesys format
    int32_t magic;
    // the block size
    int32_t blocksize;
    // the size (in blocks) of the disk
    int32_t volumeSize;
    // the location of the catalog file inode
    blocknum_t cat;
    // the location of the allocation file inode
    blocknum_t alloc;
    // the inode number of the root directory
    blocknum_t root;
    // the name of the disk
    char name[256];
    
    // class functions
    // read from drive
    void read();
    // write to drive
    void write();
};



#endif /* VCB_hpp */
