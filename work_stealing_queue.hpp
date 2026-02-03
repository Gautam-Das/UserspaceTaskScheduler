#ifndef WORK_STEALING_QUEUE
#define WORK_STEALING_QUEUE

#include <atomic>
#include <bit>
#include <cstddef>
template <typename T, size_t InjBuffer, size_t Capacity>
class work_stealing_queue {
    static_assert(std::popcount(Capacity) == 1, "Capacity must be a power of 2");
    static_assert(std::popcount(InjBuffer) == 1, "Injection Buffer size must be a power of 2");

private:
    alignas(64) std::atomic<size_t> top_{0};
    alignas(64) size_t bottom_{0};

    alignas(64) T buffer_[Capacity];

    static constexpr size_t mask = Capacity - 1;

public:
    bool push(T item) {
        size_t b = bottom_;
        size_t t = top_.load(std::memory_order_acquire);
        if (b - t >= Capacity)
            return false;

        buffer_[b & mask] = std::move(item);
        std::atomic_thread_fence(std::memory_order_release);
        bottom_ = b + 1;
        return true;
    }

    bool try_pop(T& result) {
        auto b = bottom_ - 1; // reserve
        bottom_ = b;

        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto t = top_.load(std::memory_order_acquire);
        if (t > b) {
            // empty
            bottom_ = b + 1;
            return false;
        }

        result = buffer_[b & mask];

        if (t == b) {
            // it was the last item, therefore could be race with stealing threads
            if (!top_.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
                // lost race, t not as expected
                bottom_ = b + 1;
                return false;
            }
            bottom_ = b + 1;
        }
        return true;
    }

    bool steal(T& result) {
        auto t = top_.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto b = bottom_;
        if (t >= b)
            return false;

        result = buffer_[t & mask];

        if (!top_.compare_exchange_strong(
                t, t + 1,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            return false;
        }

        return true;
    }
};

#endif