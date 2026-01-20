#ifndef TCB
#define TCB

#include <cstdint>

struct TCB {
    enum class State {
        READY = 0,
        RUNNING = 1,
        WAITING = 2,
        DONE = 3
    };
    void *rsp;
    uint32_t state : 2;
    uint32_t priority : 4;
    uint32_t small_stack : 1;
    uint32_t task_id : 25;

    void set_state(State state) {
        this->state = static_cast<int>(state);
    }
    void set_priority(char priority) {
        this->priority = priority;
    }
    void set_small_stack_flag(bool small_stack) {
        this->small_stack = small_stack;
    }
    void set_id(int task_id) {
        this->task_id = task_id;
    }

    void set_all(State state, char priority, bool small_stack, int task_id) {
        this->state = static_cast<int>(state);
        this->priority = priority;
        this->task_id = task_id;
        this->small_stack = small_stack;
    }

    constexpr auto get_state() const { return this->state; }
    constexpr auto get_priority() const { return this->priority; }
};

#endif