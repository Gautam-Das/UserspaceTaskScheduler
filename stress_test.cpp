#include "Scheduler.hpp"
#include "TCB.hpp"
#include "Worker.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <iostream>
#include <thread>
#include <vector>

TCB main_context;
Worker worker;

thread_local Worker *local_worker = nullptr;

void yield() {
    if (local_worker) {
        local_worker->yield(local_worker->current_tcb); // Passing the pointer
    }
}

void finish() {
    if (local_worker) {
        local_worker->finish(local_worker->current_tcb);
    }
}

// Helper to differentiate which thread/task is talking
#define TASK_LOG(name, id, msg) \
    printf("[Task %s #%d] %s\n", name, id, msg)

void heavy_math_prime(int id) {
    int count = 0;
    int num = 2;
    TASK_LOG("Prime", id, "Starting heavy computation...");

    while (count < 50) { // Find first 50 primes
        bool is_prime = true;
        for (int i = 2; i <= num / 2; ++i) {
            if (num % i == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            count++;
            // Yield every 10 primes found to let other tasks progress
            if (count % 10 == 0) {
                TASK_LOG("Prime", id, "Yielding...");
                yield();
            }
        }
        num++;
    }
    TASK_LOG("Prime", id, "Finished!");
    finish();
}

void fibonacci_task(int id) {
    long long a = 0, b = 1;
    TASK_LOG("Fib", id, "Starting...");

    for (int i = 0; i < 20; ++i) {
        long long next = a + b;
        a = b;
        b = next;

        // Simulate "work" and yield every 5 iterations
        if (i % 5 == 0) {
            TASK_LOG("Fib", id, "Yielding...");
            yield();
        }
    }
    TASK_LOG("Fib", id, "Done.");
    finish();
}

void task1() { heavy_math_prime(1); }
void task2() { heavy_math_prime(2); }
void task3() { fibonacci_task(3); }
void task4() { fibonacci_task(4); }

int main(void) {

    // 16 Workers, each on its own OS thread
    Scheduler<16> scheduler;

    printf("--- Stress Test Starting ---\n");

    // Dispatch 16 tasks across the 16 threads
    for (int i = 0; i < 16; ++i) {
        scheduler.run_task(task1);
        scheduler.run_task(task2);
        scheduler.run_task(task3);
        scheduler.run_task(task4);
    }

    printf("--- All tasks dispatched. Main thread entering wait loop. ---\n");

    // Keep main alive to watch the output
    // In a real system, you'd implement a 'wait_until_all_done()'
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printf("Main: Still waiting... (second %d)\n", i + 1);
    }

    printf("--- Stress Test Complete ---\n");
    return 0;
}