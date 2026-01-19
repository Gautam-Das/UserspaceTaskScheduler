## Project Requirements & Goals

### Functional Requirements

* **M:N Model:** Map  user-level tasks onto  OS threads (where  is typically the number of CPU cores).
* **Cooperative Yielding:** Tasks must voluntarily give up control (yield) to allow the scheduler to pick the next task.
* **Context Switching:** The ability to save the CPU registers and stack of one task and load another.
* **Work-Stealing:** Idle worker threads should "steal" tasks from the queues of busy threads to maintain load balance.

### Optimization Goals (The "Deep" Stuff)

* **Cache Isolation:** Zero false sharing between the "Hot" data of different worker threads.
* **Memory Density:** Task metadata must be small enough to fit hundreds of tasks into a single L1/L2 cache line.
* **Minimal Synchronization:** Use relaxed memory ordering wherever possible, reserving heavy `seq_cst` operations for the actual theft of tasks.

---

## Stage 1: The Foundation (Context & Stack)

Before you can schedule, you must be able to "pause." You need to manage the execution state of a function.

* **Task Control Block (TCB):** Create the smallest possible struct to represent a task.
* **Context Switching:** Use `getcontext`/`setcontext` (portable) or write a tiny assembly snippet (high performance) to swap the Stack Pointer () and Instruction Pointer ().
* **The Bit-Field Challenge:** Pack the task's state (Ready, Running, Waiting, Done), priority (0–15), and a "small-stack" flag into a single `uint32_t` or `uint64_t`.

---

## Stage 2: The Infrastructure (Local Queues)

Each OS thread (Worker) needs its own playground to avoid global lock contention.

* **The Run Queue:** Implement a fixed-size circular buffer for each worker.
* **Alignment:** Use `alignas(64)` on the head and tail pointers of these queues. If Thread A is pushing to the tail and Thread B is stealing from the head, they **must** be on different cache lines, or you’ll face a 50x performance penalty.
* **Memory Ordering:** Use `memory_order_release` when a task is made "Ready" and `memory_order_acquire` when a worker picks it up.

---

## Stage 3: The Intelligence (Work-Stealing)

This is where the atomics get complex. You will implement the **Chase-Lev Deque**.

* **The Logic:** 1.  Worker  pulls from the **bottom** of its own queue (LIFO - good for cache locality).
2.  If empty, Worker  looks at Worker ’s queue and tries to `compare_exchange` (CAS) a task from the **top** (FIFO).
* **Why Top/Bottom?** Stealing from the opposite end minimizes the chance of the owner and the thief colliding on the same data index.

---

## Stage 4: Extreme Compaction (Pointer Tagging)

Once the scheduler works, you optimize for "Memory Pressure."

* **Tagged Pointers:** Instead of a `struct` with a `Task*` and a `Status` enum, pack them.
* **The Math:** Since memory addresses are 8-byte aligned, the bottom 3 bits of any pointer are always zero. You can use these 3 bits to store the task status.
* `000` = Ready
* `001` = Running
* `010` = Blocked


* **Atomic Update:** You can now change a task's status and its pointer in one single 64-bit atomic instruction.

---
