
//---------------------------------------------------Dynamic Array-------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>

#define MAXCHAR 10000
#define MAXWORDLENGTH 100

//---------------------------------------------------Dynamic Array-------------------------------------------------------
struct data
{
    char word_string[100];
};
typedef struct
{
    struct data *array;
    size_t used;
    size_t size;
} Array;

void initArray(Array *a, size_t initialSize)
{
    a->array = malloc(initialSize * 100);
    a->used = 0;
    a->size = initialSize;

    // for (int i = 0; i < initialSize; i++)
    //     a->array[i] = malloc(100);
}

void insertArray(Array *a, char *element)
{
    struct data el;
    strcpy(el.word_string, element);
    if (a->used == a->size)
    {
        a->size *= 2;
        a->array = realloc(a->array, a->size * 100);
        // for (int i = a->used; i < a->size; i++)
        //     a->array[i] = malloc((MAXCHAR + 1) * sizeof(char));
    }
    // strcpy(a->array[a->used++], element);
    a->array[a->used++] = el;
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

void search()
{
    MPI_Init(NULL, NULL);
    int my_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    int num_proc;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    char target[100];
    int result;
    //-------------------------------------defining the struct--------------------------------------
    const int nitems = 1;
    int blocklengths[1] = {MAXWORDLENGTH};
    MPI_Datatype types[1] = {MPI_CHAR};
    MPI_Datatype mpi_string_type;
    MPI_Aint offsets[1];

    offsets[0] = offsetof(struct data, word_string);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_string_type);
    MPI_Type_commit(&mpi_string_type);
    //---------------------------------------------------------------------------
    int proc_subArray;
    char *filename = "./c.txt";

    if (my_id == 0)
    {
        readAndSplit(filename);
        proc_subArray = a.used / (num_proc);
        puts("Please enter a word (We are Case-Sensetive :) ) : ");
        scanf("%s", &target);
        //strcpy(&target, "in");
    }
    // printf("process %i  %s", my_id, a.array[0]);
    MPI_Bcast(&target, 100, MPI_CHAR, 0, MPI_COMM_WORLD);

    MPI_Bcast(&proc_subArray, 1, MPI_INT, 0, MPI_COMM_WORLD);

    struct data recv[proc_subArray];
    MPI_Scatter(a.array, proc_subArray, mpi_string_type, &recv, proc_subArray, mpi_string_type, 0, MPI_COMM_WORLD);
    int partial_sum = 0;

    for (int i = 0; i < proc_subArray; i++)
    {
        if (strcmp(recv[i].word_string, target) == 0)
            partial_sum++;
    }

    if (my_id == 0)
    {
        int start = proc_subArray * num_proc;
        for (int i = start; i < a.used; i++)
        {
            if (strcmp(a.array[i].word_string, target) == 0)
                partial_sum++;
        }
    }

    MPI_Reduce(&partial_sum, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (my_id == 0)
    {
        printf("the word has appeared %i times\n", result);
    }
    MPI_Finalize();
}

int main(int argc, char **argv)
{
    search();
}
