#include "hw2_output.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

void initialize_matrix(int rows, int cols, int ***m);

void free_matrix(int rows, int ***m);

void print_matrix(int rows, int cols, int **m);