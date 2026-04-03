#include "pager.h"

using namespace std;

Pager::Pager(string name){
    this->fileName = name;
    fileHandle.open(fileName, ios::in| ios::out | ios::binary);
    
    if(!fileHandle.is_open())
    {
        fileHandle.clear();
        fileHandle.open(fileName, ios::out | ios::binary);
        fileHandle.close();
        fileHandle.open(fileName, ios::in| ios::out | ios::binary);
    }
}

Pager::~Pager(){
    if(fileHandle.is_open()){
        fileHandle.close();
    }
}

bool Pager::isFileEmpty(){
    if(!fileHandle.is_open()) return true;
    fileHandle.seekg(0, ios::end);
    return fileHandle.tellg() == 0;
}

void Pager::seekAndRead(streampos position, char* outBuffer, size_t size){
    if(!fileHandle.is_open())
        return;

    fileHandle.seekg(position);
    fileHandle.read(outBuffer,size);

    if(fileHandle.fail())
    {
        cout << "Error: Failed to read from file." << endl;
        fileHandle.clear();
    }
}

void Pager::seekAndWrite(streampos position, const char* inBuffer, size_t size){
    if(!fileHandle.is_open())
        return;

    fileHandle.seekp(position);
    fileHandle.write(inBuffer,size);
    fileHandle.flush();
}

void Pager::readNode(int64_t pageId, BTreeNode& node){
    int64_t offset = pageId * BLOCK_SIZE;
    char buffer[BLOCK_SIZE] = {0};
    seekAndRead(offset, buffer, BLOCK_SIZE); 
    memcpy(&node, buffer, sizeof(BTreeNode));
}

void Pager::writeNode(int64_t pageId, const BTreeNode& node){
    int64_t offset = pageId * BLOCK_SIZE;
    char buffer[BLOCK_SIZE] = {0};
    memcpy(buffer, &node, sizeof(BTreeNode));
    seekAndWrite(offset, buffer, BLOCK_SIZE);
}

void Pager::readHeader(FileHeader& header){
    if(!fileHandle.is_open())
        return;
    fileHandle.seekg(0, ios::beg);
    fileHandle.read(reinterpret_cast<char*>(&header), sizeof(FileHeader));
    if(fileHandle.fail())
        cout << "Error: Failed to read from file." << endl;
}

void Pager::writeHeader(const FileHeader& header){
    if(!fileHandle.is_open())
        return;
    fileHandle.seekp(0, ios::beg);
    fileHandle.write(reinterpret_cast<const char*>(&header), sizeof(FileHeader));
    if(fileHandle.fail())
        cout << "Error: Failed to write to file." << endl;
    fileHandle.flush();
}

int64_t Pager::getNewPageID(int64_t& freeListHead) {
    if (freeListHead == -1) {
        fileHandle.seekp(0, ios::end);
        int64_t fileSize = fileHandle.tellp();
        return fileSize / BLOCK_SIZE;
    } 
    
    else {
        int64_t recycledPageId = freeListHead;
        
        int64_t nextFreePageId;
        seekAndRead(recycledPageId * BLOCK_SIZE, reinterpret_cast<char*>(&nextFreePageId), sizeof(int64_t));
        
        freeListHead = nextFreePageId;
        
        return recycledPageId;
    }
}

void Pager::freePage(int64_t deadPageId, int64_t& freeListHead) {
    int64_t nextFreePageId = freeListHead;
    seekAndWrite(deadPageId * BLOCK_SIZE, reinterpret_cast<const char*>(&nextFreePageId), sizeof(int64_t));
    
    freeListHead = deadPageId;
}