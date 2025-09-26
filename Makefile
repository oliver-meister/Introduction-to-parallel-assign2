
numerical-integration: numerical-integration.c
	cc -Wall -std=c11 numerical-integration.c -o numerical-integration -pthread

sieve-of-eratosthenes: sieve-of-eratosthenes.c
	cc -Wall -std=c11 sieve-of-eratosthenes.c -o sieve-of-eratosthenes -pthread -lm

bench_dbg: benchmark_example.cpp benchmark.hpp sorted_list.hpp
	g++ -Wall -Wextra -g -std=c++17 -O0 -pthread benchmark_example.cpp -o bench_dbg

benchmark_example: benchmark_example.cpp benchmark.hpp sorted_list.hpp
	g++ -Wall -std=c++17 -pthread -O3 benchmark_example.cpp -o benchmark_example

test_numerical-integration: numerical-integration
	@for threads in 1 2 4 8 16 32; do \
		for trapezes in 1 10 100 1000 10000 100000 1000000 10000000; do \
			echo "Running with threads=$$threads trapezes=$$trapezes"; \
			./numerical-integration $$threads $$trapezes; \
		done \
	done
	
test_benchmark_example: benchmark_example
	@for threads in 2 4 6 8 16 32; do \
		echo "Running with threads=$$threads"; \
		./benchmark_example $$threads; \
	done

test_sieve-of-eratosthenes: sieve-of-eratosthenes
	@for threads in 1 2 4 8 16 32; do \
		for Max in 10 100 1000 10000 100000 1000000 5000000; do \
			echo "Running with threads=$$threads Max=$$Max"; \
			./sieve-of-eratosthenes $$Max $$threads; \
		done \
	done


clean:
	$(RM) numerical-integration
	$(RM) sieve-of-eratosthenes
	$(RM) benchmark_example
	$(RM) bench_dbg