
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXCHAR 10000
#define MAXWORDLENGTH 100
//---------------------------------------------------Dynamic Array-------------------------------------------------------
typedef struct
{
    char **array;
    size_t used;
    size_t size;
} Array;

void initArray(Array *a, size_t initialSize)
{
    a->array = malloc(initialSize * sizeof(char *));
    a->used = 0;
    a->size = initialSize;

    for (int i = 0; i < initialSize; i++)
        a->array[i] = malloc((MAXCHAR + 1) * sizeof(char));
}

void insertArray(Array *a, char *element)
{
    if (a->used == a->size)
    {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(char *));
        for (int i = a->used; i < a->size; i++)
            a->array[i] = malloc((MAXCHAR + 1) * sizeof(char));
    }
    strcpy(a->array[a->used++], element);
}

void freeArray(Array *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}
//------------------------------------------------------End of Dynamic Array----------------------------------------------------
Array a;
int readAndSplit(char *filename)
{
    char text[MAXCHAR];
    FILE *fp;
    char str[MAXCHAR];

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Could not open file %s", filename);
        return 1;
    }

    initArray(&a, 5); // initially 5 elements
    int i = 0;
    while (fgets(str, MAXCHAR, fp) != NULL)
    {
        char *p = strtok(str, " ");
        while (p != NULL)
        {
            insertArray(&a, p); // automatically resizes as necessary
            p = strtok(NULL, " ");
        }
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{

    char *filename = "./a.txt";
    if (readAndSplit(filename) == 1)
    {
        return 1;
    }

    else
    {
        MPI_Init(&argc, &argv);
        int result = 0;
        // Get the number of processes
        int num_proc;
        MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
        // Get the rank of the process
        int my_id;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

        char target[100];
        if (my_id == 0)
        {
            puts("Please enter a word (We are Case-Sensetive :) ) : ");
            scanf("%s", &target);
        }
        MPI_Bcast(&target, 100, MPI_CHAR, 0,
                  MPI_COMM_WORLD);
        int partial_sum = 0;
        int proc_subArray = a.used / (num_proc - 1);
        if (my_id != 0)
        {
            int proc_start = (my_id - 1) * proc_subArray;
            int proc_end = proc_start + proc_subArray;
            for (int i = proc_start; i < proc_end; i++)
            {
                if (strcmp(a.array[i], target) == 0)
                    partial_sum++;
            }
        }
        else
        {
            partial_sum = 0;
            int remaining = a.used % (num_proc - 1);
            int last_proc_start = (num_proc - 2) * proc_subArray;
            int last_proc_end = last_proc_start + proc_subArray;

            for (int i = last_proc_end; i < last_proc_end + remaining; i++)
            {
                if (strcmp(a.array[i], target) == 0)
                    partial_sum++;
            }
        }
        MPI_Reduce(&partial_sum, &result, 1, MPI_INT,
                   MPI_SUM, 0, MPI_COMM_WORLD);
        if (my_id == 0)
        {
            printf("the word has appeared %i times\n", result);
        } // Finalize the MPI environment.
        MPI_Finalize();
    }
}