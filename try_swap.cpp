#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <iostream>

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

extern "C" {
void swap_context(TCB *cur_c, TCB *new_c);
void get_context(TCB *c);
void set_context(TCB *c);
void swap_context_stack(TCB *cur_c, TCB *new_c);
void swap_context_stack_first(TCB *cur_c, TCB *new_c);
}

TCB main_context;
TCB c;

void printContext(TCB *context) {

    printf("%p",
           context->rsp);
}

void foo() {
    static int x = 0;
    std::cout << "hello world from foo\n"
              << std::endl;
    if (x == 1)
        exit(0);
    x++;
    swap_context_stack(&c, &main_context);
    printf("second scary hello world from foo\n");
    exit(0);
}

auto bar() {
    printf("hello world from bar");
}

auto printtwice() {
    volatile int x = 0;
    TCB c;
    get_context(&c);

    printf("hello world %d\n", x);

    if (x == 0) {
        x++;
        set_context(&c);
    }
}

auto add_dummy_registers(char *sp, size_t n_regs) {
    for (auto i{0}; i < n_regs; ++i) {
        sp -= 8;
        *reinterpret_cast<uint64_t *>(sp) = 0;
    }
    return sp;
}

int main(void) {
    std::cout << sizeof(TCB) << std::endl;
    // create stack for foo
    char data[4096];

    // stack pointer, pointer at the end of the stack block
    auto sp = static_cast<char *>(data + sizeof(data));

    // align stack to 16 byte boundary
    sp = reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(sp) & -16L);

    // allow 128 bytes for red zone
    sp -= 128;

    // align stack because 7 things will be pushed that messes up the alignment
    sp -= 8;

    // push the rip to the top of the stack
    sp -= 8;
    *reinterpret_cast<uint64_t *>(sp) = reinterpret_cast<uint64_t>(foo);

    // push dummy values for 6 registers:
    // rbx, rbp, r12,r13,r14,r15
    sp = add_dummy_registers(sp, 6);
    c.rsp = reinterpret_cast<void *>(sp);

    while (true) {
        swap_context_stack(&main_context, &c);
        printf("returning from swap\n");
    }
    return 0;
}