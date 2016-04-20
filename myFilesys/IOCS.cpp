//
//  IOCS.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include <string>
#include <vector>
#include <sstream>
#include <string.h>
#include "IOCS.hpp"
#include "FSManager.hpp"
#include "FreeBlock.hpp"
#include "DirentBlock.hpp"

IOCS::IOCS(){}

IOCS *IOCS::sharedInstance = new IOCS();

void IOCS::mount(std::string volumeName){
    
    // Already have a volume mounted, must unmount it.
    if (volumeFile != NULL) {
        unmount();
    }
    
    volumeFile = fopen(volumeName.c_str(), "rb+");
    if (volumeFile == NULL) {
        std::cerr << "Unable to mount volume.\n";
        return;
    }
    
    // Instantiate volume control block from file
    volumeControlBlock = new VCB();
    volumeControlBlock->read();
    if (volumeControlBlock->magic != MAGIC) {
        std::cerr << "Unable to mount volume. Not a mountable file system.";
    }
    
    // Instantiate the catalog file inode from the data on disk
    catalogFileInode = new CatalogFile(volumeControlBlock->cat.block);
    
    // Instantiate the free space manager from the data on the disk
    freeSpaceManager = new FSManager(volumeControlBlock->alloc.block, 0);
    
    // Assign root directory inode
    rootDirInode = new Inode(volumeControlBlock->root.block);
}

// remove: size in blocks
void IOCS::create(std::string volumeName, size_t size) {
    volumeFile = fopen(volumeName.c_str(), "wb+");
    if (volumeFile == NULL) {
        std::cerr << "ERROR: Unable to create volume.\n";
    }
    
    // Create empty file (all zeros) of designated size
    FreeBlock fb;
    for (int i = 0; i < size; i++) fb.write();
    
    // Create volume control block
    volumeControlBlock = new VCB(MAGIC, BLOCK_SIZE, (int32_t)size, volumeName);
    volumeControlBlock->cat.block = 1;
    volumeControlBlock->alloc.block = 0;
    volumeControlBlock->volumeSize = (int32_t)size;
    volumeControlBlock->write();
    
    // Create Catolog File's inode
    catalogFileInode = new CatalogFile();
    
    // Set up allocation file inode pointer
    // Initially stored in the first data block, this may change
    catalogFileInode->directBlocks[0].block = 0;
    catalogFileInode->directBlocks[0].valid = VALID;
    catalogFileInode->write();
    
    // Create an inode for the allocation file
    Inode allocInode;
    allocInode.number = 0;
    allocInode.write();
    
    // Instantiate the free space manager with the location of the alloc file inode
    freeSpaceManager = new FSManager(0, size-2);
    
    // Create root directory
    rootDirInode = new Inode();
    rootDirInode->isDir = true;
    catalogFileInode->addInode(*rootDirInode, *freeSpaceManager);
    // Set the inode # of root dir in volume control block
    volumeControlBlock->root.block = rootDirInode->number;
    volumeControlBlock->root.valid = VALID;
    volumeControlBlock->write();
}

void IOCS::unmount() {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return;
    }
    std::map<std::string,Inode*>::const_iterator it;
    
    // Close all open files
    for(it = fileTable.begin(); it != fileTable.end(); it++) {
        closeFile(it->first);
    }
    
    fclose(volumeFile);
}

std::vector<std::string> &tokenize(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    if (elems.size() > 0 && elems.front().length()==0) {
        // if user starts path with "/", the first token is empty, so remove it
        elems.erase(elems.begin());
    }
    return elems;
}

void IOCS::createFile(std::string path) {
    return createInode(path, false);
}

void IOCS::createDirectory(std::string path) {
    createInode(path, true);
}

/*
 * This function is used for both createFile() and createDirectory(), as these are logically the
 * same (besides a metadata variable) in my file system.
 */
