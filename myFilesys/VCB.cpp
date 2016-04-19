//
//  VCB.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#include "VCB.hpp"
#include "IOCS.hpp"

VCB::VCB(){}

VCB::VCB(int magic, int blockSize, int volumeSize, std::string name):
magic(magic),
blocksize(blockSize),
volumeSize(volumeSize)
{
    strcpy(this->name, name.c_str());
}

void VCB::write() {
    // write data to block
    // Go to beginning of volume
    fseek(IOCS::sharedInstance->volumeFile, 0, SEEK_SET);
    
    fwrite(&magic, sizeof(magic), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&blocksize, sizeof(blocksize), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&cat, sizeof(cat), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&alloc, sizeof(alloc), 1, IOCS::sharedInstance->volumeFile);
    fwrite(&root, sizeof(root), 1, IOCS::sharedInstance->volumeFile);
    fwrite(name, sizeof(name), 1, IOCS::sharedInstance->volumeFile);
    
    // Fill unused space
    char zero = 0;
    for (int i = 0; i < 232; i++) fwrite(&zero, 1, 1, IOCS::sharedInstance->volumeFile);
}

void VCB::read() {
    // Go to beginning of volume
    fseek(IOCS::sharedInstance->volumeFile, 0, SEEK_SET);
    
    // write data to block
    fread(&magic, sizeof(magic), 1, IOCS::sharedInstance->volumeFile);
    fread(&blocksize, sizeof(blocksize), 1, IOCS::sharedInstance->volumeFile);
    fread(&cat, sizeof(cat), 1, IOCS::sharedInstance->volumeFile);
    fread(&alloc, sizeof(alloc), 1, IOCS::sharedInstance->volumeFile);
    fread(&root, sizeof(root), 1, IOCS::sharedInstance->volumeFile);
    fread(name, sizeof(name), 1, IOCS::sharedInstance->volumeFile);
}