#ifndef WORKER
#define WORKER

#include "TCB.hpp"
#include <array>
#include <atomic>

constexpr int N_TASKS = 16;

class Worker {
private:
    std::atomic<bool> is_active;
    std::array<TCB, N_TASKS> tasks;
    std::atomic<TCB *> head;
    std::atomic<TCB *> tail;

public:
    add_task(TCB task) {
    }
};

#endif