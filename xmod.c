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
        if(index > 2) return "-1"; 
        switch (mode[index]){
            case '0':
                (*changes)[index] = 0;
                break;
            case '1':
                (*changes)[index] = S_IXUSR;
                break;
            case '2':
                (*changes)[index] = S_IWUSR;
                break;
            case '3':
                (*changes)[index] = S_IWUSR | S_IXUSR;
            case '4':
                (*changes)[index] = S_IRUSR;
                break;
            case '5':
                (*changes)[index] = S_IRUSR | S_IXUSR;
                break;
            case '6':
                (*changes)[index] = S_IRUSR | S_IWUSR;
                break;
            case '7':
                (*changes)[index] = S_IXUSR | S_IRUSR | S_IWUSR;
                break;
            default:
                //invalid mode
                return "-1";
        }
        index++;
    }
    return "0";
}

struct Perms{
    char perm[4];
    int octal_mode;
};

int getCurrentPerms(char* file){
    struct stat st;
    int perms = 0;
    int *modeval = malloc(sizeof(int) * 9);
    if(stat(file, &st) == 0){
        mode_t perm = st.st_mode;
        modeval[0] = (perm & S_IRUSR) ? 100000000 : 0;
        modeval[1] = (perm & S_IWUSR) ? 10000000 : 0;
        modeval[2] = (perm & S_IXUSR) ? 100 : 0;
        modeval[3] = (perm & S_IRGRP) ? 40 : 0;
        modeval[4] = (perm & S_IWGRP) ? 20 : 0;
        modeval[5] = (perm & S_IXGRP) ? 10 : 0;
        modeval[6] = (perm & S_IROTH) ? 100 : 0;
        modeval[7] = (perm & S_IWOTH) ? 10 : 0;
        modeval[8] = (perm & S_IXOTH) ? 1 : 0;  
    } 
    for(int i = 0; i < 9; i++){
	
		perms = perms + modeval[i];
    }
    printf(" Perms already in file: %d\n", perms);
    return perms;
}


/// Creates an array with the valid combination of rwx operations
void build_Perms(struct Perms** perms_arr,int length){
    *perms_arr = malloc(length * sizeof(struct Perms));
    if(*perms_arr == NULL) return;
    struct Perms mode1;
    
    mode1.perm[0] = 'r';
    mode1.perm[1] = '\0';
    mode1.octal_mode = 4;
    (*perms_arr)[0] = mode1;

    struct Perms mode2;
    mode2.perm[0] = 'w';
    mode2.perm[1] = '\0';
    mode2.octal_mode = 10;
    (*perms_arr)[1] = mode2;

    struct Perms mode3;
    mode3.perm[0] = 'x';
    mode3.perm[1] = '\0';
    mode3.octal_mode = 1;
    (*perms_arr)[2] = mode3;

    struct Perms mode4; 
    mode4.perm[0]=  'r';
    mode4.perm[1]=  'w';
    mode4.perm[2]=  '\0';
    mode4.octal_mode = 110;
    (*perms_arr)[3] = mode4;

    struct Perms mode5;
    mode5.perm[0]=  'r';
    mode5.perm[1]=  'x';
    mode5.perm[2]=  '\0';
	mode5.octal_mode = 011;
	(*perms_arr)[4] = mode5;

    struct Perms mode6; 
    mode6.perm[0]=  'r';
    mode6.perm[1]=  'w';
    mode6.perm[2]=  'x';
    mode6.perm[3]=  '\0';
    mode6.octal_mode = 111;
    (*perms_arr)[5] = mode6;

    struct Perms mode7; 
    mode7.perm[0]=  'w';
    mode7.perm[1]=  'x';
    mode7.perm[2]=  '\0';
    mode7.octal_mode = 101;
    (*perms_arr)[6] = mode7;
}

char* set_changes_mode_str(char* str, char* file){
    struct Perms* perms_arr;
    int length = 7;
    build_Perms(&perms_arr,length);
    if(str[0] != 'u' && str[0] != 'g' && str[0] != 'o' && str[0] != 'a' ){
        return "-1";
    }
    if(str[1] != '=' && str[1] != '-' && str[1] != '+'){
        return "-1"; //command is formmatted incorectly
    }

    int perms = -1;
    int index = 2;
    char mode[4];
    while(str[index] != '\0'){
        if(index == 5) return "-1"; //invalid mode
        mode[index - 2] = str[index];
        index++;
    }
    mode[index-2] = '\0';
    //iterar pelo array de perms possiveis e verificar se o mode está lá dentro
    for(int i = 0; i < length;i++){
        if(strcmp(perms_arr[i].perm,mode) == 0 ){
            perms = perms_arr[i].octal_mode;
        }
    }
    printf("%d\n",perms);
    if(str[0] == 'u') perms = perms * 100;
    else if(str[0] == 'g') perms = perms * 10;
    else if(str[0] == 'a') perms = perms * 111;
    printf("Perms after user insert gotdamn: %d\n",perms);
    if(perms == -1){
        return "-1";
    }
    if(str[1] == '+'){
        //add the new permissions to already existant ones
        //lets read the current perms
        printf("%d perms + bef\n", perms);
        perms |= getCurrentPerms(file);
        printf("%d perms + aft\n", perms);
	}else if(str[1] == '-'){
        perms ^= getCurrentPerms(file);
        printf("%d perms -\n", perms);
    }

    char* octal_mode;
    sprintf(octal_mode, "%d", perms);
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

    printf("%s\n", mode);

    if(atoi(argv[1]) == 0){  //if atoi fails, then its a mode specified with letters
        printf("text mode\n");
        valid_mode = set_changes_mode_str(mode,filename);
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