#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include <unordered_set>
#include <queue>
#include <file_index_partition.h>

// decide who to evict
virtual class Policy {
    public:
        std::queue<int> q; // who to evict
        std::vector findVictims();
        void insert(int pid);
    private:
        void remove(int pid); // called by findVictim
}

// actually putting and removing things in buffer
class BufferManager {
    public:
        std::unordered_set<int> u; // who is in the buffer
        shared_ptr<Policy> p;
        int bufSize; // number of partitions in the memory
        int curSize;
        void put(file_index_partition* fip); // load the vectors and ids of a partition from disk
        void flush(file_index_partition* fip); // flush the vectors and ids of a partition to disk, while still keeping in memory
        
    private:
        void evict(); // evict a partition based on the eviction policy, this will be called by putBuf if the buffer is full
};