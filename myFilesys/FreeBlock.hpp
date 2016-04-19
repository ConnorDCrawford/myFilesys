//
//  FreeBlock.hpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef FreeBlock_hpp
#define FreeBlock_hpp

#include <stdio.h>
#include "Constants.h"
#include "IOCS.hpp"

class FreeBlock {
    
public:
    FreeBlock();
    char empty[512];
    
    // write to disk
    void write();
};

#endif /* FreeBlock_hpp */
