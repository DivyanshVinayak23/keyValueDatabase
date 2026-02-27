# Key-Value Storage Engine (C++)

A disk-backed key-value storage engine implemented in C++ with a B-Tree index and a custom pager subsystem. The system is designed for predictable O(log n) operations, efficient disk I/O, and scalable datasets that exceed available memory.

Repository:  
https://github.com/DivyanshVinayak23/keyValueDatabase

---

## Overview

This project implements a persistent key-value database with:

- **B-Tree indexing** for ordered storage and logarithmic search
- **Fixed-size page management** for disk-backed persistence
- **Custom pager layer** for memory–disk coordination
- **Self-balancing tree structure** optimized for block storage

The design mirrors foundational database engine principles: page-oriented storage, hierarchical indexing, and separation of logical structure from physical persistence.

---

## Architecture

The system is divided into three primary components:

### 1. B-Tree (`BTree.cpp`)

The B-Tree is the logical indexing layer responsible for:

- Key insertion
- Key lookup
- Deletion
- Structural rebalancing (split/merge operations)
- Ordered traversal and range queries

#### Characteristics

- Search: O(log n)
- Insert: O(log n)
- Delete: O(log n)
- Range Query: O(log n + k)

The tree maintains strict ordering and ensures height remains logarithmic with respect to the number of keys.

Node layout is structured to align with page boundaries to minimize disk I/O amplification.

---

### 2. Node Structure (`nodeStructure.cpp`, `nodeStructure`)

Each node contains:

- Ordered key array
- Associated value references
- Child page identifiers (for internal nodes)
- Metadata (e.g., key count, leaf flag)

The structure is designed to:

- Maximize key density per page
- Minimize pointer overhead
- Maintain deterministic memory layout
- Support serialization for disk persistence

Nodes map directly to pages managed by the pager.

---

### 3. Pager (`pager.cpp`)

The pager abstracts physical storage. It is responsible for:

- Page allocation
- Page caching in memory
- Dirty page tracking
- Lazy flushing to disk
- Page retrieval by ID

The database file is divided into fixed-size pages (e.g., 4KB). Pages are loaded into memory on demand and written back only when modified.

This design enables:

- Efficient handling of datasets larger than RAM
- Controlled memory usage
- Reduced disk writes via batching

---

## Data Flow

### Insert Operation

1. Traverse B-Tree from root to target leaf
2. Load required pages through pager
3. Insert key in sorted position
4. Split node if capacity exceeded
5. Mark modified pages as dirty
6. Flush to disk (explicit or deferred)

### Search Operation

1. Traverse tree from root
2. Pager loads pages on demand
3. Return associated value if key exists

Disk reads are limited to tree height in worst case.

---

## Why B-Tree Instead of Hash Index?
Ordered traversal -> Yes, Range queries - > Efficient, Disk locality -> High, Predictable height -> Yes.
B-Trees are optimized for block storage and minimize random disk access. This makes them suitable for persistent storage engines.

## Build Instructions

### Requirements

- C++11 or later
- g++, clang, or MSVC
- Standard build tooling

### Compile

```bash
git clone https://github.com/DivyanshVinayak23/keyValueDatabase.git
cd keyValueDatabase
g++ -std=c++11 -O2 -o kvdb BTree.cpp nodeStructure.cpp pager.cpp test.cpp
./kvdb

---




