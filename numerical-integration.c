
#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct{

    double *result;
    double w;
    int thread_id;
    int num_threads;
    int num_trapezes;
    pthread_mutex_t *lock;

} trap_args;

double function_f(double x)
{
    return 4 / (1+(x*x));
}

void *numerical_integration(void *arg)
{
    trap_args* a = (trap_args*)arg;
    double local = 0.0;

    for(int current_trap = a->thread_id; current_trap < a->num_trapezes; current_trap += a->num_threads)
    {
        double x_left = current_trap * a->w;
        double x_right = x_left + a->w;
        local += 0.5 * a->w * (function_f(x_left) + function_f(x_right));
    }
    pthread_mutex_lock(a->lock);
    *(a->result) += local;
    pthread_mutex_unlock(a->lock);
    return NULL;
}

int main(int argc, char *argv[], char* envp[])
{
    if (argc > 0 && strcmp(argv[1], "-h") == 0)
    {
        printf("Usage: ./numerical-integration <threads> <trapezes>\n");
        return 0;
    }
    if(argc != 3)
    {
        printf("Wrong number of arguments, enter: number of threads, number of trapezes\n");  
        return 0;
    }
    int num_threads = atoi(argv[1]);
    int num_trapezes = atoi(argv[2]);
    if(num_threads <= 0)
    {
        printf("number of threads must be greater than 0\n");
        return 0;
    }
    if(num_trapezes <= 0)
    {
        printf("number of trapezes must be greater than 0\n");
        return 0;
    }

    double w = 1.0 / num_trapezes;
    double result = 0.0;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    
    pthread_t threads[num_threads];
    trap_args args[num_threads];

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++)
    {
        args[i] = (trap_args) {.result = &result, 
            .w = w, 
            .thread_id = i, 
            .num_trapezes = num_trapezes, 
            .num_threads = num_threads, 
            .lock = &lock};
        pthread_create(&threads[i], NULL, numerical_integration, &args[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    pthread_mutex_destroy(&lock);
    printf("integral ≈ %f\n", result);
    printf("Elapsed: %.9f seconds\n ////////////////////////// \n", elapsed);
    return 0; 
}