void IOCS::createInode(std::string path, bool isDirectory) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return;
    }
    Inode* currentInode = rootDirInode;
    DirentBlock* currentDirent = NULL;
    std::vector<std::string> tokens;
    tokenize(path, '/', tokens);
    
    // Go to the directory inode where the file is to be created
    for (int i=0; i < tokens.size()-1; i++) {
        for (int j=0; currentInode->blockPtrWithNumber(j)->valid; j++) {
            if (currentInode->isDir) {
                currentDirent = new DirentBlock(currentInode->blockPtrWithNumber(j)->block);
                if (strcmp(currentDirent->name, tokens[i].c_str()) == 0) {
                    // dirent's name matches token
                    // get the inode number of the next directory/file from the dirent
                    currentInode = new Inode(currentDirent->block.block);
                    break;
                }
                currentDirent = NULL;
            }
        }
        if (currentDirent == NULL) {
            // Unable to find a dirent with specified name
            for (int k = 0; k <= i; k++) {
                std::cout << "/" << tokens[k];
            }
            std::cout << " does not exist.\n";
            return;
        }
    }
    // Retrieved inode of directory
    
    for (int i=0; currentInode->blockPtrWithNumber(i)->valid; i++) {
        currentDirent = new DirentBlock(currentInode->blockPtrWithNumber(i)->block);
        if (strcmp(currentDirent->name, tokens[i].c_str()) == 0) {
            // current inode is a directory inode or the dirent's name matches the name of the new file
            // this is a directory, or a file with same name already exists in this directory, cannot create
            std::cout << "Duplicate file name. Unable to create.\n";
            return;
        }
    }
    
    // Add new file's inode to catalog file
    Inode newInode;
    newInode.isDir = isDirectory;
    newInode.links = 1;
    catalogFileInode->addInode(newInode, *freeSpaceManager);
    
    // Add a dirent for the new inode
    // Assign location of next free block to block pointer
    blocknum_t freeBlock = freeSpaceManager->getFreeBlock();
    // Mark retrieved block as used
    freeSpaceManager->markBlock(freeBlock.block, false);
    // Assign the next pointer in the directory inode to the retrieved free block
    blocknum_t* blockPtr = currentInode->nextFreeBlockPtr();
    blockPtr->block = freeBlock.block;
    blockPtr->valid = VALID;
    currentInode->blocks++;
    currentInode->write();
    
    // Init new dirent
    DirentBlock dirent;
    dirent.block.block = newInode.number;
    dirent.block.valid = VALID;
    strcpy(dirent.name, tokens.back().c_str());
    // Write dirent to disk
    dirent.write(freeBlock);
    
    newInode.write();
}

FILE* IOCS::openFile(std::string path) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return NULL;
    }
    Inode* currentInode = rootDirInode;
    DirentBlock* currentDirent = NULL;
    std::vector<std::string> tokens;
    tokenize(path, '/', tokens);
    
    for (int i=0; i < tokens.size(); i++) {
        for (int j=0; currentInode->blockPtrWithNumber(j)->valid; j++) {
            currentDirent = new DirentBlock(currentInode->blockPtrWithNumber(j)->block);
            if (strcmp(currentDirent->name, tokens[i].c_str()) == 0) {
                // dirent's name matches token
                // get the inode number of the next directory/file from the dirent
                currentInode = new Inode(currentDirent->block.block);
                break;
            }
            currentDirent = NULL;
        }
        if (currentDirent == NULL) {
            // Unable to find a dirent with specified name
            std::cout << path << " does not exist.\n";
            return NULL;
        }
    }
    // Retrieved inode of file
    
    if (currentInode->isDir) {
        std::cout << currentDirent->name << " is a directory.\n";
        return NULL;
    }
    
    // Insert key-value pair of file's path with its inode into open file table
    fileTable.insert(std::pair<std::string, Inode*>(path, currentInode));
    
    return currentInode->open(tokens.back());
}

void IOCS::closeFile(std::string path) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return;
    }
    std::map<std::string,Inode*>::const_iterator it;
    it = fileTable.find(path);
 
    if (it == fileTable.end()) {
        // The file with the specified path is not open
        std::cout << "The specified file is not open or does not exist.";
    } else {
//        Inode* inode = it->second;
        std::string path = it->first;
        std::vector<std::string> tokens;
        tokenize(path, '/', tokens);
//        FILE* fp = fopen(tokens.back().c_str(), "r"); // Open the temp file previously made
        
        // REMOVE: do you really want to update file before closing?
//        if (fp != NULL) {
//            // Update file in myFileSys before closing
//            inode->writeData(fp);
//            fclose(fp);
//        }
        
        // Remove the file from the user's actual file system
        std::remove(tokens.back().c_str());
    }
}

bool IOCS::writeFile(std::string sourcePath, std::string destPath) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return false;
    }
    std::map<std::string,Inode*>::const_iterator it;
    it = fileTable.find(destPath);
    
    if (it != fileTable.end()) {
        FILE* fp = fopen(sourcePath.c_str(), "r");
        if (fp) {
            Inode* inode = it->second;
            std::string path = it->first;
            std::vector<std::string> tokens;
            tokenize(path, '/', tokens);
            
            inode->deleteData(false);
            inode->writeData(fp);
            inode->open(tokens.back()); // update user-facing file
            return true;
        }
        
    }
    
    // The file with the specified path is not open
    std::cout << "The specified file is not open or does not exist.\n";
    return false;
}

