#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include "logs.h"
#include <fcntl.h>
#include <stdbool.h>


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
        modeval[0] = (perm & S_IRUSR) ? 0400 : 0;
        modeval[1] = (perm & S_IWUSR) ? 0200 : 0;
        modeval[2] = (perm & S_IXUSR) ? 0100 : 0;
        modeval[3] = (perm & S_IRGRP) ? 040 : 0;
        modeval[4] = (perm & S_IWGRP) ? 020 : 0;
        modeval[5] = (perm & S_IXGRP) ? 010 : 0;
        modeval[6] = (perm & S_IROTH) ? 04 : 0;
        modeval[7] = (perm & S_IWOTH) ? 02 : 0;
        modeval[8] = (perm & S_IXOTH) ? 01 : 0;  
    } 
    for(int i = 0; i < 9; i++){
		perms = perms | modeval[i];
    }
   
    return perms;
}

void getPermsStringFormat(int perm, char str[9]){
    str[0] = (perm & S_IRUSR) ? 'r' : '-';
    str[1] = (perm & S_IWUSR) ? 'w' : '-';
    str[2] = (perm & S_IXUSR) ? 'x' : '-';
    str[3] = (perm & S_IRGRP) ? 'r' : '-';
    str[4] = (perm & S_IWGRP) ? 'w' : '-';
    str[5] = (perm & S_IXGRP) ? 'x' : '-';
    str[6] = (perm & S_IROTH) ? 'r' : '-';
    str[7] = (perm & S_IWOTH) ? 'w' : '-';
    str[8] = (perm & S_IXOTH) ? 'x' : '-';  
}



/// Creates an array with the valid combination of rwx operations
void build_Perms(struct Perms** perms_arr,int length){
    *perms_arr = malloc(length * sizeof(struct Perms));
    if(*perms_arr == NULL) return;

    struct Perms mode1;
    mode1.perm[0] = 'r';
    mode1.perm[1] = '\0';
    mode1.octal_mode = 04;
    (*perms_arr)[0] = mode1;

    struct Perms mode2;
    mode2.perm[0] = 'w';
    mode2.perm[1] = '\0';
    mode2.octal_mode = 02;
    (*perms_arr)[1] = mode2;

    struct Perms mode3;
    mode3.perm[0] = 'x';
    mode3.perm[1] = '\0';
    mode3.octal_mode = 01;
    (*perms_arr)[2] = mode3;

    struct Perms mode4; 
    mode4.perm[0]=  'r';
    mode4.perm[1]=  'w';
    mode4.perm[2]=  '\0';
    mode4.octal_mode = 06;
    (*perms_arr)[3] = mode4;

    struct Perms mode5;
    mode5.perm[0]=  'r';
    mode5.perm[1]=  'x';
    mode5.perm[2]=  '\0';
	mode5.octal_mode = 05;
	(*perms_arr)[4] = mode5;

    struct Perms mode6; 
    mode6.perm[0]=  'r';
    mode6.perm[1]=  'w';
    mode6.perm[2]=  'x';
    mode6.perm[3]=  '\0';
    mode6.octal_mode = 07;
    (*perms_arr)[5] = mode6;

    struct Perms mode7; 
    mode7.perm[0]=  'w';
    mode7.perm[1]=  'x';
    mode7.perm[2]=  '\0';
    mode7.octal_mode = 03;
    (*perms_arr)[6] = mode7;
}

