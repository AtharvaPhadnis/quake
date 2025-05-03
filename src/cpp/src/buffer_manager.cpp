#include <buffer_manager.h>
#include <fifo_policy.h>

// for destructor, flush all pages


BufferManager::BufferManager() {
    bufSize = 10;
    policy = FIFOPolicy();
    curSize = 0;
}

~BufferManager() {
    std::cout << "Flush contents? " << std::endl;
}

void BufferManager::put(int pid, shared_ptr<FileIndexPartition> fip) {
    if(curSize == bufSize) { // Buffer is full
        auto victims = policy.findVictims();
        for(auto victim : victims) {
            u.erase(victim);
            shared_ptr<FileIndexPartition> wb_fip = nullptr;
            auto it = partition_manager_->partitions_->partitions_.find(pid);
            if (it == partition_manager_->partitions_->partitions_.end()) {
                throw std::runtime_error("pid does not exist");
            }
            wb_fip = std::dynamic_pointer_cast<FileIndexPartition>(partition_manager_->partitions_->partitions_[pi]);
            if(wb_fip) wb_fip->save();
        }
    }
    u.insert(pid);
    fip->load();
}

void BufferManager::flush(int pid, shared_ptr<FileIndexPartition> fip) {
    std::cout << "[BufferManager] flush implementation goes here" << std::endl;
}

void BufferManager::evict() {
    std::cout << "[BufferManager] evict implementation goes here" << std::endl;
}