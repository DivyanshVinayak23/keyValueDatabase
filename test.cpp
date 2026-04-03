#include <iostream>
#include <cstdio>
#include "BTree.h"

int main() {
    std::cout << "Starting Key-Value DB Test...\n";
    // Delete existing test db if it exists
    std::remove("test.db");
    
    BTree db("test.db");
    
    std::cout << "Inserting keys 0 to 999...\n";
    for (int i = 0; i < 1000; i++) {
        db.set(i, i * 10);
    }
    
    std::cout << "Verifying previously inserted keys...\n";
    bool success = true;
    for (int i = 0; i < 1000; i++) {
        int64_t val = db.get(i);
        if (val != i * 10) {
            std::cout << "Failed at key " << i << ". Expected " << i * 10 << " got " << val << "\n";
            success = false;
        }
    }
    
    if (success) {
        std::cout << "All tests passed successfully!\n";
    }
    
    return 0;
}
