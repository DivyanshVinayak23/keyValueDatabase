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
            // Case A: The full node is the Root
            if (node.parentOffset == -1) {
                splitRoot(currentPageId, node);
                return set(key, value); 
            }
            // Case B: The full node is a normal Leaf
            else {
                BTreeNode rightNode;
                int64_t promotedValue = node.values[MAX_KEYS / 2];
                int64_t promotedKey = splitLeafData(node, rightNode);                
                FileHeader header;
                pager.readHeader(header);
                
                int64_t rightPageId = pager.getNewPageID(header.freeListHead);
                pager.writeHeader(header);
                
                rightNode.parentOffset = node.parentOffset;
                
                pager.writeNode(currentPageId, node);
                pager.writeNode(rightPageId, rightNode);
                

                insertIntoParent(node.parentOffset, promotedKey, promotedValue, rightPageId);
                return set(key, value);
            }
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

void splitRoot(int64_t oldRootPageId, BTreeNode& oldRoot) {
        BTreeNode rightNode;
        
        int64_t promotedValue = oldRoot.values[MAX_KEYS / 2]; 
        int64_t promotedKey = splitLeafData(oldRoot, rightNode);
        
        FileHeader header;
        pager.readHeader(header);
        
        int64_t rightPageId = pager.getNewPageID(header.freeListHead);
        // Hack for now: pretend we wrote rightPageId so the file grows, 
        pager.writeHeader(header);
        int64_t newRootPageId = rightPageId + 1; 

        BTreeNode newRoot;
        newRoot.isLeaf = false;
        newRoot.keyCount = 1;
        newRoot.keys[0] = promotedKey;
        newRoot.values[0] = promotedValue;
        newRoot.childrenOffsets[0] = oldRootPageId;
        newRoot.childrenOffsets[1] = rightPageId;
        newRoot.parentOffset = -1; // Root has no parent

        oldRoot.parentOffset = newRootPageId;
        rightNode.parentOffset = newRootPageId;

        pager.writeNode(oldRootPageId, oldRoot);
        pager.writeNode(rightPageId, rightNode);
        pager.writeNode(newRootPageId, newRoot);

        header.rootOffset = newRootPageId;
        pager.writeHeader(header);
        rootPageId = newRootPageId;
        
        cout << "Root split successfully. New Root is Page " << newRootPageId << endl;
    }

    void insertIntoParent(int64_t parentPageId, int64_t promotedKey, int64_t promotedValue, int64_t rightChildPageId) {
    BTreeNode parentNode;
    pager.readNode(parentPageId, parentNode);

    if (parentNode.keyCount >= MAX_KEYS) {
if (parentNode.keyCount >= MAX_KEYS) {
        BTreeNode rightNode;
        FileHeader header;
        pager.readHeader(header);
        
        int64_t newRightPageId = pager.getNewPageID(header.freeListHead);
        pager.writeHeader(header);

        int64_t grandparentValue = parentNode.values[MAX_KEYS / 2];
        int64_t grandparentKey = splitInternalNode(parentNode, rightNode, newRightPageId);

        // CASE A: The full node is the ROOT (Internal Root)
        if (parentNode.parentOffset == -1) {
            int64_t newRootPageId = newRightPageId + 1; // Hack for new Page ID until Free List is done
            
            BTreeNode newRoot;
            newRoot.isLeaf = false;
            newRoot.keyCount = 1;
            newRoot.keys[0] = grandparentKey;
            newRoot.values[0] = grandparentValue;
            newRoot.childrenOffsets[0] = parentPageId;
            newRoot.childrenOffsets[1] = newRightPageId;
            newRoot.parentOffset = -1;

            parentNode.parentOffset = newRootPageId;
            rightNode.parentOffset = newRootPageId;

            pager.writeNode(parentPageId, parentNode);
            pager.writeNode(newRightPageId, rightNode);
            pager.writeNode(newRootPageId, newRoot);

            header.rootOffset = newRootPageId;
            pager.writeHeader(header);
            rootPageId = newRootPageId;
            
            cout << "Internal Root split! Tree grew taller. New Root: " << newRootPageId << endl;
        } 
        // CASE B: Normal Internal Node
        else {
            rightNode.parentOffset = parentNode.parentOffset;
            pager.writeNode(parentPageId, parentNode);
            pager.writeNode(newRightPageId, rightNode);

            insertIntoParent(parentNode.parentOffset, grandparentKey, grandparentValue, newRightPageId);
        }
        if (promotedKey < grandparentKey) {
            return insertIntoParent(parentPageId, promotedKey, promotedValue, rightChildPageId);
        } else {
            return insertIntoParent(newRightPageId, promotedKey, promotedValue, rightChildPageId);
        }
    }
    }

    int insertIndex = 0;
    while (insertIndex < parentNode.keyCount && parentNode.keys[insertIndex] < promotedKey) {
        insertIndex++;
    }

    for(int j = parentNode.keyCount; j > insertIndex; j--){
        parentNode.keys[j] = parentNode.keys[j - 1];
        parentNode.values[j] = parentNode.values[j - 1];
    }
    

    for(int j = parentNode.keyCount + 1; j > insertIndex + 1; j--){
        parentNode.childrenOffsets[j] = parentNode.childrenOffsets[j - 1];
    }

    parentNode.keys[insertIndex] = promotedKey;
    parentNode.values[insertIndex] = promotedValue;
    
    parentNode.childrenOffsets[insertIndex + 1] = rightChildPageId;
    
    parentNode.keyCount++;
    pager.writeNode(parentPageId, parentNode);
}


int64_t splitInternalNode(BTreeNode& fullNode, BTreeNode& rightNode, int64_t rightNodePageId) {
    int midIndex = MAX_KEYS / 2; 

    int64_t promotedKey = fullNode.keys[midIndex];
    
    int newIndex = 0;
    for (int i = midIndex + 1; i < MAX_KEYS; i++) {
        rightNode.keys[newIndex] = fullNode.keys[i];
        rightNode.values[newIndex] = fullNode.values[i];
        newIndex++;
    }

    //Move Children Pointers
    int newChildIndex = 0;
    for (int i = midIndex + 1; i <= MAX_KEYS; i++) {
        rightNode.childrenOffsets[newChildIndex] = fullNode.childrenOffsets[i];
        newChildIndex++;
    }

    // Update the Parent Offsets of the moved children!

    for(int i = 0; i < newChildIndex; i++){
        int64_t childPageId = rightNode.childrenOffsets[i];
        if(childPageId != -1){
            BTreeNode childNode;
            pager.readNode(childPageId, childNode);
            childNode.parentOffset = rightNodePageId;
            pager.writeNode(childPageId, childNode);
        }
    }

    // Update Key Counts
    rightNode.keyCount = MAX_KEYS - midIndex - 1;
    fullNode.keyCount = midIndex;
    rightNode.isLeaf = false;

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