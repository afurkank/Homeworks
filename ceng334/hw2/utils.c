#include "hw2_output.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "utils.h"

void initialize_matrix(int rows, int cols, int ***m){
    int i, j;
    *m = (int **) malloc(rows * sizeof(int *));
    for(i = 0; i < rows; i++){
        (*m)[i] = (int *) malloc(cols * sizeof(int));
    }
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            scanf("%d", &((*m)[i][j]));
        }
    }
}

void free_matrix(int rows, int ***m){
    int i;
    for(i = 0; i < rows; i++){
        free((*m)[i]);
    }
    free(*m);
}

void print_matrix(int rows, int cols, int **m){
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            printf("%2d ", m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}