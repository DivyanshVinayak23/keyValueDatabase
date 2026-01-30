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
            // 1. Fetch the node from disk
            pager.readNode(currentPageId, node);

            // 2. Search inside the node (Linear Search)
            int i = 0;
            while (i < node.keyCount) {
                // Case A: Exact Match found!
                if (key == node.keys[i]) {
                    return node.values[i];
                }
                
                // Case B: Key is smaller than current separator?
                // This is your "Left Child" case! 
                // We found the correct door. Stop searching this node.
                if (key < node.keys[i]) {
                    break; 
                }
                
                // Continue to next key
                i++;
            }

            // At this point, 'i' is the correct child index.
            // If i == 0, go to Left Child.
            // If i == keyCount, go to Far Right Child.

            // 3. If we are at a leaf and didn't find it -> It doesn't exist.
            if (node.isLeaf) {
                return -1; 
            }

            // 4. Go deeper (Update currentPageId to the child)
            currentPageId = node.childrenOffsets[i]; 
        }

        return -1;
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