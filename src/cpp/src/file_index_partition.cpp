#include "file_index_partition.h"
#include <fstream>

// Parameterized constructor
FileIndexPartition::FileIndexPartition(int64_t num_vectors,
                                       uint8_t* codes,
                                       idx_t* ids,
                                       int64_t code_size) {
    // Implementation here
}

// Move constructor
FileIndexPartition::FileIndexPartition(FileIndexPartition&& other) noexcept {
    // Implementation here
}

// Move assignment operator
FileIndexPartition& FileIndexPartition::operator=(FileIndexPartition&& other) noexcept {
    // Implementation here
    return *this;
}

// Destructor
FileIndexPartition::~FileIndexPartition() {
    // munmap stuff needs to happen here?
    // std::cout << "FIP: Destructor " << codes_file_path_ << std::endl;
    munmap(codes_, buffer_size_*static_cast<size_t>(code_size_));
    munmap(ids_, buffer_size_*sizeof(idx_t));
    codes_ = nullptr;
    ids_ = nullptr;
}

// void FileIndexPartition::ensure_capacity(int64_t required_size) {
//     std::cout << "Ensure capacity called with required_size: " <<
//     required_size << ", current size is " << buffer_size_ << std::endl;
//     // Now id required_size > current_map_size, some remapping needs to be done
//     // Also potentially different behavior if mmap pointer = nullptr
//     if(required_size > buffer_size_) {
//         // Need to remap for a bigger size
//         // In memory version simply doubles allocated region size when we
//         // hit the limit, disk based needs a different heuristic?
//         if(debug_) {
//             std::cout << "Required size is larger than current map size, will call remap()" << std::endl;
//         }
//         remap_files(required_size);
//     }

// }

void FileIndexPartition::remap_files(int64_t required) {
    if(debug_) {
        // std::cout << "remap_files() called" << std::endl;
    }

    if (codes_) {
        if(debug_) {
            // std::cout << "Unmapping codes" << std::endl;
        }
        munmap(codes_, buffer_size_*static_cast<size_t>(code_size_));
        codes_ = nullptr;
    }

    if (ids_) {
        munmap(ids_, buffer_size_*sizeof(idx_t));
        ids_ = nullptr;
    }

    int64_t new_capacity;
    if (required > buffer_size_) {
        new_capacity = std::max<int64_t>(1024, buffer_size_);
        while (new_capacity < required) {
            new_capacity *= 2;
        }
    } else {
        return;
    }
    // This will need to change with the buffer approach, we dont want to close the file descriptors of partitions
    // in the cache, they might be needed
    int codes_fd = open(codes_file_path_.c_str(), O_RDWR | O_CREAT, 0644);
    
    if (ftruncate(codes_fd, new_capacity*static_cast<size_t>(code_size_)) == -1) 
        throw std::runtime_error("Codes file resize failed");
    
    uint8_t* new_codes_ = (uint8_t*)mmap(nullptr, new_capacity*static_cast<size_t>(code_size_), PROT_READ | PROT_WRITE, MAP_SHARED, codes_fd, 0);
    if (new_codes_ == MAP_FAILED) 
        throw std::runtime_error("Codes remap failed");

    close(codes_fd);

    int ids_fd = open(ids_file_path_.c_str(), O_RDWR | O_CREAT, 0644);
    
    if (ftruncate(ids_fd, new_capacity*sizeof(idx_t)) == -1) 
        throw std::runtime_error("Ids file resize failed");

    idx_t* new_ids_ = (idx_t*)mmap(nullptr, new_capacity*sizeof(idx_t), PROT_READ | PROT_WRITE, MAP_SHARED, ids_fd, 0);
    if (new_ids_ == MAP_FAILED) 
        throw std::runtime_error("Ids remap failed");

    close(ids_fd);

    codes_ = new_codes_;
    ids_ = new_ids_;
    buffer_size_ = new_capacity;
}

