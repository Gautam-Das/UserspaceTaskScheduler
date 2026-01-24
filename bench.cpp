#include "Scheduler.hpp"
#include <atomic>
#include <chrono>
#include <iostream>
#include <vector>

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

// We'll use a global atomic counter to track when tasks are actually done
std::atomic<int> tasks_remaining{0};

void benchmark_finish() {
    tasks_remaining--; // Decrement the count
    if (local_worker) {
        local_worker->finish(local_worker->current_tcb);
    }
}

// Slightly modified tasks to use the atomic counter
void prime_task_bench() {
    int count = 0;
    int num = 2;
    while (count < 5000) { // Increased from 50 to 5000
        bool is_prime = true;
        for (int i = 2; i <= num / 2; ++i) {
            if (num % i == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            count++;
            if (count % 500 == 0)
                yield(); // Yield less often
        }
        num++;
    }
    benchmark_finish();
}

void fib_task_bench() {
    long long a = 0, b = 1;
    for (int i = 0; i < 5000; ++i) {
        long long next = a + b;
        a = b;
        b = next;
        if (i % 500 == 0)
            yield();
    }
    benchmark_finish();
}

int main() {
    const int num_repeats = 16;
    const int total_tasks = num_repeats * 4;

    // --- SEQUENTIAL TEST ---
    std::cout << "Starting Sequential Baseline..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();

    // Reset counter for sequential
    tasks_remaining = total_tasks;
    for (int i = 0; i < num_repeats; ++i) {
        // We call them directly (no scheduler, no threads)
        // We override 'yield' and 'finish' conceptually by just letting them run
        // For this test, we just run the logic blocks
        prime_task_bench();
        prime_task_bench();
        fib_task_bench();
        fib_task_bench();
    }

    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_seq = end_seq - start_seq;
    std::cout << "Sequential Time: " << diff_seq.count() << "s\n"
              << std::endl;

    // --- SCHEDULER TEST ---
    std::cout << "Starting Scheduler (16 Workers)..." << std::endl;
    tasks_remaining = total_tasks;
    Scheduler<16> scheduler;

    // We create the scheduler inside the timed block to include thread startup
    auto start_sched = std::chrono::high_resolution_clock::now();

    {
        for (int i = 0; i < num_repeats; ++i) {
            scheduler.run_task(prime_task_bench);
            scheduler.run_task(prime_task_bench);
            scheduler.run_task(fib_task_bench);
            scheduler.run_task(fib_task_bench);
        }

        // Wait loop: Poll the atomic counter until it hits zero
        while (tasks_remaining.load() > 0) {
            std::this_thread::yield();
        }
        // Scheduler destructor called here (joins threads)
    }

    auto end_sched = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_sched = end_sched - start_sched;

    std::cout << "Scheduler Time:  " << diff_sched.count() << "s\n"
              << std::endl;
    std::cout << "Speedup: " << diff_seq.count() / diff_sched.count() << "x" << std::endl;

    return 0;
}