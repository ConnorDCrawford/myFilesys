//
//  DataBlock.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef Block_hpp
#define Block_hpp

#include <stdio.h>
#include "Constants.h"

class DataBlock {
    
public:
    blocknum_t blocknum;
    
    // write to disk
    void write();
    
private:
    char data[BLOCK_SIZE];
};

#endif /* Block_hpp */
