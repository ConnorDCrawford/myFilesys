//
//  FreeBlock.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include "FreeBlock.hpp"

FreeBlock::FreeBlock() {
    memset(empty, 0x00, BLOCK_SIZE);
}

void FreeBlock::write() {
    fwrite(empty, BLOCK_SIZE, 1, IOCS::sharedInstance->volumeFile);
}