int set_changes_mode_str(char* str, char* file, int oldPerms){
    struct Perms* perms_arr;
    int length = 7;
    build_Perms(&perms_arr,length);
    if(str[0] != 'u' && str[0] != 'g' && str[0] != 'o' && str[0] != 'a' ){
        return -1;
    }
    if(str[1] != '=' && str[1] != '-' && str[1] != '+'){
        return -1; //command is formmatted incorectly
    }

    int perms = -1;
    int index = 2;
    char mode[4];
    while(str[index] != '\0'){
        if(index == 5) return -1; //invalid mode
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
    if(str[0] == 'u') perms = perms << 6;
    else if(str[0] == 'g') perms = perms << 3;
    else if(str[0] == 'a') perms = perms <<6 | perms <<3 | perms;
    if(perms == -1){
        return -1;
    }
    if(str[1] == '+'){
        //add the new permissions to already existant ones
        //lets read the current perms
		perms |= oldPerms;
	}else if(str[1] == '-'){
		perms ^= oldPerms;
    }

	free(perms_arr);
    return perms;
}

int is_valid_mode(char* mode){
    int index = 0;
    while(mode[index] != '\0'){
        switch(mode[index]){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            //nothing happens number is valid
            break;
            default:
                return -1;
        }
        index++;
    }
    return 0;
}

void print_changes_command(int oldPerms,int newPerms,char filename[200]){
    char oldPermsString[9];
	char newPermsString[9];
	getPermsStringFormat(oldPerms, oldPermsString);
    getPermsStringFormat(newPerms, newPermsString);
	printf("mode of '%s' changed from %o (%s) to %o (%s)\n", filename, oldPerms, oldPermsString,newPerms,newPermsString);
}

void print_verbose_retain_command(int oldPerms,char filename[200]){
    char oldPermsString[9];
	getPermsStringFormat(oldPerms, oldPermsString);
	printf("mode of '%s' retained as %o (%s)\n", filename, oldPerms,oldPermsString);
}

int main(int argc, char *argv[] ){
    clock_t begin = clock(); //time at beggining of execution
    if(check_if_env_var_set() == 0){
        create_log_file();
        send_proc_create(begin,argv,argc);
    }
    //send first creation of parent prcoess 
    char filename[200];
    char mode[120];
    strcpy(mode,argv[argc - 2]);
    strcpy(filename,argv[argc - 1]);
    struct stat buffer;
    struct passwd *pwd;
    int status;
    bool verbose = false, changes = false, recursive = false;
    int newPerms;

    //check if file exists
    if( access( filename, F_OK ) != 0 ) {
        printf("xmod: cannot access '%s': No such file or directory\n",filename);
        printf("failed to change mode of '%s' from 0000 (---------) to 0000 (---------)\n",filename);
        send_proc_exit(begin,-1);
        return -1;
    } 

    //check for flags "-v", "-r", "-c"
    // -c - displays only if there were changes in file perms
    // -v - always displays even if nothing changes
    // -R - recursive
    for(int i = 1; i < argc - 2;i++){ //iterate until argc - 2 which specifies mode/octal-mode
        if(strcmp("-v",argv[i]) == 0){
            verbose = true;
        }
        else if(strcmp("-r",argv[i]) == 0){
            recursive = true;
        }
        else if(strcmp("-c",argv[i]) == 0){
            changes = true;
        }
        else{
            send_proc_exit(begin,-1);
            return -1;
        }
    }
    
    //check if file exists
    if( access( filename, F_OK ) != 0 ) {
        printf("File does not exist\n");
        send_proc_exit(begin,-1);
        return -1;
    } 


    int oldPerms = getCurrentPerms(filename);
	
    
	if(atoi(argv[argc - 2]) == 0){  //if atoi fails, then its a mode specified with letters
        newPerms = set_changes_mode_str(mode,filename,oldPerms);
    }
    else{

		if(is_valid_mode(mode) == -1){
            printf("Illegal mode! ");
            send_proc_exit(begin,-1);
            return -1;
        }
        newPerms = strtoll(mode,NULL,8);
        //check if its a valid mode
        
    }
    
    if(newPerms == -1){
        printf("Illegal mode\n");
        send_proc_exit(begin,-1);
        return -1;
    }
    
	if (chmod(filename, newPerms) != 0)
	{
		perror("Chmod failed: ");
        send_proc_exit(begin,-1);
        return -1;
	}
	status = stat(filename,&buffer);
    if (status != 0){
        perror("Stat failed: \n");
        send_proc_exit(begin,-1);
        return -1;
    }

    if(check_if_env_var_set() == 0) send_file_mode_change(begin,oldPerms,newPerms,filename);

    if((changes || verbose) && oldPerms != newPerms){
        print_changes_command(oldPerms,newPerms,filename);
	}
    if(verbose && oldPerms == newPerms){
        print_verbose_retain_command(oldPerms,filename);
    } 
    
    /* Print out owner's name if it is found using getpwuid(). 
    if ((pwd = getpwuid(buffer.st_uid)) != NULL)
        printf(" %-8.8s\n", pwd->pw_name);
    else
        printf(" %-8d\n", buffer.st_uid);   
    */
    send_proc_exit(begin,0);
    return 0;
}
