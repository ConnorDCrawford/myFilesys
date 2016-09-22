//
//  Inode.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include <unistd.h>
#include <sys/wait.h>
#include "Inode.hpp"
#include "IOCS.hpp"

Inode::Inode():
number(0),
links(0),
blocks(0),
isDir(false),
cTime(time(0)),
mTime(time(0)),
aTime(time(0))
{}

Inode::Inode(unsigned int number) {
    blocknum_t inodePointer;
    FILE* volumeFile;
    
    inodePointer = *IOCS::sharedInstance->catalogFileInode->blockPtrWithNumber(number);
    volumeFile = IOCS::sharedInstance->volumeFile;
    
    // Go to inode's block
    fseek(volumeFile, DATA_OFFSET+(inodePointer.block*BLOCK_SIZE), SEEK_SET);
    
    // Set instance variables to data stored on disk
    fread(&(this->number), sizeof(this->number), 1, volumeFile);
    fread(&links, sizeof(links), 1, volumeFile);
    fread(&blocks, sizeof(blocks), 1, volumeFile);
    fread(&isDir, sizeof(isDir), 1, volumeFile);
    fread(&cTime, sizeof(cTime), 1, volumeFile);
    fread(&mTime, sizeof(mTime), 1, volumeFile);
    fread(&aTime, sizeof(aTime), 1, volumeFile);
    
    fread(directBlocks, sizeof(blocknum_t), 12, volumeFile);
    fread(&indirectBlock, sizeof(indirectBlock), 1, volumeFile);
    fread(&dIndirectBlock, sizeof(dIndirectBlock), 1, volumeFile);
    fread(&tIndirectBlock, sizeof(tIndirectBlock), 1, volumeFile);
    fread(&_lastBlockSize, sizeof(_lastBlockSize), 1, volumeFile);
}


FILE* Inode::open(std::string name) {
    char buffer[512];
    FILE *outFile = fopen(name.c_str(), "wb+");
    int i = 0;
    size_t read;
    while (blockPtrWithNumber(i)->valid) {
        fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(blockPtrWithNumber(i)->block*BLOCK_SIZE), SEEK_SET);
        
        if (i < blocks - 1) {
            read = fread(buffer, 1, BLOCK_SIZE, IOCS::sharedInstance->volumeFile); // Read from drive to buffer
        } else {
            // Last segement of data that may not be the full block size, write only used data
            read = fread(buffer, 1, _lastBlockSize, IOCS::sharedInstance->volumeFile);
        }
        
        if (read > 0) {
            fwrite(buffer, 1, read, outFile); // Write from buffer to out file if able to read at least one byte
        }
        i++;
        
    }
    
    fflush(outFile);
    return outFile;
}

void Inode::write() {
    blocknum_t inodePointer;
    FILE* volumeFile;
    
    inodePointer = *IOCS::sharedInstance->catalogFileInode->blockPtrWithNumber(number);
    volumeFile = IOCS::sharedInstance->volumeFile;
    
    // Go to inode's block
    fseek(volumeFile, DATA_OFFSET+(inodePointer.block*BLOCK_SIZE), SEEK_SET);
    
    // Set instance variables to data stored on disk
    fwrite(&(this->number), sizeof(this->number), 1, volumeFile);
    fwrite(&links, sizeof(links), 1, volumeFile);
    fwrite(&blocks, sizeof(blocks), 1, volumeFile);
    fwrite(&isDir, sizeof(isDir), 1, volumeFile);
    fwrite(&cTime, sizeof(cTime), 1, volumeFile);
    fwrite(&mTime, sizeof(mTime), 1, volumeFile);
    fwrite(&aTime, sizeof(aTime), 1, volumeFile);
    fwrite(directBlocks, sizeof(blocknum_t), 12, volumeFile);
    fwrite(&indirectBlock, sizeof(indirectBlock), 1, volumeFile);
    fwrite(&dIndirectBlock, sizeof(dIndirectBlock), 1, volumeFile);
    fwrite(&tIndirectBlock, sizeof(tIndirectBlock), 1, volumeFile);
    fwrite(&_lastBlockSize, sizeof(_lastBlockSize), 1, volumeFile);
}

