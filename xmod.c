#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "logs.h"
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdbool.h>

long procTimeSinceBoot;
bool write_logs;

struct Perms{
    char perm[4];
    int octal_mode;
};

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

int process_file(char* filename, char* mode, bool verbose, bool changes){
    int oldPerms = getCurrentPerms(filename);
    int newPerms;
    if(atoi(mode) == 0){  //if atoi fails, then its a mode specified with letters
        newPerms = set_changes_mode_str(mode,filename,oldPerms);
    }
    else{
        if(is_valid_mode(mode) == -1){
            printf("Illegal mode! ");
            if(write_logs) send_proc_exit(procTimeSinceBoot,-1);
            return -1;
    }
    newPerms = strtoll(mode,NULL,8);
    //check if its a valid mode
    if(newPerms == -1){
        printf("Illegal mode\n");
        if(write_logs) send_proc_exit(procTimeSinceBoot,-1);
        return -1;
        }
    }
    if (chmod(filename, newPerms) != 0){
        perror("Chmod failed: ");
        if(write_logs) send_proc_exit(procTimeSinceBoot,-1);
        return -1;
    }

    if(write_logs && oldPerms != newPerms) send_file_mode_change(procTimeSinceBoot,oldPerms,newPerms,filename);

    if((changes || verbose) && oldPerms != newPerms){
        print_changes_command(oldPerms,newPerms,filename);
	}
    if(verbose && oldPerms == newPerms){
        print_verbose_retain_command(oldPerms,filename);
    }
}



void sigint_handler(int signo) {
	if(write_logs) send_signal_recv(procTimeSinceBoot,signo);
}

void define_sigint_handler(){
    struct sigaction new, old;
	sigset_t smask;	// defines signals to block while func() is running

	// prepare struct sigaction
	if (sigemptyset(&smask)==-1)	// block no signal
		perror("sigsetfunctions");
	new.sa_handler = sigint_handler;
	new.sa_mask = smask;
	new.sa_flags = 0;	// usually works

	if(sigaction(SIGINT, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGINT, &new, &old) == -1)
		perror("sigaction");
}

int search_dir(char* dir,char* mode,bool verbose,bool changes){
     struct dirent *de;  // Pointer for directory entry     
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir(dir); 
    printf("running search_dir in dir: %s \n", dir);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return -1; 
    } 
  
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    long int res;

    while ((de = readdir(dr)) != NULL){
        if(de->d_type == DT_DIR){ //if path is directory
              continue;
        }
        else if(de->d_type == DT_REG){
            char * filename = malloc(260 * sizeof(char));
            strcat(filename,dir);
            strcat(filename,"/");
            strcat(filename,de->d_name);
            process_file(filename,mode, verbose,changes);
              
            free(filename);  
        }   
    }
    return 0; 
}

int search_dir_recursive(char* args[],int arg_num, bool verbose, bool changes){
    struct dirent *de;  // Pointer for directory entry     
    // opendir() returns a pointer of DIR type.  
    char* dir = args[arg_num -1];
    DIR *dr = opendir(dir); 
    printf("running search_dir in dir: %s \n", dir);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return -1; 
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
                            char* curr_dir = malloc(260 * sizeof(char));
                            strcat(curr_dir,dir);
                            strcat(curr_dir,"/");
                            strcat(curr_dir,de->d_name);
                            char* args_to_exec[arg_num];
                            for(int i = 1; i < arg_num;i++){
                                args_to_exec[i - 1] = args[i];
                            }
                            args_to_exec[arg_num - 2] = curr_dir;
                            args_to_exec[arg_num - 1] = NULL;
                            
                            printf("\n");
                            printf("child process with pid %d exploring dir %s \n", getpid(),de->d_name);
                            
                            execvp(args[0],args_to_exec);
                            free(curr_dir);
                            
                            return 0;
                        }
						
                        break;
                    default: //parent process
                        if(write_logs) send_proc_create(procTimeSinceBoot,args,arg_num);
                        printf("parent process with pid %d exploring dir %s \n", getpid(), dir);
						break;
				}
            }    
        }
        else if(de->d_type == DT_REG){
            char * filename = malloc(260 * sizeof(char));
            strcat(filename,dir);
            strcat(filename,"/");
            strcat(filename,de->d_name);
            
            process_file(filename,args[arg_num - 2], verbose,changes);
              
           free(filename);  
        }   
    }
    return 0; 
}





