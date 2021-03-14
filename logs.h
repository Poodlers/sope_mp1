#include <time.h>
#ifndef LOGS_H
#define LOGS_H

int check_if_env_var_set();

int create_log_file();

int send_proc_create(clock_t begin,char* args[],int num_args);

int close_log_file();

int send_proc_exit(clock_t begin, int exit_status);

int get_time_until_now(clock_t begin);

int send_signal_recv(clock_t begin,int signal);

int get_sig_name(int signal, char output[100]);

int send_signal_sent(clock_t begin,int signal,pid_t pid);

int send_file_mode_change(clock_t begin,int oldPerms, int newPerms, char filename[200]);

int get_real_file_path(char filename[200],char realpath[200]);

#endif