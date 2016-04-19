//
//  FSManager.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef FSManager_hpp
#define FSManager_hpp

#include "Constants.h"
#include "Inode.hpp"

/**
 * Free Space Manager
 */
class FSManager {
    
public:
    FSManager();
    FSManager(unsigned int inodeNumber, size_t dataSegementBlocks);
    Inode allocationFileInode;
    
    blocknum_t getFreeBlock();
//    blocknum_t getContiguousFreeSpace(size_t &size);
    void markBlock(unsigned int blocknum, bool free);
    
};

#endif /* FSManager_hpp */
