#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"

#include "hw2_output.h"
#include "utils.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

sem_t *s_row1;
sem_t **s_sum2;

int **A, **B, **C, **D, **sum1, **sum2, **mul;
int r1, c1, r2, c2; // A&B: (r1 x c1), C&D: (r2 x c2)

// function to calculate one row of sum1 which is A+B
void* calculate_row1(void* p){
    int i;
    int curr_row = (int) p;
    pthread_mutex_lock(&mutex1);
    for(i = 0; i < c1; i++){
        sum1[curr_row][i] = A[curr_row][i] + B[curr_row][i];
        hw2_write_output(0, curr_row, i, sum1[curr_row][i]);
    }
    sem_post(&s_row1[curr_row]);
    pthread_mutex_unlock(&mutex1);
    
}

// function to calculate one row of sum2 which is C+D
void* calculate_row2(void* p){
    int i;
    int curr_row = (int) p;
    pthread_mutex_lock(&mutex2);
    for(i = 0; i < c2; i++){
        sum2[curr_row][i] = C[curr_row][i] + D[curr_row][i];
        sem_post(&s_sum2[curr_row][i]);
        hw2_write_output(1, curr_row, i, sum2[curr_row][i]);
    }
    pthread_mutex_unlock(&mutex2);
}

// function to calculate one row of mul which is sum1*sum2
void* calculate_mul(void* p){
    int i,j;
    int curr_row = (int) p;
    pthread_mutex_lock(&mutex3);
    for(i=0;i<c2;i++){
        int sum = 0;
        sem_wait(&s_row1[curr_row]);
        for(j=0;j<r2;j++){
            sem_wait(&s_sum2[j][i]);
        }
        for(j=0;j<c1;j++){
            sum += sum1[curr_row][j] * sum2[j][i];
        }
        mul[curr_row][i] = sum;
        sem_post(&s_row1[curr_row]);
        for(j=0;j<r2;j++){
            sem_post(&s_sum2[j][i]);
        }
        hw2_write_output(2, curr_row, i, mul[curr_row][i]);
    }
    pthread_mutex_unlock(&mutex3);
}

int main(){
    hw2_init_output();

    // define iterators
    int i, j;

    // initialize mutexes
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_mutex_init(&mutex3, NULL);
    
    // scan dimensions and matrix elements
    scanf("%d %d", &r1, &c1);      // scan A's dimensions
    initialize_matrix(r1, c1, &A); //allocate memory for A and scan it's input
    scanf("%d %d", &r1, &c1);      // scan B's dimensions
    initialize_matrix(r1, c1, &B); //allocate memory for B and scan it's input 
    scanf("%d %d", &r2, &c2);      // scan C's dimensions
    initialize_matrix(r2, c2, &C); //allocate memory for C and scan it's input
    scanf("%d %d", &r2, &c2);      // scan D's dimensions
    initialize_matrix(r2, c2, &D); //allocate memory for D and scan it's input 
    
    // calculate number of threads and create thread array
    int num_threads = r1 + r2 + r1;
    pthread_t threads[num_threads];

    // allocate memory for semaphores
    s_row1 = malloc(r1 * sizeof(sem_t));
    s_sum2 = malloc(r2 * sizeof(sem_t *));
    for(i=0;i<r2;i++){
        s_sum2[i] = malloc(c2 * sizeof(sem_t));
    }

    // initialize semaphores
    for(i=0;i<r1;i++){
        sem_init(&s_row1[i], 0, 0);
    }
    for(i=0;i<r2;i++){
        for(j=0;j<c2;j++){
            sem_init(&s_sum2[i][j], 0, 0);
        }
    }

    // allocate memory for resulting matrices
    sum1 = malloc(r1 * sizeof(int *));
    for(i = 0; i < r1; i++){
        sum1[i] = malloc(c1 * sizeof(int));
    }
    sum2 = malloc(r2 * sizeof(int *));
    for(i = 0; i < r2; i++){
        sum2[i] = malloc(c2 * sizeof(int));
    }
    mul = malloc(r1 * sizeof(int *));
    for(i = 0; i < r1; i++){
        mul[i] = malloc(c2 * sizeof(int));
    }

    // create threads
    int n = (r1 > r2) ? r1 : r2;
    i=0;j=0;
    while(n--){
        if(i < r1){
            pthread_create(&threads[i], NULL, calculate_row1, (void *) i);
            pthread_create(&threads[r1+r2+i], NULL, calculate_mul, (void *) i);
            i++;
        }
        if(j < r2){
            pthread_create(&threads[r1+j], NULL, calculate_row2, (void *) j);
            j++;
        }
    }

    // join threads
    n = (r1 > r2) ? r1 : r2;
    i=0;j=0;
    while(n--){
        if(i < r1){
            pthread_join(threads[i], NULL);
            pthread_join(threads[r1+r2+i], NULL);
            i++;
        }
        if(j < r2){
            pthread_join(threads[r1+j], NULL);
            j++;
        }
    }

    // destroy semaphores
    for(i=0;i<r1;i++){
        sem_destroy(&s_row1[i]);
    }
    for(i=0;i<r2;i++){
        for(j=0;j<c2;j++){
            sem_destroy(&s_sum2[i][j]);
        }
    }

    // free allocated memory
    free(s_row1);
    for(i=0;i<r2;i++){
        free(s_sum2[i]);
    }
    free(s_sum2);
    free_matrix(r1, &A);
    free_matrix(r1, &B);
    free_matrix(r2, &C);
    free_matrix(r2, &D);
    free_matrix(r1, &sum1);
    free_matrix(r2, &sum2);
    free_matrix(r1, &mul);

    return 0;
}