void FileIndexPartition::append(int64_t n_entry, const idx_t* new_ids, const uint8_t* new_codes) {
    // std::cout << "FileIndexPartition::append implementation goes here" << std::endl;

    // Possible implementation
    /*
        - If filename is null, call something like do_mmap()
        - do_mmap() should
            1. Create a file in the file system
            2. Mmap that file into memory and return a pointer to it? (Maybe we can have a member variable
            in FileIndexPartition hold the pointer as well)

        - Else, ensure there is enough capacity in the already mmap region to store the new vectors
        ensure_capacity() maybe
        - Cast codes to bytes
        - Write into mmap region
    */

    if(n_entry <= 0) return;

    // ensure_capacity(num_vectors_ + n_entry);

    // std::cout << "Currently has " << num_vectors_ << " vectors with current_map_size: " << buffer_size_<< std::endl;
    // ensure_capacity(num_vectors_ + n_entry);
    // Debatable if this is needed, can we simply expand a mmaped region as needed?
    // If we do end up needing this, need to decide if it allocates memory or not
    // If it does, the mmap logic below might not be needed

    // int codes_fd = open(codes_file_path_.c_str(), O_RDWR | O_CREAT, 0644);
    // if (codes_fd == -1) throw std::runtime_error("Failed to open file");

    // int ids_fd = open(ids_file_path_.c_str(), O_RDWR | O_CREAT, 0644);
    // if (ids_fd == -1) throw std::runtime_error("Failed to open file");

    remap_files(num_vectors_ + n_entry);
    const size_t code_bytes = static_cast<size_t>(code_size_);

    std::memcpy(codes_ + num_vectors_ * code_bytes, new_codes, n_entry * code_bytes);
    std::memcpy(ids_ + num_vectors_, new_ids, n_entry * sizeof(idx_t));
    num_vectors_ += n_entry;


    // With a buffer, we dont want to immediately unmap, we want to keep this around till all appends into this 
    // partition are done
    munmap(codes_, buffer_size_);
    munmap(ids_, buffer_size_);
    buffer_size_ = 0;
    codes_ = nullptr;
    ids_ = nullptr;
} 

void FileIndexPartition::update(int64_t offset, int64_t n_entry, const idx_t* new_ids, const uint8_t* new_codes) {
    // Implementation here
}

void FileIndexPartition::remove(int64_t index) {
    // Implementation here
    // if (index < 0 || index >= num_vectors_) {
    //     throw std::runtime_error("Index out of range in remove");
    // }
    // if (index == num_vectors_ - 1) {
    //     num_vectors_--;
    //     return;
    // }

    // int64_t last_idx = num_vectors_ - 1;
    // const size_t code_bytes = static_cast<size_t>(code_size_);

    // if (is_in_memory) {
    //     std::lock_guard<std::mutex> lock(ref_mutex);
    //     std::cout << "[File_index_partition] remove : Removing index " << index << " of partition ID (in memory) " << file_path_ << std::endl; 
    //     std::memcpy(codes_ + index * code_bytes, codes_ + last_idx * code_bytes, code_bytes);
    //     ids_[index] = ids_[last_idx];
    //     is_dirty = true;
    //     num_vectors_--;
    // }
    // else {
    //     std::cout << "[File_index_partition] remove : Removing index " << index << " of file path " << file_path_ << std::endl; 
    //     std::fstream file(file_path_, std::ios::in | std::ios::out | std::ios::binary);
    //     if (!file) {
    //         throw std::runtime_error("Failed to open file for remove: " + file_path_);
    //     }

    //     std::vector<uint8_t> last_code(code_bytes);
    //     idx_t last_id;
        
    //     // seek the last entry
    //     file.seekg(last_idx * (code_bytes + sizeof(idx_t)), std::ios::beg);
    //     file.read(reinterpret_cast<char*>(last_code.data()), code_bytes);
    //     file.read(reinterpret_cast<char*>(&last_id), sizeof(idx_t));
    
    //     // seek the deleted entry and replace with the last entry
    //     file.seekp(index * (code_bytes + sizeof(idx_t)), std::ios::beg);
    //     file.write(reinterpret_cast<const char*>(last_code.data()), code_bytes);
    //     file.write(reinterpret_cast<const char*>(&last_id), sizeof(idx_t));
    
    //     file.close();
    //     num_vectors_--;
    // }

}

