#include <iostream>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <fstream>

using namespace std;

// Define a block of memory
struct MemoryBlock {
    int startAddress;
    int size;
    int referenceCount;
};

// Function to allocate memory
int allocateMemory(list<MemoryBlock>& freeList, list<MemoryBlock>& usedList, int size) {
    // Check if there is a block of memory large enough
    for (auto it = freeList.begin(); it != freeList.end(); ++it) {
        if (it->size >= size) {
            // Allocate memory from the free block
            MemoryBlock allocatedBlock = { it->startAddress, size, 1 };
            
            // Update free block (split if necessary)
            if (it->size > size) {
                it->startAddress += size;
                it->size -= size;
            } else {
                freeList.erase(it);
            }

            // Add the allocated block to the used list
            usedList.push_back(allocatedBlock);

            return allocatedBlock.startAddress;
        }
    }

    // Unable to allocate memory
    return -1;
}

// Function to deallocate memory
void deallocateMemory(list<MemoryBlock>& freeList, list<MemoryBlock>& usedList, int startAddress) {
    // Find the block in the used list
    auto it = find_if(usedList.begin(), usedList.end(), [startAddress](const MemoryBlock& block) {
        return block.startAddress == startAddress;
    });

    if (it != usedList.end()) {
        // Decrease the reference count
        it->referenceCount--;

        // If reference count is zero, deallocate the memory
        if (it->referenceCount == 0) {
            // Add the block back to the free list
            freeList.push_back(*it);

            // Remove the block from the used list
            usedList.erase(it);
        }
    }
}

// Function to compact memory
void compactMemory(list<MemoryBlock>& freeList, list<MemoryBlock>& usedList) {
    // Merge adjacent free blocks
    freeList.sort([](const MemoryBlock& a, const MemoryBlock& b) {
        return a.startAddress < b.startAddress;
    });

    for (auto it = freeList.begin(); it != prev(freeList.end()); ++it) {
        auto nextIt = next(it);
        if (it->startAddress + it->size == nextIt->startAddress) {
            // Merge adjacent free blocks
            it->size += nextIt->size;
            freeList.erase(nextIt);
        }
    }

    // Update start addresses of used blocks
    int currentAddress = 0;
    for (auto& block : usedList) {
        block.startAddress = currentAddress;
        currentAddress += block.size;
    }
}

// Function to print memory contents
void printMemory(list<MemoryBlock>& freeList, list<MemoryBlock>& usedList) {
    cout << "Free Memory Blocks:" << endl;
    for (const auto& block : freeList) {
        cout << "Start: " << block.startAddress << ", Size: " << block.size << endl;
    }

    cout << "Used Memory Blocks:" << endl;
    for (const auto& block : usedList) {
        cout << "Start: " << block.startAddress << ", Size: " << block.size << ", RefCount: " << block.referenceCount << endl;
    }
}

int main() {
    list<MemoryBlock> freeList;
    list<MemoryBlock> usedList;

    // Assume initial memory is 64 MB
    MemoryBlock initialBlock = { 0, 64 * 1024 * 1024, 0 };
    freeList.push_back(initialBlock);

    // Read transactions from input file
    ifstream inputFile("transactions.txt");
    if (!inputFile.is_open()) {
        cerr << "Error opening input file." << endl;
        return 1;
    }

    string transaction;
    while (inputFile >> transaction) {
        if (transaction == "allocate") {
            int size;
            inputFile >> size;

            int startAddress = allocateMemory(freeList, usedList, size);
            if (startAddress == -1) {
                cout << "Error: Unable to allocate memory of size " << size << endl;
            } else {
                cout << "Allocated " << size << " bytes at address " << startAddress << endl;
            }
        } else if (transaction == "deallocate") {
            int startAddress;
            inputFile >> startAddress;

            deallocateMemory(freeList, usedList, startAddress);
            cout << "Deallocated memory at address " << startAddress << endl;
        } else if (transaction == "compact") {
            compactMemory(freeList, usedList);
            cout << "Memory compacted." << endl;
        }
    }

    // Print final memory contents
    printMemory(freeList, usedList);

    // Close input file
    inputFile.close();

    return 0;
}
