//
//  main.cpp
//  myFilesys
//
//  Created by Connor Crawford on 3/30/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

//#include <iostream>
#include <string>
#include "IOCS.hpp"
#include "VCB.hpp"

int main(int argc, const char * argv[]) {
    
    
    IOCS::sharedInstance->create("TEST.iso", 5000);
//    IOCS::sharedInstance->mount("TEST.iso");
    IOCS::sharedInstance->createFile("new.jpg");
    IOCS::sharedInstance->writeFile("maxresdefault.jpg", "new.jpg");
    IOCS::sharedInstance->openFile("new.jpg");
    IOCS::sharedInstance->createFile("new1.jpg");
    IOCS::sharedInstance->writeFile("maxresdefault.jpg", "new1.jpg");
    IOCS::sharedInstance->openFile("new1.jpg");
    IOCS::sharedInstance->deleteFile("new.jpg");
    IOCS::sharedInstance->createFile("new2.jpg");
    IOCS::sharedInstance->writeFile("maxresdefault.jpg", "new2.jpg");
    fclose(IOCS::sharedInstance->volumeFile);
    return 0;
}
