#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

int set_changes_mode_int(int** changes,int length, char* mode){
    printf("%s",mode);
    int index = 0;
    free(*changes);
    *changes = (int*)malloc(length * sizeof(int));
    while(mode[index] != '\0'){
        switch(index){
            case 0:
                switch (mode[index])
                {
                    case '0':
                        (*changes)[0] = 0;
                        break;
                    case '1':
                        (*changes)[0] = S_IXUSR;
                        break;
                    case '2':
                        (*changes)[0] = S_IWUSR;
                        break;
                    case '3':
                        (*changes)[0] = S_IWUSR | S_IXUSR;
                    case '4':
                        (*changes)[0] = S_IRUSR;
                        break;
                    case '5':
                        (*changes)[0] = S_IRUSR | S_IXUSR;
                        break;
                    case '6':
                        (*changes)[0] = S_IRUSR | S_IWUSR;
                        break;
                    case '7':
                        (*changes)[0] = S_IXUSR | S_IRUSR | S_IWUSR;
                        break;
                    default:
                        //invalid mode
                        return -1;
                
                }
                break;
            case 1:
                switch (mode[index])
                {
                    case '0':
                        (*changes)[1] = 0;
                        break;
                    case '1':
                        (*changes)[1] = S_IXGRP;
                        break;
                    case '2':
                        (*changes)[1] = S_IWGRP;
                        break;
                    case '3':
                        (*changes)[1] = S_IWGRP | S_IXGRP;
                    case '4':
                        (*changes)[1] = S_IRGRP;
                        break;
                    case '5':
                        (*changes)[1] = S_IRGRP | S_IXGRP;
                        break;
                    case '6':
                        (*changes)[1] = S_IRGRP | S_IWGRP;
                        break;
                    case '7':
                        (*changes)[1] = S_IXGRP | S_IRGRP | S_IWGRP;
                        break;
                     default:
                        //invalid mode
                        return -1;
                
                }
                break;
            case 2:
                switch (mode[index])
                {
                    case '0':
                        (*changes)[2] = 0;
                        break;
                    case '1':
                        (*changes)[2] = S_IXOTH;
                        break;
                    case '2':
                        (*changes)[2] = S_IWOTH;
                        break;
                    case '3':
                        (*changes)[2] = S_IWOTH | S_IXOTH;
                    case '4':
                        (*changes)[2] = S_IROTH;
                        break;
                    case '5':
                        (*changes)[2] = S_IROTH | S_IXOTH;
                        break;
                    case '6':
                        (*changes)[2] = S_IROTH | S_IWOTH;
                        break;
                    case '7':
                        (*changes)[2] = S_IXOTH | S_IROTH | S_IWOTH;
                        break;
                     default:
                        //invalid mode
                        return -1;
                
                 }
                 break;
                }
            index++;
                
        }
        return 0;
}

int set_changes_mode_str(int** changes,int length, char* str){
    if(str[0] != '-'){
        return -1;
    }
    int index = 1;
    while(str[index] != '\0'){
        
    }
    return 0;
}

int main(int argc, char *argv[] ){
    int *changes;
    changes = NULL;
    char filename[120];
    char mode[120];
    strcpy(mode,argv[1]);
    strcpy(filename,argv[2]);
    struct stat buffer;
    struct passwd *pwd;
    int status;
    printf("%s\n", filename);
    int valid_mode;
    

    if(atoi(argv[1]) == 0){  //if atoi fails, then its a mode specified with letters
        valid_mode = set_changes_mode_str(&changes,3,mode);

    }else{
        valid_mode = set_changes_mode_int(&changes,3,mode);
    }

    if(valid_mode == -1){
        printf("Invalid mode! \n");
        return -1;
    }
    

    int final_change = changes[0] | changes[1] | changes[2];

    if(chmod(filename, final_change) != 0){
        perror("Chmod failed: ");
    }
    status = stat(filename,&buffer);
    if (status != 0){
        perror("Stat failed: \n");
    }
    
    /* Print out owner's name if it is found using getpwuid(). */
    if ((pwd = getpwuid(buffer.st_uid)) != NULL)
        printf(" %-8.8s\n", pwd->pw_name);
    else
        printf(" %-8d\n", buffer.st_uid);
    
    free(changes);
       
}

     
