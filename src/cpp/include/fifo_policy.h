#ifndef FIFO_POLICY_H
#define FIFO_POLICY_H

#include <policy.h>
#include <queue>

class FIFOPolicy : public Policy {
    public:
        std::queue<int> fifo_q; // who to evict
        std::vector<int> findVictims();
        void insert(int pid);
        ~FIFOPolicy();
    private:
        void remove(int pid); // called by findVictim
};

#endif