blocknum_t* Inode::blockPtrWithNumber(unsigned int number) {
    
    // Direct blocks
    if (number < 12) {
        return &directBlocks[number];
    }
    
    // Indirect block
    else if (number < 140) { // 128+12
        IndirectBlock* indrctBlock;
        if (!indirectBlock.valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            indirectBlock = free;
            indirectBlock.valid = VALID;
            
            indrctBlock = new IndirectBlock();
            indrctBlock->blocknum = free;
            write();
        } else {
            indrctBlock = new IndirectBlock(indirectBlock);
        }
        
        return &indrctBlock->blocks[number-12];
    }
    
    // Doubly indirect block
    else if (number < 16524) { // 128^2 + 128 + 12
        IndirectBlock* dIndrctBlock;
        if (!dIndirectBlock.valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            dIndirectBlock = free;
            dIndirectBlock.valid = VALID;
            
            dIndrctBlock = new IndirectBlock();
            dIndrctBlock->blocknum = free;
            write();
        } else {
            // Go double block pointer
            dIndrctBlock = new IndirectBlock(dIndirectBlock);
        }
        
        // Determine which block the block pointer is in
        int offset = number - 140;
        int dIndex = offset / 128; // 128^1
        
        IndirectBlock* indrctBlock;
        if (!dIndrctBlock->blocks[dIndex].valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            dIndrctBlock->blocks[dIndex] = free;
            dIndrctBlock->blocks[dIndex].valid = VALID;
            
            indrctBlock = new IndirectBlock();
            indrctBlock->blocknum = free;
            dIndrctBlock->write();
        } else {
            // Get the indirect block
            indrctBlock = new IndirectBlock(dIndrctBlock->blocks[dIndex]);
        }
        
        int index = offset % 128;
        
        return &indrctBlock->blocks[index];
    }
    
    // Trebly indirect block
    else if (number < 2113676) { // 128^3 + 128^2 + 128 + 12
        IndirectBlock* tIndrctBlock;
        if (!tIndirectBlock.valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            tIndirectBlock = free;
            tIndirectBlock.valid = VALID;
            
            tIndrctBlock = new IndirectBlock();
            tIndrctBlock->blocknum = free;
            write();
        } else {
            // Go double block pointer
            tIndrctBlock = new IndirectBlock(dIndirectBlock);
        }
        
        // Determine which block the double block pointer is in
        int offset = number-16524;
        int tIndex = offset / 16384; // 128^2
        
        IndirectBlock* dIndrctBlock;
        if (!tIndrctBlock->blocks[tIndex].valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            tIndrctBlock->blocks[tIndex] = free;
            tIndrctBlock->blocks[tIndex].valid = VALID;
            
            dIndrctBlock = new IndirectBlock();
            dIndrctBlock->blocknum = free;
            tIndrctBlock->write();
        } else {
            // Get the double indirect block
            dIndrctBlock = new IndirectBlock(tIndrctBlock->blocks[tIndex]);
        }
        
        // Determine which block the block pointer is in
        int dIndex = offset % 16384;
        
        IndirectBlock* indrctBlock;
        if (!dIndrctBlock->blocks[dIndex].valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            dIndrctBlock->blocks[dIndex] = free;
            dIndrctBlock->blocks[dIndex].valid = VALID;
            
            indrctBlock = new IndirectBlock();
            indrctBlock->blocknum = free;
            dIndrctBlock->write();
        } else {
            // Get the indirect block
            indrctBlock = new IndirectBlock(dIndrctBlock->blocks[dIndex]);
        }
        
        // Determine the block
        int index = dIndex % 128;
        
        return &indrctBlock->blocks[index];
    }
    
    return NULL;
}

blocknum_t* Inode::nextFreeBlockPtr() {
    for (int i = 0; i < sizeof(directBlocks); i++) {
        if (!directBlocks[i].valid)
            return &directBlocks[i];
    }
    if (indirectBlock.valid) {
        IndirectBlock indrctBlock = *new IndirectBlock(indirectBlock);
        return indrctBlock.nextFreeBlockPointer();
    }
    if (dIndirectBlock.valid) {
        // Go double block pointer
        IndirectBlock dIndrctBlock = *new IndirectBlock(dIndirectBlock);
        
        // Go through double block pointer's valid blocks pointers
        for (int i = 0; i < 128; i++) {
            if (dIndrctBlock.blocks[i].valid) {
                // Get the indirect block
                IndirectBlock indrctBlock = *new IndirectBlock(dIndrctBlock.blocks[i]);
                // See if any blocks available
                blocknum_t* free = indrctBlock.nextFreeBlockPointer();
                if (free != NULL) {
                    return free;
                }
            }
        }
        
    }
    
    if (tIndirectBlock.valid) {
        // Go double block pointer
        IndirectBlock tIndrctBlock = *new IndirectBlock(dIndirectBlock);
        
        // Go through triple block pointer's valid blocks pointers
        for (int i = 0; i < 128; i++) {
            
            if (tIndrctBlock.blocks[i].valid) {
                // Get the double indirect block
                IndirectBlock dIndrctBlock = *new IndirectBlock(tIndrctBlock.blocks[i]);
                for (int j = 0; j < 128; j++) {
                    
                    if (dIndrctBlock.blocks[j].valid) {
                        // Get the indirect block
                        IndirectBlock indrctBlock = *new IndirectBlock(dIndrctBlock.blocks[j]);
                        // See if any blocks available
                        blocknum_t* free = indrctBlock.nextFreeBlockPointer();
                        if (free != NULL) {
                            return free;
                        }
                    }
                    
                }
                
            }
            
        }
        
    }
    
    return NULL;
}