//returns 1 if the file is a directory and 2 if it is a regular file
int is_regular_file(const char *path)
{
    struct stat path_stat;
    if(stat(path, &path_stat) == -1){
        return -1;
    };
    if(S_ISDIR(path_stat.st_mode)){
        return 1;
    };
    if(S_ISREG(path_stat.st_mode)){
        return 2;
    }
}



long getProcTimeSinceBoot(){
    char begin_str[10];
    sprintf(begin_str, "%d", getpid());
    char proc_file_path[256] = "";
    strcat(proc_file_path,"/proc/");
    strcat(proc_file_path,begin_str);
    strcat(proc_file_path,"/stat");  
    FILE *fp = fopen(proc_file_path, "r");

    char* line = NULL;
    size_t len = 32;
    size_t read;
    line = (char *) malloc(len * sizeof(char));

    long procTimeSinceBoot;
    while ((read = getline(&line, &len, fp)) != -1)
	{
        procTimeSinceBoot = get_long_from_str(line,21);
	}
    procTimeSinceBoot = (long)((double)procTimeSinceBoot / sysconf(_SC_CLK_TCK) * 1000 );
    return procTimeSinceBoot;
}


int main(int argc, char *argv[]){
    write_logs = false;
    
    if(check_if_env_var_set() == 0){
        write_logs = true;
    }

	if(getenv("PARENT_TIME") == NULL){
        //saving the inicial processes procTimeSinceLinuxBoot
        procTimeSinceBoot = getProcTimeSinceBoot();
        char begin_str[20];
        sprintf(begin_str, "%ld", procTimeSinceBoot);
        char env_var_set_str[256] = "";
        strcat(env_var_set_str,"PARENT_TIME=");
        strcat(env_var_set_str,begin_str);
		putenv(env_var_set_str);

        if (write_logs){
            create_log_file();
        }
	}else{
        procTimeSinceBoot = atol(getenv("PARENT_TIME"));
    } 
    if (write_logs){
        send_proc_create(procTimeSinceBoot,argv,argc);
    }
    
	define_sigint_handler();
    
    char filename[200];
    char mode[120];

    strcpy(mode,argv[argc - 2]);
    strcpy(filename,argv[argc - 1]);

    struct stat buffer;
    struct passwd *pwd;
    bool verbose = false, changes = false, recursive = false;
    int newPerms;
	int searchOptions;

    pid_t wpid;
    int status = 0;


	//check for flags "-v", "-r", "-c"    //check for flags "-v", "-r", "-c"
    // -c - displays only if there were changes in file perms
    // -v - always displays even if nothing changes
    // -r - recursive
    for(int i = 1; i < argc - 2; i++){ //iterate until argc - 2 which specifies mode/octal-mode
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
            while ((wpid = wait(&status)) > 0);

            if(write_logs) send_proc_exit(procTimeSinceBoot,-1);
            return -1;
        }
    }
    
    //check if file exists
    if(is_regular_file(filename) == -1) { //whatever we typed in does not exist
        printf("xmod: cannot access '%s': No such file or directory\n",filename);
        printf("failed to change mode of '%s' from 0000 (---------) to 0000 (---------)\n",filename);
        while ((wpid = wait(&status)) > 0);

        if(write_logs) send_proc_exit(procTimeSinceBoot,-1);
        return -1;
    }
    else if(is_regular_file(filename) == 1){ //it is a directory
       if(recursive){
			if(search_dir_recursive(argv,argc,verbose,changes) != 0){
                while ((wpid = wait(&status)) > 0);  
                if(write_logs) send_proc_exit(procTimeSinceBoot,-1);
                return -1;
            }
        }
        else{
            search_dir(filename,mode,verbose,changes);
        }
    }
    else if(is_regular_file(filename) == 2){ //is a file
        printf("We are in presence of a file \n");
        process_file(filename,mode,verbose,changes);
    }
    
    //tell parent to wait for children
    while ((wpid = wait(&status)) > 0);
    
	if(write_logs) send_proc_exit(procTimeSinceBoot,0);
    return 0;
}
