.type swap_context_stack_first, @function
.global swap_context_stack_first
swap_context_stack_first:
  # Save the return address.
  movq (%rsp), %r8
  movq %r8, 8*0(%rdi) # RIP
  

  # try saving on stack
  pushq %rbx
  pushq %rbp
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15

  movq %rsp, 8*1(%rdi) # RSP

  # Should return to the address set with {get, swap}_context.
  movq 8*0(%rsi), %r8

  # Load new stack pointer.
  movq 8*1(%rsi), %rsp

  # Push RIP to stack for RET.
  pushq %r8

  # Return.
  xorl %eax, %eax
  ret