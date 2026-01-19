.type swap_context_stack, @function
.global swap_context_stack
swap_context_stack:
    # Save return address (RIP)
    movq (%rsp), %r8
    movq %r8, 8*0(%rdi)

    # Save callee-saved registers
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Save RSP AFTER pushes
    movq %rsp, 8*1(%rdi)

    # Load target context
    movq 8*1(%rsi), %rsp    # RSP
    movq 8*0(%rsi), %r8     # RIP

    # Restore registers FROM SAME STACK
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    # Resume execution
    pushq %r8
    xor %eax, %eax
    ret
