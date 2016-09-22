# Connor Crawford, 915047545 - Operating Systems Project 4, File System

HEADERS = CatalogFile.hpp Constants.h DirentBlock.hpp FreeBlock.hpp FSManager.hpp IndirectBlock.hpp Inode.hpp IOCS.hpp VCB.hpp
SOURCE = main.cpp CatalogFile.cpp DirentBlock.cpp FreeBlock.cpp FSManager.cpp IndirectBlock.cpp Inode.cpp IOCS.cpp VCB.cpp
OPTS = -Wall -g -std=c++0x -w

default: myFilesys

%.o: %.c $(HEADERS)
	g++ -c $< -o $@

myFilesys: $(SOURCE)
	g++ $(OPTS) $(SOURCE) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f myFilesys