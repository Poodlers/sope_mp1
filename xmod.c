#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

char* set_changes_mode_int(int** changes,int length, char* mode){
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
                        return "-1";
                
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
                        return "-1";
                
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
                        return "-1";
                
                 }
                 break;
                }
            index++;
                
        }
        return "0";
}

struct Perms{
    char* perm;
    char* octal_mode;
};


void build_Perms(struct Perms** perms_arr,int length){
    *perms_arr = malloc(length * sizeof(struct Perms));
    if(*perms_arr == NULL) return;

    struct Perms mode1;
    strcpy(mode1.perm, "ur");
    strcpy(mode1.octal_mode, "400");
    (*perms_arr)[0] = mode1;

    struct Perms mode2;
    strcpy(mode2.perm, "uw");
    strcpy(mode2.octal_mode, "200");
    (*perms_arr)[1] = mode2;

}

char* set_changes_mode_str(char* str){
    struct Perms* perms_arr;
    build_Perms(&perms_arr,28);
    if(str[0] != 'u' && str[0] != 'g' && str[0] != 'o' && str[0] != 'a' ){
        return "-1";
    }
    if(str[1] != '=' && str[1] != '-' && str[1] != '+'){
        return "-1"; //command is formmatted incorectly
    }
    int index = 2;
    char mode[4];
    mode[0] = str[0];
    while(str[index] != '\0'){
        if(index == 5) return "-1"; //invalid mode
        mode[index - 1] = str[index];
        index++;
    }
    char* octal_mode;

    for(int i = 0; i < 28;i++){
        if(strcmp(perms_arr[i].perm,mode) == 0 ){
            strcpy(octal_mode,perms_arr[i].octal_mode);
            break;
        }
    }
    free(perms_arr);
    return octal_mode;

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
    
    char* valid_mode;
    char* failed_mode = "-1";

    if(atoi(argv[1]) == 0){  //if atoi fails, then its a mode specified with letters
        valid_mode = set_changes_mode_str(mode);
        if(strcmp(valid_mode,failed_mode) != 0){
            set_changes_mode_int(&changes,3,valid_mode);
        }

    }else{
        valid_mode = set_changes_mode_int(&changes,3,mode);
    }

    if(strcmp(valid_mode,failed_mode) == 0){
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

     
