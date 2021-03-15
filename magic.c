#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>


long int findSize(char file_name[260]) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    long int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 

int search_dir(char* dir){
    struct dirent *de;  // Pointer for directory entry     
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir(dir); 
    printf("running search_dir in dir: %s \n", dir);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
  
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    long int res;

    while ((de = readdir(dr)) != NULL){
        if(de->d_type == DT_DIR){ //if path is directory

            if(de->d_name[0] != '.'){
                //se não é nem o current_dir nem o pai do curr_dir
                //criar novo processo para dar parse às files deste dir
                printf("process with pid: %d running on dir: %s is about to cause a fork for dir %s\n\n",getpid(), dir, de->d_name);
                pid_t id = fork();
                int status;
                switch (id) {
                    case -1:
                        perror ("fork"); 
                        exit (1);
                    case 0: //child process
                        // chamar search_dir() com novo 
                        {
                            printf("\n");
                            printf("child process with pid %d exploring dir %s \n", getpid(),de->d_name);
                            char* curr_dir = malloc(260 * sizeof(char));
                            strcat(curr_dir,dir);
                            strcat(curr_dir,"/");
                            strcat(curr_dir,de->d_name);
                            search_dir(curr_dir);
                            free(curr_dir);
                            return 0;
                        }
						
                        break;
                    default: //parent process
                        printf("parent process with pid %d exploring dir %s \n", getpid(), dir);
                        waitpid(id,&status,0);
                        printf("CHILD PROCESS DIED\n");
						break;
				}
            }    
        }
        else if(de->d_type == DT_REG){
            char * filename = malloc(260 * sizeof(char));
            strcat(filename,dir);
            strcat(filename,"/");
            strcat(filename,de->d_name);
            res = findSize(filename);
            free(filename); 
            if (res != -1) 
                printf("%s - %ld bytes \n",de->d_name, res);
        }
	}
	
    return 0; 
}

int main( int argc, char *argv[] ) 
{ 

    search_dir(argv[1]);
} 