void Inode::setBlockPtr(blocknum_t blocknum, unsigned int withNumber) {
    // Direct blocks
    if (withNumber < 12) {
        directBlocks[withNumber] = blocknum;
        write();
    }
    
    // Indirect block
    else if (withNumber < 140) { // 128+12
        IndirectBlock* indrctBlock;
        if (!indirectBlock.valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            indirectBlock = free;
            indirectBlock.valid = VALID;
            
            indrctBlock = new IndirectBlock();
            indrctBlock->blocknum = free;
            write();
        } else {
            indrctBlock = new IndirectBlock(indirectBlock);
        }
        
        indrctBlock->blocks[withNumber-12] = blocknum;
        indrctBlock->write();
    }
    
    // Doubly indirect block
    else if (withNumber < 16524) { // 128^2 + 128 + 12
        IndirectBlock* dIndrctBlock;
        if (!dIndirectBlock.valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            dIndirectBlock = free;
            dIndirectBlock.valid = VALID;
            
            dIndrctBlock = new IndirectBlock();
            dIndrctBlock->blocknum = free;
            write();
        } else {
            // Go double block pointer
            dIndrctBlock = new IndirectBlock(dIndirectBlock);
        }
        
        // Determine which block the block pointer is in
        int offset = withNumber - 140;
        int dIndex = offset / 128; // 128^1
        
        IndirectBlock* indrctBlock;
        if (!dIndrctBlock->blocks[dIndex].valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            dIndrctBlock->blocks[dIndex] = free;
            dIndrctBlock->blocks[dIndex].valid = VALID;
            
            indrctBlock = new IndirectBlock();
            indrctBlock->blocknum = free;
            dIndrctBlock->write();
        } else {
            // Get the indirect block
            indrctBlock = new IndirectBlock(dIndrctBlock->blocks[dIndex]);
        }
        
        int index = offset % 128;
        indrctBlock->blocks[index] = blocknum;
        indrctBlock->write();
    }
    
    // Trebly indirect block
    else if (withNumber < 2113676) { // 128^3 + 128^2 + 128 + 12
        IndirectBlock* tIndrctBlock;
        if (!tIndirectBlock.valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            tIndirectBlock = free;
            tIndirectBlock.valid = VALID;
            
            tIndrctBlock = new IndirectBlock();
            tIndrctBlock->blocknum = free;
            write();
        } else {
            // Go double block pointer
            tIndrctBlock = new IndirectBlock(dIndirectBlock);
        }
        
        // Determine which block the double block pointer is in
        int offset = withNumber-16524;
        int tIndex = offset / 16384; // 128^2
        
        IndirectBlock* dIndrctBlock;
        if (!tIndrctBlock->blocks[tIndex].valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            tIndrctBlock->blocks[tIndex] = free;
            tIndrctBlock->blocks[tIndex].valid = VALID;
            
            dIndrctBlock = new IndirectBlock();
            dIndrctBlock->blocknum = free;
            tIndrctBlock->write();
        } else {
            // Get the double indirect block
            dIndrctBlock = new IndirectBlock(tIndrctBlock->blocks[tIndex]);
        }
        
        // Determine which block the block pointer is in
        int dIndex = offset % 16384;
        
        IndirectBlock* indrctBlock;
        if (!dIndrctBlock->blocks[dIndex].valid) {
            blocknum_t free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            dIndrctBlock->blocks[dIndex] = free;
            dIndrctBlock->blocks[dIndex].valid = VALID;
            
            indrctBlock = new IndirectBlock();
            indrctBlock->blocknum = free;
            dIndrctBlock->write();
        } else {
            // Get the indirect block
            indrctBlock = new IndirectBlock(dIndrctBlock->blocks[dIndex]);
        }
        
        // Determine the block
        int index = dIndex % 128;
        indrctBlock->blocks[index] = blocknum;
        indrctBlock->write();
    }
}

