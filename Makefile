
all:
	as get_context.s -o get_context.o
	as set_context.s -o set_context.o
	as swap_context.s -o swap_context.o
	as swap_context_stack.s -o swap_context_stack.o
	as swap_context_stack_first.s -o swap_context_stack_first.o
	g++ -c try_swap.cpp -o try_swap.o
	g++ try_swap.o get_context.o set_context.o swap_context.o swap_context_stack.o swap_context_stack_first.o -o try_swap
	./try_swap
clean:
	rm set_context.o get_context.o try_swap.o swap_context_stack_first.o swap_context_stack.o swap_context.o