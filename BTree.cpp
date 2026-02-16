#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>


using namespace std;

class BTree{
    Pager pager;
    int64_t rootPageId;


    public:

    BTree(string fileName) : pager(fileName){
        rootPageId = -1; //invalid page id

        const uint32_t MAGIC_NUMBER = 0x54755475;
        if(pager.isFileEmpty()){
            // CASE 1: New Empty File
            cout << "New empty file detected. Initializing..." << endl;
            initializeNewFile();
        }
        else {
            // CASE 2: Existing File (Check Magic Number)
            FileHeader header;
            pager.readHeader(header);
            if(header.magicNumber != MAGIC_NUMBER){
                // CASE 2.1: Corrupted File
                cout << "Corrupted file detected. Initializing..." << endl;
                exit(EXIT_FAILURE);
            }
            else {
                // CASE 2.2: Valid File
                rootPageId = header.rootOffset;
            }

        }
    }

    int64_t get(int64_t key) {
        int64_t currentPageId = rootPageId;
        BTreeNode node;

        while (currentPageId != -1) {
            pager.readNode(currentPageId, node);

            // 2. Search inside the node (Linear Search)
            int i = 0;
            while (i < node.keyCount) {
                // Case A: Exact Match found!
                if (key == node.keys[i]) {
                    return node.values[i];
                }
                
                // Case B: Key is smaller than current separator?
                if (key < node.keys[i]) {
                    break; 
                }
                
                // Continue to next key
                i++;
            }


            // 3. If we are at a leaf and didn't find it -> It doesn't exist.
            if (node.isLeaf) {
                return -1; 
            }

            // 4. Go deeper (Update currentPageId to the child)
            currentPageId = node.childrenOffsets[i]; 
        }

        return -1;
    }

    void set(int64_t key, int64_t value) {
        int64_t currentPageId = rootPageId;
        BTreeNode node;

        while (true) {
            pager.readNode(currentPageId, node);

            int i = 0;

            while (i < node.keyCount) {
                if (key == node.keys[i]) {
                    node.values[i] = value;
                    pager.writeNode(currentPageId, node);
                    return;
                }
                if (key < node.keys[i]) {
                    break; 
                }
                i++;
            }

            if (node.isLeaf) {
                break;
            }

            // Not a leaf? Keep going deeper.
            currentPageId = node.childrenOffsets[i];
        }


        if (node.keyCount >= MAX_KEYS) {
            cout << "Error: Node is full. Splitting not implemented yet." << endl;
            return;
        }

        // Find the specific index where the new key goes
        int insertIndex = 0;
        while (insertIndex < node.keyCount && node.keys[insertIndex] < key) {
            insertIndex++;
        }

        for (int j = node.keyCount; j > insertIndex; j--) {
            node.keys[j] = node.keys[j - 1];
            node.values[j] = node.values[j - 1];
        }

        node.keys[insertIndex] = key;
        node.values[insertIndex] = value;
        node.keyCount++; 

        pager.writeNode(currentPageId, node);
    }
  

// It returns the key that needs to be pushed up to the parent.
int64_t splitLeafData(BTreeNode& fullNode, BTreeNode& newNode) {
    int midIndex = MAX_KEYS / 2; // This is 84
    

    int64_t promotedKey = fullNode.keys[midIndex];
    int64_t promotedValue = fullNode.values[midIndex];
    
    int newIndex = 0;
    for (int index = midIndex + 1; index < MAX_KEYS; index++) {
        newNode.keys[newIndex] = fullNode.keys[index];
        newNode.values[newIndex] = fullNode.values[index];
        newIndex++;
    }

    newNode.keyCount = MAX_KEYS - midIndex - 1;
    fullNode.keyCount = midIndex;
    
    newNode.isLeaf = true;
    
    return promotedKey;
}

    private:
        void initializeNewFile(){
            //We will create a new file with a single node.
            FileHeader header;
            header.magicNumber = MAGIC_NUMBER;
            header.rootOffset = 1; //0 is reserved for the header.
            header.freeListHead = -1;
            
            
            BTreeNode rootNode;
            rootNode.isLeaf = true;
            rootNode.keyCount = 0;
            rootNode.parentOffset = -1;

            pager.writeNode(1, rootNode);
            rootPageId = 1;

            pager.writeHeader(header);
        }
};