#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "TCB.hpp"
#include "Worker.hpp"
#include "task_queue.hpp"
#include <bit>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

template <size_t n_workers>
class Scheduler {
    using queue = task_queue<TCB *, N_TASKS>;
    static_assert(std::popcount(n_workers) == 1, "n_workers must be power of 2");

private:
    Worker workers[n_workers];
    std::vector<std::thread> worker_threads;
    queue worker_queues[n_workers];
    size_t cur_worker{0};

    auto add_dummy_registers(char *sp, size_t n_regs) {
        for (auto i{0}; i < n_regs; ++i) {
            sp -= 8;
            *reinterpret_cast<uint64_t *>(sp) = 0;
        }
        return sp;
    }

    auto setup_stack(char *data, size_t size, uint64_t rip) {
        // stack pointer, pointer at the end of the stack block
        auto sp = static_cast<char *>(data + size);

        // align stack to 16 byte boundary
        sp = reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(sp) & -16L);

        // allow 128 bytes for red zone
        sp -= 128;

        // align stack because 7 things will be pushed that messes up the alignment
        sp -= 8;

        // push the rip to the top of the stack
        sp -= 8;
        *reinterpret_cast<uint64_t *>(sp) = rip;

        // push dummy values for 6 registers:
        // rbx, rbp, r12,r13,r14,r15
        sp = add_dummy_registers(sp, 6);

        return sp;
    }

    auto get_next_worker() {
        return cur_worker++ & (n_workers - 1);
    }

public:
    Scheduler() {
        worker_threads.resize(n_workers);
        for (auto i{0}; i < n_workers; ++i) {
            workers[i].set_queue_ptr(&(worker_queues[i]));
            worker_threads[i] = std::thread(&Worker::run, &workers[i]);
        }
        // std::cout << "Threads created successfully" << std::endl;
    }

    void run_task(void (*function)()) {
        auto& worker = workers[get_next_worker()];

        // Allocate TCB and Stack on the heap
        TCB *task_tcb = new TCB();
        char *data = new char[16384];

        auto sp = setup_stack(data, 16384, reinterpret_cast<uint64_t>(function));

        task_tcb->rsp = reinterpret_cast<void *>(sp);
        task_tcb->stack = data;
        task_tcb->set_all(TCB::State::READY, 1, false, 0);

        // Push the pointer to the worker
        if (!worker.add_task(task_tcb)) {
            // Safety: if queue is full, cleanup immediately to avoid leaks
            delete[] data;
            delete task_tcb;
        }
    }

    ~Scheduler() {
        std::cout << "destroying " << std::endl;
        // signal workers to stop here
        for (size_t i = 0; i < n_workers; ++i) {
            workers[i].is_active.store(false, std::memory_order_relaxed);
        }
        for (auto& t : worker_threads) {
            if (t.joinable())
                t.join();
        }
    }
};

#endif
