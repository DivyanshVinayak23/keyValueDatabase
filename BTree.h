#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <string>
#include <cstdint>
#include "pager.h"
#include "nodeStructure.h"

class BTree {
private:
    Pager pager;
    int64_t rootPageId;

    void splitRoot(int64_t oldRootPageId, BTreeNode& oldRoot);
    int64_t splitLeafData(BTreeNode& fullNode, BTreeNode& newNode);
    void insertIntoParent(int64_t parentPageId, int64_t promotedKey, int64_t promotedValue, int64_t rightChildPageId);
    int64_t splitInternalNode(BTreeNode& fullNode, BTreeNode& rightNode, int64_t rightNodePageId);
    void initializeNewFile();

public:
    BTree(std::string fileName);
    int64_t get(int64_t key);
    void set(int64_t key, int64_t value);
};

#endif // BTREE_H
