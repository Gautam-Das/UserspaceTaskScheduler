#include "Scheduler.hpp"
#include "TCB.hpp"
#include "Worker.hpp"
#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <iostream>
#include <thread>

extern "C" {
void swap_context_stack(TCB *cur_c, TCB *new_c);
}

TCB main_context;
Worker worker;

thread_local Worker *local_worker = nullptr;

void yield() {
    if (local_worker) {
        local_worker->yield(local_worker->current_tcb);
    }
}

void finish() {
    if (local_worker) {
        local_worker->finish(local_worker->current_tcb);
    }
}

void foo() {
    std::cout << "hello world from foo\n"
              << std::endl;
    yield();
    printf("second scary hello world from foo\n");
    // exit(0);
    finish();
}

void bar() {
    std::cout << "hello world from bar\n"
              << std::endl;
    yield();
    printf("second scary hello world from bar\n");
    exit(0);
}

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

int main(void) {

    char data[4096];
    auto sp = setup_stack(data, 4096, reinterpret_cast<uint64_t>(foo));

    TCB c;

    c.rsp = reinterpret_cast<void *>(sp);
    c.set_all(TCB::State::READY, 1, false, 0);

    worker.add_task(c);
    auto t1 = std::thread(&Worker::run, &worker);
    t1.detach();
    Scheduler<1> scheduler;
    scheduler.run_task(bar);

    std::cout << "main ends here" << std::endl;
    return 0;
}