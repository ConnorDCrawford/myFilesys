//
//  IndirectBlock.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef IndirectBlock_hpp
#define IndirectBlock_hpp

#include <stdio.h>
#include "Constants.h"

class IndirectBlock {
    
public:
    IndirectBlock();
    IndirectBlock(blocknum_t blocknum);
    blocknum_t blocks[128];
    // This instance's location on the disk
    blocknum_t blocknum;
    // write to disk
    void write();
    blocknum_t* nextFreeBlockPointer();
};

#endif /* IndirectBlock_hpp */
