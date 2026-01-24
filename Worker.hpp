#ifndef WORKER
#define WORKER

#include "TCB.hpp"
#include "task_queue.hpp"
#include <array>
#include <atomic>
#include <iostream>

constexpr size_t N_TASKS = 16;

extern "C" {
void swap_context_stack(TCB *cur_c, TCB *new_c);
}

class Worker {
private:
    std::atomic<bool> is_active;
    task_queue<TCB, N_TASKS> queue;
    TCB worker_context;

    void handle_tcb_delete() {
        delete[] current_tcb.stack; // Free stack
    }

public:
    TCB current_tcb;

    bool add_task(TCB task) {
        return queue.push(std::move(task));
    }
    void run() {
        while (queue.try_pop(current_tcb)) {
            if (current_tcb.get_state() == TCB::State::DONE)
                continue;
            auto sp = current_tcb.rsp;
            swap_context_stack(&worker_context, &current_tcb);
            if (current_tcb.get_state() == TCB::State::DONE) {
                handle_tcb_delete();
                continue;
            }
            add_task(current_tcb);
        }
    }
    void yield(TCB& tcb) {
        tcb.set_state(TCB::State::WAITING);
        swap_context_stack(&tcb, &worker_context);
    }

    void finish(TCB& tcb) {
        tcb.set_state(TCB::State::DONE);
        swap_context_stack(&tcb, &worker_context);
    }
};

extern thread_local Worker *local_worker;

#endif