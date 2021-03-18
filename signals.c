#include "signals.h"
#include "logs.h"
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

extern long timeSinceEpochParentStart;
extern bool write_logs;
extern int child_process_index; 
extern int files_processed;
extern int files_modified;
extern char filename[4096];
extern pid_t child_processes[500];

void send_signal(int pid, int signal){
    kill(pid,signal);
}

void signal_handler_default(int signo) {
	if(write_logs) send_signal_recv(timeSinceEpochParentStart,signo);
}

void signal_handler_SIGINT_child(int signo){
    if(write_logs) send_signal_recv(timeSinceEpochParentStart,signo);
    display_info_signal(filename,files_processed,files_modified);
    pause();
}

void signal_handler_SIGINT_parent(int signo){
    char answer = 'b';

    if(write_logs) send_signal_recv(timeSinceEpochParentStart,signo);
    display_info_signal(filename,files_processed,files_modified);
    sleep(1);
    do{
        printf("Do you wish to resume the program? (y/n) ?: ");
        scanf("%c", &answer);
        printf("you answered: %c \n",answer);
        if(answer == 'y' || answer == 'Y'){
            for(int i = 0; i < child_process_index;i++){
                send_signal(child_processes[i],SIGHUP); // para continuar o processo
            }
            break;
        }else if(answer == 'n' || answer == 'N'){

            for(int i = 0; i < child_process_index;i++){
                send_signal(child_processes[i], SIGALRM);  //matar as crianças
            }
            
            if(write_logs) send_proc_exit(timeSinceEpochParentStart,-1);
            exit(-1);
        }
        //limpar buffer
        answer = ' ';
    }while(answer != 'y' && answer != 'Y' && answer != 'n' && answer != 'N');

}


void define_handlers_parent(){
  struct sigaction new, new_sigint, old;
	sigset_t smask;	// defines signals to block while func() is running
	// prepare struct sigaction
	if (sigemptyset(&smask)==-1)	// block no signal
		perror("sigsetfunctions");
	new.sa_handler = signal_handler_default;
	new.sa_mask = smask;
	new.sa_flags = 0;	// usually works

    new_sigint.sa_handler = signal_handler_SIGINT_parent;
	new_sigint.sa_mask = smask;
	new_sigint.sa_flags = 0;	// usually works

	if(sigaction(SIGINT, &new_sigint, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGHUP, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGQUIT, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGBUS, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGSEGV, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGPIPE, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGALRM, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGTERM, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(17, &new, &old) == -1)
		perror("sigaction");
}

void signal_handler_SIGHUP(int signo){ //continue execution
    signal_handler_default(signo);
    for(int i =0; i < child_process_index;i++){
        send_signal(child_processes[i],SIGHUP);
    }
}

void signal_handler_SIGALRM(int signo){ //kill all children of this process as well
    signal_handler_default(signo);
    for(int i =0; i < child_process_index;i++){
        send_signal(child_processes[i],SIGALRM);
    }
    if(write_logs) send_proc_exit(timeSinceEpochParentStart,-1);
    exit(-1);

}


void define_sigint_handler_children(){
    struct sigaction new, new_sigint, new_sighup, new_sigalrm, old;
	sigset_t smask;	// defines signals to block while func() is running
	// prepare struct sigaction
	if (sigemptyset(&smask)==-1)	// block no signal
		perror("sigsetfunctions");
	new.sa_handler = signal_handler_default;
	new.sa_mask = smask;
	new.sa_flags = 0;	// usually works

    new_sigint.sa_handler = signal_handler_SIGINT_child;
	new_sigint.sa_mask = smask;
	new_sigint.sa_flags = 0;	// usually works

    //this will be the signal that kids receive to continue execution
    new_sighup.sa_handler = signal_handler_SIGHUP;
	new_sighup.sa_mask = smask;
	new_sighup.sa_flags = 0;

    //this is will the signal that kids receive that means they should die right there
    new_sigalrm.sa_handler = signal_handler_SIGALRM;
	new_sigalrm.sa_mask = smask;
	new_sigalrm.sa_flags = 0;

	if(sigaction(SIGINT, &new_sigint, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGHUP, &new_sighup, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGQUIT, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGBUS, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGSEGV, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGPIPE, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGALRM, &new_sigalrm, &old) == -1)
		perror("sigaction");
    if(sigaction(SIGTERM, &new, &old) == -1)
		perror("sigaction");
    if(sigaction(17, &new, &old) == -1)
		perror("sigaction");
}
