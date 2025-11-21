#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace std;

constexpr size_t BLOCK_SIZE = 4096;

#pragma pack(push, 1)

struct FileHeader {
    uint32_t magicNumber;    // 4 bytes
    int64_t rootOffset;      // 8 bytes
    int64_t freeListHead;    // 8 bytes
    
    // 4 + 8 + 8 = 20 bytes used.
    //pad the rest to ensure the header takes exactly one block.
    char padding[BLOCK_SIZE - 20];
};

// bool (1) + int32 (4) + int64 (8) = 13 bytes.
//
// For each key (Order M), we have:
// 1 Key (8 bytes)
// 1 Value (8 bytes)
// 1 Child Offset (8 bytes)
//
// Equation: 13 + (N * 8) + (N * 8) + ((N + 1) * 8) <= 4096
// 13 + 24N + 8 <= 4096
// 24N <= 4075
// N ~= 169
constexpr int MAX_KEYS = 169;

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


static_assert(sizeof(FileHeader) == BLOCK_SIZE, "FileHeader must be exactly BLOCK_SIZE");
static_assert(sizeof(BTreeNode) <= BLOCK_SIZE, "BTreeNode structure exceeds block size!");

int main() {
    cout << "Configuration Check:" << endl;
    cout << "Block Size: " << BLOCK_SIZE << " bytes" << endl;
    cout << "Max Keys per Node: " << MAX_KEYS << endl;
    
    cout << "\nStruct Sizes:" << endl;
    cout << "FileHeader: " << sizeof(FileHeader) << " bytes" << endl;
    cout << "BTreeNode:  " << sizeof(BTreeNode) << " bytes" << endl;
    cout << "Free space in Node: " << BLOCK_SIZE - sizeof(BTreeNode) << " bytes" << endl;

    return 0;
}