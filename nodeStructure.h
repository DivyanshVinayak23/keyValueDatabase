#ifndef NODESTRUCTURE_H
#define NODESTRUCTURE_H

#include <cstdint>
#include <cstring>

constexpr size_t BLOCK_SIZE = 4096;
constexpr int MAX_KEYS = 169;

#pragma pack(push, 1)

struct FileHeader {
    uint32_t magicNumber;    // 4 bytes
    int64_t rootOffset;      // 8 bytes
    int64_t freeListHead;    // 8 bytes
    // 4 + 8 + 8 = 20 bytes used.
    //pad the rest to ensure the header takes exactly one block.
    char padding[BLOCK_SIZE - 20];
};

struct BTreeNode {
    bool isLeaf;             // 1 byte
    int32_t keyCount;        // 4 bytes
    int64_t parentOffset;    // 8 bytes

    int64_t childrenOffsets[MAX_KEYS + 1];

    int64_t keys[MAX_KEYS];

    int64_t values[MAX_KEYS];

    BTreeNode() {
        isLeaf = false;
        keyCount = 0;
        parentOffset = -1;
        memset(childrenOffsets, -1, sizeof(childrenOffsets));
        memset(keys, 0, sizeof(keys));
        memset(values, 0, sizeof(values));
    }
};
#pragma pack(pop)

#endif // NODESTRUCTURE_H
