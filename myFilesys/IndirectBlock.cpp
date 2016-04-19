//
//  IndirectBlock.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include "IndirectBlock.hpp"
#include "IOCS.hpp"

IndirectBlock::IndirectBlock(){}

IndirectBlock::IndirectBlock(blocknum_t blocknum):
blocknum(blocknum) {
    // Go to block's location on disk
    fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(blocknum.block*BLOCK_SIZE), SEEK_SET);
    
    // Read data into instance variables
    fread(blocks, sizeof(blocknum_t), 128, IOCS::sharedInstance->volumeFile);
}

void IndirectBlock::write() {
    // Go to block's location on disk
    fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(blocknum.block*BLOCK_SIZE), SEEK_SET);
    
    // Write instance variables to disk
    fwrite(blocks, sizeof(blocknum_t), 128, IOCS::sharedInstance->volumeFile);
}

blocknum_t* IndirectBlock::nextFreeBlockPointer() {
    for (int i = 0; i < 128; i++) {
        if (!blocks[i].valid) {
            return &blocks[i];
        }
    }
    return NULL;
}