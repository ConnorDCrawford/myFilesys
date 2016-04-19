//
//  FSManager.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include "FSManager.hpp"
#include "IOCS.hpp"
//#include "DirentBlock.hpp"
//#include "DataBlock.hpp"

FSManager::FSManager(){}

FSManager::FSManager(unsigned int inodeNumber, size_t dataSegmentBlocks):
allocationFileInode(Inode(inodeNumber)) {
    if (allocationFileInode.blockPtrWithNumber(0)->valid == INVALID) {
        
        // Creating a new allocation file
        
        int freeBlock;
        
        // Go to block 1 for allocation file's first data block, since block 0 contains its inode
        fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET + BLOCK_SIZE, SEEK_SET);
        
        // Write a bitmap with each bit representing a block on the volume
        char zero = 0;
        int i = 0;
        
        for (; i < dataSegmentBlocks/8; i++) {
            fwrite(&zero, 1, 1, IOCS::sharedInstance->volumeFile);
            
            if (i == 0) {
                // the free space bitmap does not yet exist, so we need to create the first data block for it
                // set allocation file's first data block location to 1, since block 0 contains its inode
                allocationFileInode.blockPtrWithNumber(0)->block = 1;
                allocationFileInode.blockPtrWithNumber(0)->valid = VALID;
                markBlock(0, false);
                markBlock(1, false);
            }
            else if (i % 511 == 0) {
                // allocation file data needs another block
                freeBlock = getFreeBlock().block;
                allocationFileInode.nextFreeBlockPtr()->block = freeBlock;
                markBlock(freeBlock, false);
                allocationFileInode.nextFreeBlockPtr()->valid = VALID;
            }
        }
        if (dataSegmentBlocks%8 != 0) {
            
            if (i % 511 == 0) {
                // allocation file data needs another block
                freeBlock = getFreeBlock().block;
                allocationFileInode.nextFreeBlockPtr()->block = freeBlock;
                markBlock(freeBlock, false);
                allocationFileInode.nextFreeBlockPtr()->valid = VALID;
            }
            // need to mark end bits as used, because they do not exist
            unsigned char lastByte = 0xFF;
            lastByte >>= dataSegmentBlocks%8;
            fwrite(&lastByte, 1, 1, IOCS::sharedInstance->volumeFile);
            i++;
        }
        
        // Fill leftover bytes with used indicators
        char used = 0xFF;
        for (; i < 512; i++) {
            fwrite(&used, 1, 1, IOCS::sharedInstance->volumeFile);
        }
        
        // write allocation file inode to disk
        allocationFileInode.write();
    }
}

blocknum_t FSManager::getFreeBlock() {
    int i = 0;
    char current = 0;
    blocknum_t freeBlock;
    freeBlock.block = 0;
    
    while (allocationFileInode.blockPtrWithNumber(i)->valid) {
        fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET + (allocationFileInode.blockPtrWithNumber(i)->block*BLOCK_SIZE), SEEK_SET);
        
        for (int i = 0; i < BLOCK_SIZE; i++) {
            fread(&current, 1, 1, IOCS::sharedInstance->volumeFile);
            
            if (~current) {
                // At least one of these 8 blocks is free
                // find which block in the 8 is free
                while (current & 0x80) {
                    current <<= 1;
                    freeBlock.block++;
                }
                freeBlock.valid = VALID;
                return freeBlock;
            }
            
            freeBlock.block += 8;
        }
        i++;
    }
    freeBlock.block = -1;
    freeBlock.valid = INVALID;
    return freeBlock;
}

void FSManager::markBlock(unsigned int blocknum, bool free) {
    int blockPtrNum = blocknum/(BLOCK_SIZE*8); // Get which block pointer the block's indicator bit is in
    int byte = (blocknum % (BLOCK_SIZE*8)) / 8;
    char bit = 0x80 >> (blocknum % 8);
    char c;
    
    // Go to character containing the bit reperesenting the designated block
    fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET + (allocationFileInode.blockPtrWithNumber(blockPtrNum)->block*BLOCK_SIZE)+ byte, SEEK_SET);
    
    fread(&c, 1, 1, IOCS::sharedInstance->volumeFile);
    
    if (free) {
        // mark block's bit as free, so make it 0 (ex. 1111 w/ 0100 becomes 1011)
        c &= ~bit;
    } else {
        // mark block's bit as used, so make it 1  (ex. 0000 w/ 0100 becomes 0100)
        c |= bit;
    }
    
    // Go back to character containing the bit reperesenting the designated block
    fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET + (allocationFileInode.blockPtrWithNumber(blockPtrNum)->block*BLOCK_SIZE) + byte, SEEK_SET);
    
    // Replace byte with modified one
    fwrite(&c, 1, 1, IOCS::sharedInstance->volumeFile);
}

//blocknum_t FSManager::getContiguousFreeSpace(size_t &size) {
//    int i = 0;
//    char current = 0;
//    blocknum_t freeBlock;
//    
//    while (allocationFileInode.blockPtrWithNumber(i)->valid == VALID) {
//        fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+allocationFileInode.blockPtrWithNumber(i)->block, SEEK_SET);
//        fread(&current, 1, 1, IOCS::sharedInstance->volumeFile);
//        
//        if (current ^ 0xFF) {
//            // At least one of these 8 blocks is free
//            // find which block in the 8 is free
//            while (current & 0x) {
//                current <<= 1;
//                freeBlock.block++;
//            }
//        }
//        
//        freeBlock.block += 8;
//    }
//    return freeBlock;
//}