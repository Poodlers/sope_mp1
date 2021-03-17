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

double get_double_from_str(char* str,int desired_ind){
  char * pch;
  pch = strtok(str," ");
  int index = 0;
  while (pch != NULL)
  {
    if(index == desired_ind) {
        return atof(pch);
    }
    pch = strtok(NULL, " ");
    index++;
  }

}

long get_long_from_str(char* str,int desired_ind){
  char * pch;
  pch = strtok(str," ");
  int index = 0;
  while (pch != NULL)
  {
    if(index == desired_ind) {
        return atol(pch);
    }
    pch = strtok(NULL, " ");
    index++;
  }

}


long get_time_until_now(long procTimeSinceBoot){
    //read the env_var to check what the parents PID is
    FILE* fd;
    char buff[128];
    time_t boottime;
    char *p;
    struct timeval tv;
    double uptime;
    long long msecs_time_now;
    long long boot_time_msecs;
    char* line = NULL;
    size_t len = 32;
    size_t read;
    line = (char *) malloc(len * sizeof(char));

    fd = fopen("/proc/uptime", "r");
    while ((read = getline(&line, &len, fd)) != -1)
	{
		uptime = get_double_from_str(line,0);
        gettimeofday(&tv,NULL);
        msecs_time_now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}
    fclose(fd);
    uptime = (uptime * 1000);
    long uptime_long = (long)(uptime);
    boot_time_msecs = msecs_time_now - uptime_long;

    printf("uptime_linux - procTimeParent: %ld \n", uptime_long - procTimeSinceBoot);

    //milliseconds since linux started running

    //get process_time now
    gettimeofday(&tv,NULL);
    msecs_time_now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000); //how many milliseconds has it been since Epoch

    long long process_time =  msecs_time_now - boot_time_msecs - procTimeSinceBoot;

    //printf("Total process time at this time has been %lld milliseconds \n",process_time);

	if(line)
        free(line);

	
	return process_time;
}

int send_proc_create(long procTimeBegin, char* args[],int num_args){
    long time_elapsed = get_time_until_now(procTimeBegin);
    char* output = malloc(sizeof(char) * 50);
    snprintf(output, 50, "%ld", time_elapsed);
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

int send_proc_exit(long procTimeBegin,int exit_status){
    long time_elapsed = get_time_until_now(procTimeBegin);
    char output[50];
    snprintf(output, 50, "%ld", time_elapsed);
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

int send_signal_recv(long procTimeBegin,int signal){
    char str[50];
    get_sig_name(signal,str);
    long time_elapsed = get_time_until_now(procTimeBegin);
    char output[50];
    snprintf(output, 50, "%ld", time_elapsed);
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

int send_signal_sent(long procTimeBegin,int signal,pid_t pid){
    char str[40];
    get_sig_name(signal,str);
    long time_elapsed = get_time_until_now(procTimeBegin);
    char output[50];
    snprintf(output, 50, "%ld", time_elapsed);
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

int send_file_mode_change(long procTimeBegin,int oldPerms, int newPerms,char filename[200]){
    long time_elapsed = get_time_until_now(procTimeBegin);
    char output[50];
    snprintf(output, 50, "%ld", time_elapsed);
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