bool Inode::writeData(FILE* fromFile) {
    char buffer[512];
    int i = 0;
    size_t read;
    blocknum_t free;
    
    do {
        read = fread(buffer, 1, BLOCK_SIZE, fromFile);
        
        if (read > 0) {
            free = IOCS::sharedInstance->freeSpaceManager->getFreeBlock();
            if (free.valid == INVALID) {
                // Volume is full
                return false;
            }
            IOCS::sharedInstance->freeSpaceManager->markBlock(free.block, false);
            setBlockPtr(free, i++);
            blocks++;
            
            fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(free.block*BLOCK_SIZE), SEEK_SET);
            fwrite(buffer, 1, read, IOCS::sharedInstance->volumeFile);
            
            if (read < BLOCK_SIZE) {
                _lastBlockSize = (int32_t)read;
            }
        }
        
    } while (read > 0);
    write();
    
    return true;
}

void Inode::readData() {
    char buffer[512];
    int i = 0;
    size_t read;
    pid_t cPID;
    
    FILE* tmpfp;
    char format[] = "/tmp/temp.XXXXXX";
    const char *name = mktemp(format); // Get temp name
    tmpfp = fopen(name, "w+");  // Create the file
    
    if (tmpfp != NULL) {
        while (blockPtrWithNumber(i)->valid) {
            fseek(IOCS::sharedInstance->volumeFile, DATA_OFFSET+(blockPtrWithNumber(i)->block*BLOCK_SIZE), SEEK_SET);
            
            if (i < blocks-1) {
                read = fread(buffer, 1, BLOCK_SIZE, IOCS::sharedInstance->volumeFile); // Read from drive to buffer
            } else {
                // Last segement of data that may not be the full block size, write only used data
                read = fread(buffer, 1, _lastBlockSize, IOCS::sharedInstance->volumeFile);
            }
            
            if (read > 0) {
                fwrite(buffer, 1, read, tmpfp); // Write from buffer to out file if able to read at least one byte
            }
            i++;
        }
        fflush(tmpfp);
        
        cPID = fork();
        if (cPID == 0) {
            execlp("more", "more", name, NULL); // execute more filter with readme file
            perror("execlp"); // exec failed
        } else {
            waitpid(cPID, NULL, 0); // wait for process to complete
        }
        fclose(tmpfp);
        remove(name);
    }
    
}

void Inode::deleteData(bool shouldDeleteEntry) {
    int i = 0;
    blocknum_t* nextBlock = blockPtrWithNumber(0);
    
    // Delete data blocks
    while (nextBlock->valid) {
        IOCS::sharedInstance->freeSpaceManager->markBlock(nextBlock->block, true);
        nextBlock->valid = INVALID;
        nextBlock = blockPtrWithNumber(++i);
    }
    
    // Delete indirect blocks
    if (indirectBlock.valid) {
        IOCS::sharedInstance->freeSpaceManager->markBlock(indirectBlock.block, true);
        indirectBlock.valid = INVALID;
    }
    
    if (dIndirectBlock.valid) {
        IndirectBlock dIndrct(dIndirectBlock);
        for (int i = 0; i < 128; i++) {
            if (dIndrct.blocks[i].valid) {
                IOCS::sharedInstance->freeSpaceManager->markBlock(dIndrct.blocks[i].block, true);
            }
        }
        
        IOCS::sharedInstance->freeSpaceManager->markBlock(dIndirectBlock.block, true);
        dIndirectBlock.valid = INVALID;
    }
    
    if (tIndirectBlock.valid) {
        IndirectBlock tIndrct(tIndirectBlock);
        for (int i = 0; i < 128; i++) {
            if (tIndrct.blocks[i].valid) {
                
                IndirectBlock dIndrct(tIndrct.blocks[i]);
                for (int i = 0; i < 128; i++) {
                    if (dIndrct.blocks[i].valid) {
                        IOCS::sharedInstance->freeSpaceManager->markBlock(dIndrct.blocks[i].block, true);
                    }
                }
                
                IOCS::sharedInstance->freeSpaceManager->markBlock(tIndrct.blocks[i].block, true);
            }
        }
        
        IOCS::sharedInstance->freeSpaceManager->markBlock(tIndirectBlock.block, true);
        tIndirectBlock.valid = INVALID;
    }
    
    if (!shouldDeleteEntry) { // Don't bother updating if inode is going to be deleted
        write();
    }
}