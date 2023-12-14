#include <stdlib.h>    //free
#include <unistd.h>    //sleep
#include <locale.h>    //setlocale
#include <semaphore.h>
#include <stdio.h>     //printf
#include <sys/stat.h>  //umask, mkdir
#include <errno.h>     //EEXIST
#include <string.h>
#include <pthread.h>

#include "global.h"
#include "lib/log.h"
#include "lib/file.h"

#include "threads.h"

int Start(void) {

	//Set the thread name
	pthread_setname_np(pthread_self(), "main");
	
	//Initialise
	int r = chdir(DATA_FOLDER);// Change the current working directory.
	if (r)
	{
		LogErrno("ERROR making " DATA_FOLDER " the default directory");
	    return 1;
	}
	LogInit(); //Requires DATA_FOLDER for settings - prior to this the log level is 'i'


	setlocale(LC_ALL,"en_GB.UTF-8");
	
	//Save the pid in the data folder
	pid_t pid = getpid();
	char text[10];
	sprintf(text, "%d", pid);
	FileSaveText("pid", text);

	//Start threads
	Log('i', "Starting threads");
	ThreadsStart();
	Log('w', "Application started");
	
	//Wait for all threads to finish - normally never
	ThreadsWait();

	//Exit
	Log('w', "Exit");
	return 0;
}
int daemonise(void) { //returns 0 for the child, +1 for the parent and -1 on error
		pid_t process_id = 0;
		pid_t sid = 0;

		// Create child process
		process_id = fork();
		if (process_id < 0)
		{
			printf("fork failed!\n");
			return -1;
		}

		// We are the parent process and just to need to exit successfully with +1
		if (process_id > 0) return 1;

		//We are the child process
		umask(0);             //unmask the file mode
		sid = setsid();       //set new session
		if(sid < 0) return -1;

		// Close stdin. stdout and stderr
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		return 0;
}
int main()
{
	if (DAEMONISE)
	{
		switch(daemonise())
		{
			case -1: return 1; //Error
			case +1: return 0; //Parent
			case  0: break;    //Child
		}
	}

	//Run the application
	Start();

	return 0;
}
void Stop(void) {
	Log('w', "Stopping threads");
	ThreadsKill();
}
