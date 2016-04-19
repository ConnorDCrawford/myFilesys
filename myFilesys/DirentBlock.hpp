//
//  DirentBlock.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef DirentBlock_hpp
#define DirentBlock_hpp

#include "Constants.h"

class DirentBlock {
    
public:
    DirentBlock();
    DirentBlock(unsigned int blocknum);
    blocknum_t block;
    char name[256];
    unsigned char type;
    // write to disk
    void write(blocknum_t block);
};

#endif /* DirentBlock_hpp */
