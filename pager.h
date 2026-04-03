#ifndef PAGER_H
#define PAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include "nodeStructure.h"

class Pager {
private:
    std::fstream fileHandle;
    std::string fileName;

public:
    Pager(std::string name);
    ~Pager();

    bool isFileEmpty();
    
    void seekAndRead(std::streampos position, char* outBuffer, size_t size);
    void seekAndWrite(std::streampos position, const char* inBuffer, size_t size);

    void readNode(int64_t pageId, BTreeNode& node);
    void writeNode(int64_t pageId, const BTreeNode& node);

    void readHeader(FileHeader& header);
    void writeHeader(const FileHeader& header);

    int64_t getNewPageID(int64_t& freeListHead);
    void freePage(int64_t deadPageId, int64_t& freeListHead);
};

#endif // PAGER_H
