#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

typedef struct{
    int num_seeds;
    int *seeds;
    bool *marked;
    int *chuncks;
    int thread_id;
} sieve_args;


void* sieve(void * args)
{
    sieve_args* a = (sieve_args*)args; 
    for(int i = 0; i < a->num_seeds; i++)
    {
        int seed = a->seeds[i];
        int start = a->chuncks[a->thread_id];
        int end = a->chuncks[a->thread_id + 1];
        int first = ((start + seed - 1) / seed) * seed;
        for(int current = first; current <= end; current += seed)
        {
            a->marked[current] = true;
        }
    }
    return NULL;
}

int main(int argc, char *argv[], char* envp[])
{
    if (argc > 0 && strcmp(argv[1], "-h") == 0)
    {
        printf("Usage: ./sieve-of-eratosthenes <Max> <threads> \n");
        return 0;
    }
    if(argc != 3)
    {
        printf("Wrong number of arguments, enter: Max, number of threads\n");  
        return 0;
    }
    int Max = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    if(Max <= 1)
    {
        printf("Max must be greater than 1\n");
        return 0;
    }
    
    int sqrt_max = (int)sqrt(Max);
    bool marked[Max+1];

    for (int i = 0; i <= Max; i ++)
    {
        marked[i] = false;
    }

    marked[0] = true;
    marked[1] = true;

    int k = 2;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (k*k <= sqrt_max)
    {
        for(int multiple = k*k; multiple <= sqrt_max; multiple += k)
        {
            marked[multiple] = true;
        }
        int smallest;
        for(smallest = k+1; smallest <= sqrt_max; smallest ++)
        {
            if(marked[smallest] == false)
            {
                k = smallest;
                break;
            }
        }
        if (smallest > sqrt_max)
        {
            break;
        }
    }

    int seeds[sqrt_max];
    int num_seeds = 0;

    for(int i = 2; i <= sqrt_max; i++)
    {
        if (marked[i] == false)
        {
            seeds[num_seeds] = i;
            num_seeds++;
        }
    }   

    int chuncks[num_threads+1];
    chuncks[0] = sqrt_max+1;
    chuncks[num_threads] = Max;
    
    int offset = (Max - (sqrt_max+1)) / num_threads;
    for(int i = 1; i < num_threads; i++)
    {
        chuncks[i] = chuncks[0] + offset * i;
    }

    pthread_t threads[num_threads];
    sieve_args args[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        args[i] = (sieve_args) {.chuncks = chuncks, .marked = marked, .num_seeds = num_seeds, .seeds = seeds, .thread_id = i};
        pthread_create(&threads[i], NULL, sieve, &args[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed: %.9f seconds\n", elapsed);

    printf("[");
    for(int i = 2; i <= Max; i++)
    {
        if (marked[i] == false)
        {
            printf("%d, ", i);
        }
    } 
    printf("]\n");


}