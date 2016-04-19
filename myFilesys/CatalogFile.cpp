//
//  CatalogFile.cpp
//  myFilesys
//
//  Created by Connor Crawford on 4/12/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include "CatalogFile.hpp"
#include "IOCS.hpp"
#include "VCB.hpp"

CatalogFile::CatalogFile() {}

CatalogFile::CatalogFile(unsigned int blocknum) {
    // Go to inode's block
    fseek(IOCS::sharedInstance->volumeFile, blocknum*BLOCK_SIZE, SEEK_SET);
    
    // Set instance variables to data stored on disk
    fread(&(this->number), sizeof(this->number), 1, IOCS::sharedInstance->volumeFile);
    fread(&links, sizeof(links), 1, IOCS::sharedInstance->volumeFile);
    fread(&blocks, sizeof(blocks), 1, IOCS::sharedInstance->volumeFile);
    fread(&isDir, sizeof(isDir), 1, IOCS::sharedInstance->volumeFile);
    fread(&cTime, sizeof(cTime), 1, IOCS::sharedInstance->volumeFile);
    fread(&mTime, sizeof(mTime), 1, IOCS::sharedInstance->volumeFile);
    fread(&aTime, sizeof(aTime), 1, IOCS::sharedInstance->volumeFile);
    for (int i = 0; i < 12; i++) {
        fread(&directBlocks[i], sizeof(blocknum_t), 1, IOCS::sharedInstance->volumeFile);
    }
    fread(&indirectBlock, sizeof(indirectBlock), 1, IOCS::sharedInstance->volumeFile);
    fread(&dIndirectBlock, sizeof(dIndirectBlock), 1, IOCS::sharedInstance->volumeFile);
    fread(&tIndirectBlock, sizeof(tIndirectBlock), 1, IOCS::sharedInstance->volumeFile);
}

void CatalogFile::write() {
    
    // Go to inode's block
    fseek(IOCS::sharedInstance->volumeFile, IOCS::sharedInstance->volumeControlBlock->cat.block*BLOCK_SIZE, SEEK_SET);
    
    // Write instance variables to disk
    fwrite(&(this->number), sizeof(this->number), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&links, sizeof(links), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&blocks, sizeof(blocks), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&isDir, sizeof(isDir), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&cTime, sizeof(cTime), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&mTime, sizeof(mTime), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&aTime, sizeof(aTime), 1, IOCS::sharedInstance->volumeFile);
    for (int i = 0; i < 12; i++) {
        fwrite(&directBlocks[i], sizeof(blocknum_t), 1, IOCS::sharedInstance->volumeFile);
    }
    fwrite(&indirectBlock, sizeof(indirectBlock), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&dIndirectBlock, sizeof(dIndirectBlock), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&tIndirectBlock, sizeof(tIndirectBlock), 1, IOCS::sharedInstance->volumeFile);
    
    // Fill unused space to fill block
    char empty[415];
    fwrite(&empty, 1, 415, IOCS::sharedInstance->volumeFile);
}

void CatalogFile::addInode(Inode &inode, FSManager& fsManager) {
    // Find a block to hold the new inode
    int freeBlock = fsManager.getFreeBlock().block;
    // Assign the next available pointer to the block's location
    nextFreeBlockPtr()->block = freeBlock;
    nextFreeBlockPtr()->valid = VALID;
    // Mark the block as used
    fsManager.markBlock(freeBlock, false);
    
    write();
    
    // Set the inode's number to the next highest
    inode.number = ++blocks;
    inode.write();
}

void CatalogFile::deleteInode(Inode &inode, FSManager &fsManager) {
    blocknum_t* blockPtr = blockPtrWithNumber(inode.number);
    
    inode.deleteData(true);
    
    if (blockPtr != NULL) {
        fsManager.markBlock(blockPtr->block, true); // Mark inode's block as free
        blockPtr->valid = INVALID;
        write();
    }
}