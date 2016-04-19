//
//  Constants.h
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef Constants_h
#define Constants_h

#define BLOCK_SIZE 512
#define MAGIC 0x88
#define VALID 1
#define INVALID 0
#define CATALOG_INODE_OFFSET 512
#define DATA_OFFSET 1024

struct blocknum_t {
    int block:31;
    int valid:1;
};



#endif /* Constants_h */