void IOCS::readFile(std::string path) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return;
    }
    std::map<std::string,Inode*>::const_iterator it;
    it = fileTable.find(path);
    
    if (it != fileTable.end()) {
        Inode* inode = it->second;
        inode->readData();
    } else {
        // The file with the specified path is not open
        std::cout << "The specified file is not open or does not exist.\n";
    }
}

void IOCS::deleteFile(std::string path, Inode* dirInode) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return;
    }
    Inode* currentInode = dirInode==NULL ? rootDirInode : dirInode; // start with provided inode if provided
    int blocknum = 0;
    DirentBlock* currentDirent = NULL;
    std::vector<std::string> tokens;
    tokenize(path, '/', tokens);
    
    // Go to dirent's directory inode
    for (int i=0; i < tokens.size(); i++) {
        for (int j=0; j < currentInode->blocks; j++) {
            if (currentInode->blockPtrWithNumber(j)->valid) {
                currentDirent = new DirentBlock(currentInode->blockPtrWithNumber(j)->block);
                if (strcmp(currentDirent->name, tokens[i].c_str()) == 0) {
                    // dirent's name matches token
                    // get the inode number of the next directory/file from the dirent
                    
                    // Obtained dirent of file to be deleted
                    if (currentDirent->name == tokens.back()) {
                        // Save the inode of the directory that contains it
                        dirInode = currentInode;
                        // Save its block number in directory
                        blocknum = j;
                    }
                    
                    // Get the inode of the file to be deleted
                    currentInode = new Inode(currentDirent->block.block);
                    break;
                }
                currentDirent = NULL;
            }
        }
        if (currentDirent == NULL) {
            // Unable to find a dirent with specified name
            std::cout << "File does not exist.\n";
            return;
        }
        
    }
    // Retrieved inode of file
    
    // If deleting a directory, delete its subcontents
    if (currentInode->isDir) {
        DirentBlock* cDirent;
        Inode inode = *currentInode;
        for (int i = 0; currentInode->blockPtrWithNumber(i)->valid; i++) {
            cDirent = new DirentBlock(currentInode->blockPtrWithNumber(i)->block);
            deleteFile(cDirent->name, &inode);
        }
    }
    
    // Mark dirent for deletion
    dirInode->blockPtrWithNumber(blocknum)->valid = INVALID;
    dirInode->blocks--;
    dirInode->write();
    freeSpaceManager->markBlock(dirInode->blockPtrWithNumber(blocknum)->block, true);
    
    // Decrement number of links to file. If zero, delete inode and its data
    if (--(currentInode->links) == 0) {
        catalogFileInode->deleteInode(*currentInode, *freeSpaceManager);
        
        std::map<std::string,Inode*>::const_iterator it;
        it = fileTable.find(path);
        
        if (it != fileTable.end()) {
            // The file with the specified path is open
            // Remove the file from the user's actual file system
            std::remove(tokens.back().c_str());
        }
        
    } else {
        currentInode->write(); // Update link count
    }
    
}

void IOCS::list(std::string path) {
    if (volumeFile == NULL) {
        std::cout << "No mounted volume. Create or mount a volume to continue.\n";
        return;
    }
    Inode* currentInode = rootDirInode;
    DirentBlock* currentDirent = NULL;
    std::vector<std::string> tokens;
    tokenize(path, '/', tokens);
    
    for (int i=0; i < tokens.size(); i++) {
        for (int j=0; currentInode->blockPtrWithNumber(j)->valid; j++) {
            currentDirent = new DirentBlock(currentInode->blockPtrWithNumber(j)->block);
            if (strcmp(currentDirent->name, tokens[i].c_str()) == 0) {
                // dirent's name matches token
                // get the inode number of the next directory/file from the dirent
                currentInode = new Inode(currentDirent->block.block);
                break;
            }
            currentDirent = NULL;
        }
        if (currentDirent == NULL) {
            // Unable to find a dirent with specified name
            std::cout << path << " does not exist.\n";
            return;
        }
    }
    // Retrieved inode of file
    
    if (!currentInode->isDir) {
        std::cout << currentDirent->name << " is a file.\n";
        return;
    }
    
    for (int i = 0; currentInode->blockPtrWithNumber(i)->valid; i++) {
        currentDirent = new DirentBlock(currentInode->blockPtrWithNumber(i)->block);
        std::cout << "\t" << currentDirent->name;
    }
    std::cout << "\n";
}