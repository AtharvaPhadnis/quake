#include <buffer_manager.h>

class FIFO::Policy {
    public:
        std::queue<int> q; // who to evict
        std::vector findVictims();
        void insert(int pid);
    private:
        void remove(int pid); // called by findVictim
}