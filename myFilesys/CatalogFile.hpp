//
//  CatalogFile.hpp
//  myFilesys
//
//  Created by Connor Crawford on 4/12/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef CatalogFile_hpp
#define CatalogFile_hpp

#include "Inode.hpp"
#include "FSManager.hpp"

class CatalogFile: public Inode {
    
public:
    CatalogFile();
    CatalogFile(unsigned int blocknum);
    void write();
    void addInode(Inode &inode, FSManager& fsManager);
    void deleteInode(Inode &inode, FSManager &fsManager);
};

#endif /* CatalogFile_hpp */
