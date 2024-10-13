#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

STREAM_TYPE *a, *b, *c;

typedef enum {
    OP_COPY,
    OP_SCALE,
    OP_ADD,
    OP_TRIAD
} operation_t;

typedef struct {
    int thread_id;
    ssize_t start_index;
    ssize_t end_index;
    int num_iterations;
    operation_t operation;
    STREAM_TYPE scalar;
} thread_data_t;

/* Function prototypes with noinline attribute */
__attribute__((noinline)) void array_copy(ssize_t start, ssize_t end);
__attribute__((noinline)) void array_scale(ssize_t start, ssize_t end, STREAM_TYPE scalar);
__attribute__((noinline)) void array_add(ssize_t start, ssize_t end);
__attribute__((noinline)) void array_triad(ssize_t start, ssize_t end, STREAM_TYPE scalar);

void *thread_function(void *arg);

int main(int argc, char *argv[]) {
    int num_threads = 1;
    ssize_t array_size = 10000000;
    int num_iterations = 10;
    operation_t operation = OP_COPY;
    STREAM_TYPE scalar = 3.0;

    int opt;
    while ((opt = getopt(argc, argv, "n:s:i:o:c:")) != -1) {
        switch (opt) {
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 's':
                array_size = atoll(optarg);
                break;
            case 'i':
                num_iterations = atoi(optarg);
                break;
            case 'o':
                if (strcmp(optarg, "copy") == 0) {
                    operation = OP_COPY;
                } else if (strcmp(optarg, "scale") == 0) {
                    operation = OP_SCALE;
                } else if (strcmp(optarg, "add") == 0) {
                    operation = OP_ADD;
                } else if (strcmp(optarg, "triad") == 0) {
                    operation = OP_TRIAD;
                } else {
                    fprintf(stderr, "Unknown operation: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'c':
                scalar = atof(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -n num_threads -s array_size -i num_iterations -o operation -c scalar\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Check parameters
    if (num_threads < 1) {
        fprintf(stderr, "Number of threads must be at least 1\n");
        exit(EXIT_FAILURE);
    }
    if (array_size < 1) {
        fprintf(stderr, "Array size must be at least 1\n");
        exit(EXIT_FAILURE);
    }
    if (num_iterations < 1) {
        fprintf(stderr, "Number of iterations must be at least 1\n");
        exit(EXIT_FAILURE);
    }

    // Allocate arrays
    a = (STREAM_TYPE *) malloc(sizeof(STREAM_TYPE) * array_size);
    b = (STREAM_TYPE *) malloc(sizeof(STREAM_TYPE) * array_size);
    c = (STREAM_TYPE *) malloc(sizeof(STREAM_TYPE) * array_size);

    if (a == NULL || b == NULL || c == NULL) {
        fprintf(stderr, "Failed to allocate arrays\n");
        exit(EXIT_FAILURE);
    }

    // Initialize arrays
    ssize_t j;
    for (j = 0; j < array_size; j++) {
        a[j] = 1.0;
        b[j] = 2.0;
        c[j] = 0.0;
    }

    // Create threads
    pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
    thread_data_t *thread_data = (thread_data_t *) malloc(sizeof(thread_data_t) * num_threads);

    if (threads == NULL || thread_data == NULL) {
        fprintf(stderr, "Failed to allocate thread structures\n");
        exit(EXIT_FAILURE);
    }

    // Divide the array among threads
    ssize_t chunk_size = array_size / num_threads;
    ssize_t remainder = array_size % num_threads;

    // Divide iterations among threads
    int iterations_per_thread = num_iterations / num_threads;
    int iterations_remainder = num_iterations % num_threads;

    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);

    int i;
    for (i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].start_index = i * chunk_size;
        thread_data[i].end_index = (i == num_threads - 1) ? array_size : (i + 1) * chunk_size;
        thread_data[i].num_iterations = iterations_per_thread + (i < iterations_remainder ? 1 : 0);
        thread_data[i].operation = operation;
        thread_data[i].scalar = scalar;
        int rc = pthread_create(&threads[i], NULL, thread_function, (void *)&thread_data[i]);
        if (rc) {
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for threads to complete
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end_time, NULL);

    double elapsed_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                           (end_time.tv_usec - start_time.tv_usec)) / 1000000.0;

    // Report results
    printf("Operation: %s\n", (operation == OP_COPY) ? "Copy" :
                               (operation == OP_SCALE) ? "Scale" :
                               (operation == OP_ADD) ? "Add" : "Triad");
    printf("Threads: %d\n", num_threads);
    printf("Array size: %zd\n", array_size);
    printf("Iterations per thread: %d\n", iterations_per_thread);
    printf("Total iterations: %d\n", num_iterations);
    printf("Elapsed time: %f seconds\n", elapsed_time);
    printf("Bandwidth: %f MB/s\n", (double)(array_size * sizeof(STREAM_TYPE) * num_iterations * 1.0e-6) / elapsed_time);

    // Clean up
    free(a);
    free(b);
    free(c);
    free(threads);
    free(thread_data);

    return 0;
}

__attribute__((noinline)) void array_copy(ssize_t start, ssize_t end) {
    ssize_t j;
    for (j = start; j < end; j++) {
        c[j] = a[j];
    }
}

__attribute__((noinline)) void array_scale(ssize_t start, ssize_t end, STREAM_TYPE scalar) {
    ssize_t j;
    for (j = start; j < end; j++) {
        b[j] = scalar * c[j];
    }
}

__attribute__((noinline)) void array_add(ssize_t start, ssize_t end) {
    ssize_t j;
    for (j = start; j < end; j++) {
        c[j] = a[j] + b[j];
    }
}

__attribute__((noinline)) void array_triad(ssize_t start, ssize_t end, STREAM_TYPE scalar) {
    ssize_t j;
    for (j = start; j < end; j++) {
        a[j] = b[j] + scalar * c[j];
    }
}

void *thread_function(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int i;
    for (i = 0; i < data->num_iterations; i++) {
        switch (data->operation) {
            case OP_COPY:
                array_copy(data->start_index, data->end_index);
                break;
            case OP_SCALE:
                array_scale(data->start_index, data->end_index, data->scalar);
                break;
            case OP_ADD:
                array_add(data->start_index, data->end_index);
                break;
            case OP_TRIAD:
                array_triad(data->start_index, data->end_index, data->scalar);
                break;
            default:
                fprintf(stderr, "Unknown operation\n");
                pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}
