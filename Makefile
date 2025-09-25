
numerical-integration: numerical-integration.c
	cc -Wall -std=c11 numerical-integration.c -o numerical-integration -pthread

sieve-of-eratosthenes: sieve-of-eratosthenes.c
	cc -Wall -std=c11 sieve-of-eratosthenes.c -o sieve-of-eratosthenes -pthread -lm

test_numerical-integration: numerical-integration
	@for threads in 1 2 4 8 16 32; do \
		for trapezes in 1 10 100 1000 10000 100000 1000000 10000000; do \
			echo "Running with threads=$$threads trapezes=$$trapezes"; \
			./numerical-integration $$threads $$trapezes; \
		done \
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