#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

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


int get_until_now(int parent){
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
    long long uptime_long = (long long)(uptime);
    boot_time_msecs = msecs_time_now - uptime_long;

    printf("Milliseconds since January 1st that linux started running: %lld \n", boot_time_msecs);

	char begin_str[10];
    sprintf(begin_str, "%d", parent);
    char proc_file_path[256] = "";
    strcat(proc_file_path,"/proc/");
    strcat(proc_file_path,begin_str);
    strcat(proc_file_path,"/stat");  
    printf("Oi %s\n", proc_file_path);

	FILE *fp = fopen(proc_file_path, "r");
    long procTimeSinceBoot;

    while ((read = getline(&line, &len, fp)) != -1)
	{
	
        procTimeSinceBoot = get_long_from_str(line,21);
	}
    printf("%ld \n", procTimeSinceBoot);
    procTimeSinceBoot = (long)((double)procTimeSinceBoot / sysconf(_SC_CLK_TCK) * 1000 );
    
    //milliseconds since linux started running
    

    //get process_time now
    gettimeofday(&tv,NULL);
    msecs_time_now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000); //how many milliseconds has it been since Epoch

    
    long long process_time =  msecs_time_now - boot_time_msecs - procTimeSinceBoot;

    printf("Total process time at this time has been %lld milliseconds \n",process_time);

	if(line)
        free(line);

    fclose(fp);
	
	return 0;

}

int main( int argc, char *argv[] ) 
{
    sleep(2);
    get_until_now(getpid());
    printf("Called by %d \n",getpid());
    int id = fork();
    switch(id){
        case 0: //child
            sleep(2);
            printf("Called by %d \n",getpid() - 1);
            get_until_now(getpid() - 1);
            break;
        default:
            printf("In the parent %d \n", getpid());
            break;
    }
    return 0;

} 