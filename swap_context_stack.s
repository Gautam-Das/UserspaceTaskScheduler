.type swap_context_stack, @function
.global swap_context_stack
swap_context_stack:
    # Save callee-saved registers
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Save RSP AFTER pushes
    movq %rsp, (%rdi)

    # Load target context
    movq (%rsi), %rsp    # RSP

    # Restore registers FROM NEW STACK
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    
    # Resume execution
    xor %eax, %eax
    ret
