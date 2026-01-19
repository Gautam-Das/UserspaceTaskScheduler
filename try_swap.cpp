#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <iostream>

struct Context {
    void *rsp;
};

extern "C" {
void swap_context(Context *cur_c, Context *new_c);
void get_context(Context *c);
void set_context(Context *c);
void swap_context_stack(Context *cur_c, Context *new_c);
void swap_context_stack_first(Context *cur_c, Context *new_c);
}

Context main_context;
Context c;

void printContext(Context *context) {

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
    Context c;
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

    // create stack for foo
    char data[4096];

    // stack pointer, pointer at the end of the stack block
    auto sp = static_cast<char *>(data + sizeof(data));

    // align stack to 16 byte boundary
    sp = reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(sp) & -16L);

    // allow 128 bytes for red zone
    sp -= 128;

    // align stack
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