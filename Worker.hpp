#ifndef WORKER
#define WORKER

#include "TCB.hpp"
#include "task_queue.hpp"
#include <atomic>
#include <thread>

constexpr size_t N_TASKS = 16;

extern thread_local class Worker *local_worker;

extern "C" {
void swap_context_stack(TCB *cur_c, TCB *new_c);
}

class Worker {
private:
    task_queue<TCB *, N_TASKS> queue; // Now stores pointers
    TCB worker_context;

    void handle_tcb_delete(TCB *tcb) {
        if (tcb) {
            delete[] tcb->stack; // Free the heap-allocated stack
            delete tcb;          // Free the heap-allocated TCB
        }
    }

public:
    TCB *current_tcb = nullptr; // Pointer to active task
    std::atomic<bool> is_active{true};

    bool add_task(TCB *task) {
        return queue.push(task);
    }

    void run() {
        local_worker = this;
        while (is_active.load(std::memory_order_relaxed)) {
            TCB *next_task = nullptr;
            if (!queue.try_pop(next_task)) {
                std::this_thread::yield();
                continue;
            }
            current_tcb = next_task;

            // Stability check: if someone pushed a finished task
            if (current_tcb->get_state() == TCB::State::DONE) {
                handle_tcb_delete(current_tcb);
                continue;
            }

            // Perform context switch
            swap_context_stack(&worker_context, current_tcb);

            // Post-execution check
            if (current_tcb->get_state() == TCB::State::DONE) {
                handle_tcb_delete(current_tcb);
            } else {
                add_task(current_tcb);
            }
        }
    }

    void yield(TCB *tcb) {
        tcb->set_state(TCB::State::WAITING);
        swap_context_stack(tcb, &worker_context);
    }

    void finish(TCB *tcb) {
        tcb->set_state(TCB::State::DONE);
        swap_context_stack(tcb, &worker_context);
    }
};

#endif