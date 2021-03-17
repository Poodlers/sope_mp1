#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

extern const char * const sys_siglist[];

int fd;

int check_if_env_var_set(){
    if(getenv("LOG_FILENAME") == NULL){
        return -1;
    }
    return 0;
}

int create_log_file(){

    fd = open(getenv("LOG_FILENAME"),O_TRUNC | O_CREAT | O_WRONLY);
    if(fd == -1){
        perror("Could not open Log File");
    }

}

int close_log_file(){
    if( close(fd) < 0){
        return -1;
    }
    return 0;
}

void upcase(char *s)
{
    while (*s)
    {
        *s = toupper(*s);
        s++;        
    }
}

int get_sig_name(int signal, char output[40]){
    char *str = strdup(sys_siglist[signal]);
    if (!str)
        return -1;

    upcase(str);
    snprintf(output, 40, "SIG%s", str);
    free(str);
    return 0;

}


int get_time_until_now(){
    //read the env_var to check what the parents PID is
    FILE* fd;
    char buff[128];
    time_t boottime;
    char *p;
    struct timeval tv;
    unsigned long uptime;
    

	fd = fopen ("/proc/uptime", "r");
    if (fd != NULL)
    {
       while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }
      

      fclose (fp);
    }

    char begin_str[10];
    sprintf(begin_str, "%d", getenv("PARENT_PID"));
    char proc_file_path[256] = "";
    strcat(proc_file_path,"proc/");
    strcat(proc_file_path,begin_str);
    strcat(proc_file_path,"/stat");  

    char proc_file_buff[500];

    FILE *fp = fopen(proc_file_path, "r");
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }

    if (line)
        free(line);
    fclose(fp);
}

int send_proc_create(clock_t begin, char* args[],int num_args){
    int time_elapsed = get_time_until_now(begin);
    char* output = malloc(sizeof(char) * 50);
    snprintf(output, 50, "%d", time_elapsed);
    write(fd,output,strlen(output));
    write(fd, " ; ",3);
    snprintf(output,50,"%d",getpid());
    write(fd,output,strlen(output));
    write(fd," ; ",3);
    write(fd,"PROC_CREAT",strlen("PROC_CREAT"));
    write(fd, " ; ",3);
    for(int i = 0; i < num_args;i++){
        write(fd,args[i],strlen(args[i]));
        write(fd, " ",1);
    }
    write(fd,"\n",1);
    free(output);

}

int send_proc_exit(clock_t begin,int exit_status){
    int time_elapsed = get_time_until_now(begin);
    char output[50];
    snprintf(output, 50, "%d", time_elapsed);
    write(fd,output,strlen(output));
    write(fd, " ; ",3);
    snprintf(output,50,"%d",getpid());
    write(fd,output,strlen(output));
    write(fd," ; ",3);
    write(fd,"PROC_EXIT",strlen("PROC_EXIT"));
    write(fd, " ; ",3);
    snprintf(output, 50, "%d", exit_status);
    write(fd,output,strlen(output));
    write(fd,"\n",1);
}

int send_signal_recv(clock_t begin,int signal){
    char str[50];
    get_sig_name(signal,str);
    int time_elapsed = get_time_until_now(begin);
    char output[50];
    snprintf(output, 50, "%d", time_elapsed);
    write(fd,output,strlen(output));
    write(fd, " ; ",3);
    snprintf(output,50,"%d",getpid());
    write(fd,output,strlen(output));
    //
    write(fd,"SIGNAL_RECV",strlen("SIGNAL_RECV"));
    write(fd, " ; ",3);
    write(fd,str,strlen(str));
    write(fd,"\n",1);


}

int send_signal_sent(clock_t begin,int signal,pid_t pid){
    char str[40];
    get_sig_name(signal,str);
    int time_elapsed = get_time_until_now(begin);
    char output[50];
    snprintf(output, 50, "%d", time_elapsed);
    write(fd,output,strlen(output));
    write(fd, " ; ",3);
    snprintf(output,50,"%d",getpid());
    write(fd,output,strlen(output));
    write(fd,"SIGNAL_SENT",strlen("SIGNAL_SENT")); 
    write(fd, " ; ",3);
    snprintf(output, 50, "%s : %d", str,pid);
    write(fd,output,strlen(output));
    write(fd,"\n",1);
}

int get_real_file_path(char filename[200],char real_path[200]){
    strcpy(real_path,realpath(filename, NULL)); 
    if (real_path == NULL){
        return -1;        
    }
    return 0;
}

int send_file_mode_change(clock_t begin,int oldPerms, int newPerms,char filename[200]){
    int time_elapsed = get_time_until_now(begin);
    char output[50];
    snprintf(output, 50, "%d", time_elapsed);
    write(fd,output,strlen(output));
    write(fd, " ; ",3);
    snprintf(output,50,"%d",getpid());
    write(fd,output,strlen(output));
    write(fd, " ; ",3);
    write(fd,"FILE_MODF",strlen("FILE_MODF")); 
    write(fd, " ; ",3);
    char real_path[200];
    if(get_real_file_path(filename,real_path) == 0){
        write(fd,real_path,strlen(real_path));

    }else{
        write(fd,filename,strlen(filename));
    }

    snprintf(output,50," : %o : %o", oldPerms,newPerms);
    write(fd,output,strlen(output));
    write(fd,"\n",1);

}