void FileIndexPartition::resize(int64_t new_capacity) {
    // Implementation here
}

void FileIndexPartition::clear() {
    // Implementation here
}

int64_t FileIndexPartition::find_id(idx_t id) const {
    // Implementation here
    return -1; // Placeholder return
}

void FileIndexPartition::reallocate(int64_t new_capacity) {
    // Implementation here
}

// for testing
void FileIndexPartition::load() {
    // std::cout << "[FileIndexPartition] load" << std::endl;
    // std::ifstream in(file_path_, std::ios::binary);
    // if (!in) {
    //     throw std::runtime_error("Unable to open file for reading: " + file_path_);
    // }
    std::lock_guard<std::mutex> lock(ref_mutex);
    
    ref_cnt ++;
    if (is_in_memory) return;

    int codes_fd = open(codes_file_path_.c_str(), O_RDWR, 0644);
    if (codes_fd == -1) throw std::runtime_error("Failed to open file");

    int ids_fd = open(ids_file_path_.c_str(), O_RDWR, 0644);
    if (ids_fd == -1) throw std::runtime_error("Failed to open file");

    // in.close();
    is_in_memory = true;
    // mmap the file into memory
    int64_t file_size = num_vectors_ * static_cast<size_t>(code_size_);

    codes_ = (uint8_t*)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, codes_fd, 0);
    if (codes_ == MAP_FAILED) 
        throw std::runtime_error("Failed to mmap file");

    
    ids_ = (idx_t*)mmap(nullptr, num_vectors_*sizeof(idx_t), PROT_READ | PROT_WRITE, MAP_SHARED, ids_fd, 0);
    if (ids_ == MAP_FAILED) 
        throw std::runtime_error("Ids remap failed");

    close(codes_fd);
    close(ids_fd);
}

// decrement the reference bit, if its zero called the buffer manager to flush the file
// the buffer manager should loop over the buffer pool and evict all buffers that belongs to the file
void FileIndexPartition::save() {
    //std::cout << "[FileIndexPartition::save]" << std::endl;
    std::lock_guard<std::mutex> lock(ref_mutex);
    ref_cnt --;
    if (ref_cnt == 0) {
        buffer_size_ = 0;
        //free_memory(); // for now, we don't have a dedicated buffer pool so just free the memory
        is_in_memory = false;

        if(is_dirty) {
            munmap(codes_, buffer_size_*static_cast<size_t>(code_size_));
            munmap(ids_, buffer_size_*sizeof(idx_t));
            codes_ = nullptr;
            ids_ = nullptr;
        }
    }
    else {
        // Someone else is still pointing to this partition
        return;
    }

    // std::cout << "[FileIndexPartition::save] about to return" << std::endl;
}

void FileIndexPartition::set_codes_file_path(std::string codes_file_path) {
    codes_file_path_ = codes_file_path;
}

void FileIndexPartition::set_ids_file_path(std::string ids_file_path) {
    ids_file_path_ = ids_file_path;
}

// void FileIndexPartition::free_memory() {
//     if (codes_ == nullptr && ids_ == nullptr) {
//         return;
//     }
//     std::free(codes_);
//     std::free(ids_);
    
//     codes_ = nullptr;
//     ids_ = nullptr;
// }

#ifdef QUAKE_USE_NUMA
void FileIndexPartition::set_numa_node(int new_numa_node) {
    // Implementation here
}
#endif
