#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int main(void)
{    
   char *symlinkpath = "exercise_5.c";
    char *ptr;

    ptr = realpath(symlinkpath, NULL); 
    printf("%s", ptr);
    
}