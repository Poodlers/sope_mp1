#include <stdio.h>
#include <stdlib.h>

void change(int **array, int length,char* mode)
{
    printf("%s",mode);
    *array = malloc(length * sizeof(int));
    if (*array == NULL)
        return;
    for (int i = 0 ; i < length ; i++)
        (*array)[i] = 1;
}

int main(){
    int *array;
    int length = 3;
    char* mode = "bruh";
    array = NULL;
    change(&array, length,mode);

    change(&array, length,mode);
    change(&array, length,mode);
    change(&array, length,mode);
    for (int i = 0 ; i < length ; i++)
        printf("%d",array[i]);
    free(array);
}