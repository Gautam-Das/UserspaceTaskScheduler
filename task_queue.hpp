#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <array>
#include <atomic>
#include <memory>
#include <mutex>

template <typename T, size_t size>
class task_queue {
    static_assert((size & (size - 1)) == 0, "size must be a power of two");

private:
    // using T rather than shared pointer because in Linked list queue, the
    // data must exist even after the node is deleted, keeping data in the node
    // can mean that its destroyed when the node is popped.
    // here there is no such risk, the only risk is that it can be overwritten
    // moving out and moving back in is probably faster due to the small size
    // no need for ownership
    std::array<T, size> tasks;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
    std::mutex push_mutex;

    static constexpr size_t mask = size - 1;

public:
    task_queue() {};
    task_queue(const task_queue& other) = delete;
    auto operator=(const task_queue& other) = delete;

    auto try_pop(T& out) {
        auto cur_head = head.load();
        auto cur_tail = tail.load();
        if (cur_head == cur_tail)
            return false;

        out = std::move(tasks[head & (size - 1)]);
        head++;
        return true;
    }

    auto push(T new_value) {
        std::lock_guard guard(push_mutex);

        auto h = head.load();
        auto t = tail.load();
        if (t - h >= size) {
            return false;
        }

        tasks[t & mask] = std::move(new_value);
        tail++;
        return true;
    }
};

#endif