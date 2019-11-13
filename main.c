#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
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
static Array a;
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

            insertArray(&a, p);
            p = strtok(NULL, " ");
        }
    }
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[])
{
    int result = 0;
    char *filename = "./a.txt";
    if (readAndSplit(filename) == 1)
    {
        return 1;
    }

    else
    {
        char target[MAXCHAR];

        puts("Please enter a word (We are Case-Sensetive :) ) : ");
        scanf("%s", target);
        for (int i = 0; i < a.used; i++)
        {
            if (strcmp(a.array[i], target) == 0)
                result++;
        }

        printf("the word has appeared %i times \n", result);
    }

    return 0;
}
