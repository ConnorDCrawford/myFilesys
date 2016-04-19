//
//  DirentBlock.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include "DirentBlock.hpp"
#include "IOCS.hpp"

DirentBlock::DirentBlock(){}

DirentBlock::DirentBlock(unsigned int blocknum) {
    // Go to inode's block
    fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(blocknum*BLOCK_SIZE), SEEK_SET);
    
    // Set instance variables to data stored on disk
    fread(&block, sizeof(block), 1, IOCS::sharedInstance->volumeFile);
    fread(name, sizeof(name), 1, IOCS::sharedInstance->volumeFile);
    fread(&type, sizeof(type), 1, IOCS::sharedInstance->volumeFile);
}

void DirentBlock::write(blocknum_t block) {
    if (block.valid) {
        // Go to blocks location
        fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(block.block*BLOCK_SIZE), SEEK_SET);
        // Write instance variables' values to file
        fwrite(&(this->block), sizeof(block), 1, IOCS::sharedInstance->volumeFile);
        fwrite(name, sizeof(name), 1, IOCS::sharedInstance->volumeFile);
        fwrite(&type, sizeof(type), 1, IOCS::sharedInstance->volumeFile);
    }
    
}