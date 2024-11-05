#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "lab3.h"
#include <string.h>
#include <ctype.h>


extern int **sudoku_board;
int *worker_validation;
void *validate(void *param);

int **read_board_from_file(char *filename)
{
    FILE *fp = NULL;
    int **board = NULL;
    worker_validation = (int *)malloc(NUM_OF_THREADS * sizeof(int));

    const int MAX_SIZE = 255;
    const int NUM_OF_ROWS = 9;
    const int NUM_OF_COLS = 9;

    sudoku_board = (int **)malloc(NUM_OF_ROWS * sizeof(int *));
    for (int i = 0; i < NUM_OF_COLS; i++)
    {
        sudoku_board[i] = (int *)malloc(NUM_OF_COLS * sizeof(int));
    }

    fp = fopen(filename, "r");
    char buffer[MAX_SIZE];
    int numbers = 0;

    if (fp == NULL)
    {
        printf("Error reading file");
    }
    else
    {
        // create sudoku board (9x9 matrix)
        int row = 0;
        int col = 0;
        while (fgets(buffer, MAX_SIZE, fp))
        {
            for (int i = 0; i < strlen(buffer); i++)
            {
                if (isdigit(buffer[i]))
                {
                    numbers += 1;
                    row = ((numbers - 1) / 9);
                    col = ((numbers - 1) % 9);
                    char element = buffer[i];
                    int numElement = (int)(element - '0');
                    sudoku_board[row][col] = numElement;
                }
            }
        }
    }

    fclose(fp);
    return sudoku_board;
}

int is_board_valid()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_t *tid = (pthread_t *)malloc(sizeof(int) * NUM_OF_THREADS);
    param_struct *params = (param_struct *)malloc(sizeof(param_struct) * NUM_OF_THREADS);

    int index = 0;

    // check if every row is valid
    for (int i = 0; i < 9; i++)
    {
        params[index].id = index;
        params[index].starting_row = i;
        params[index].starting_col = 0;
        params[index].ending_row = i + 1;
        params[index].ending_col = 0;
        pthread_create(&(tid[index]), &attr, validate, &(params[index]));
        pthread_join(tid[i], NULL);
        index++;
    }

    // check if every column is valid
    for (int i = 0; i < 9; i++)
    {
        params[index].id = index;
        params[index].starting_row = 0;
        params[index].starting_col = i;
        params[index].ending_row = 0;
        params[index].ending_col = i + 1;
        pthread_create(&(tid[index]), &attr, validate, &(params[index]));
        pthread_join(tid[i], NULL);
        index++;
    }

    // check if every subgrid is valid
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int start_row = i * 3;
            int end_row = start_row + 2;
            int start_col = j * 3;
            int end_col = start_col + 2;
            params[index].id = index;
            params[index].starting_row = start_row;
            params[index].starting_col = start_col;
            params[index].ending_row = end_row;
            params[index].ending_col = end_col;
            pthread_create(&(tid[index]), &attr, validate, &(params[index]));
            pthread_join(tid[i], NULL);
            index++;
        }
    }

    // checks if every thread returned a valid or invalid number
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        if (worker_validation[i] != 1)
        {
            return 0;
        }
    }

    return 1;
}

// function used to sort the numbers in a given colunm/row/subgrid
int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

void *validate(void *param)
{

    param_struct *parameter;
    parameter = param;

    int start_row = parameter->starting_row;
    int end_row = parameter->ending_row;
    int start_col = parameter->starting_col;
    int end_col = parameter->ending_col;
    int param_id = parameter->id;

    int current_numbers[9];

    // adds each number in each row to the numbers array
    if (start_col == 0 && end_col == 0)
    {
        for (int i = 0; i < 9; i++)
        {
            current_numbers[i] = sudoku_board[0][i];
        }
    }
    // adds each number in each column to the numbers array
    else if (start_row == 0 && end_row == 0)
    {
        for (int i = 0; i < 9; i++)
        {
            current_numbers[i] = sudoku_board[i][start_col];
        }
    }
    // adds each number in each subgrid to the numbers array
    else
    {
        int index = 0;
        for (int i = start_row; i <= end_row; i++)
        {
            for (int j = start_col; j <= end_col; j++)
            {
                current_numbers[index] = sudoku_board[i][j];
                index++;
            }
        }
    }

    qsort(current_numbers, 9, sizeof(int), compare);

    int isValid = 1;
    // checks if the numbers array contains every digit from 1-9
    for (int i = 1; i <= 9; i++)
    {
        if (i != current_numbers[i - 1])
        {
            isValid = 0;
        }
    }

    worker_validation[param_id] = isValid;
    pthread_exit(0);
}
