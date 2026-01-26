
all:
	as get_context.s -o get_context.o
	as set_context.s -o set_context.o
	as swap_context.s -o swap_context.o
	as swap_context_stack.s -o swap_context_stack.o
	g++ -c try_swap.cpp -o try_swap.o
	g++ try_swap.o get_context.o set_context.o swap_context.o swap_context_stack.o -o try_swap
	./try_swap

main1:
	as get_context.s -o get_context.o
	as set_context.s -o set_context.o
	as swap_context.s -o swap_context.o
	as swap_context_stack.s -o swap_context_stack.o
	g++ -c main.cpp -o main.o -std=c++20
	g++ main.o get_context.o set_context.o swap_context.o swap_context_stack.o -o main
	./main

stress_test:
	as get_context.s -o get_context.o
	as set_context.s -o set_context.o
	as swap_context.s -o swap_context.o
	as swap_context_stack.s -o swap_context_stack.o
	g++ -c stress_test.cpp -o stress_test.o -std=c++20
	g++ stress_test.o get_context.o set_context.o swap_context.o swap_context_stack.o -o stress_test
	./stress_test

bench:
	as get_context.s -o get_context.o
	as set_context.s -o set_context.o
	as swap_context.s -o swap_context.o
	as swap_context_stack.s -o swap_context_stack.o
	g++ -O3 -c bench.cpp -o bench.o -std=c++20
	g++ -O3 bench.o get_context.o set_context.o swap_context.o swap_context_stack.o -o bench
	./bench

clean:
	rm set_context.o get_context.o try_swap.o swap_context_stack.o swap_context.o main.o main stress_test stress_test.o bench bench.o try_swap a.exe