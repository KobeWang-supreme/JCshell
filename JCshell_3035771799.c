/*
• Student name and No.: Wang Kesheng 3035771799

• Development platform: workbench2 Linux platform

• Remark – Describe how much you have completed

    All the required things are completed:
        - Process creation and execution
        - Process creation and execution – use of ‘|’
        - Print process’s running statistics
        - Use of signals
        - Built-in command: exit
*/



#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
//#include <fcntl.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>

char *stringArray[100];
int arraySize = 0;
volatile sig_atomic_t received = 0;
char error_message[30] = "An error has occurred\n";
char error_message2[30] = "debug error ";
int sig = 0;
char *signal_message ;

// function used to add status string to array
void addStringToArray(const char *str) {
    if (arraySize >= 100) {
        printf("full \n");
        return;
    }
    
    
    stringArray[arraySize] = strdup(str);
    arraySize++;
}


void remove_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
    }
}

void sigusr1_handler(int signum) {
    received = 1;
}

void sigint_handler(int signum) {
    printf("\n## JCshell [%d] ## ", (int) getpid());
    fflush(stdout);
}

void sigint_handler1(int signum) {
    return;
}


// free cmds memory
void freeCmd(char **cmds,int numCmds){
	for(int i=0;i<numCmds;i++){
        free(cmds[i]);
    }
    free(cmds);
}


//check if all spaces
bool checkSpaces(char *str){
	while(*str != 0){
		if(!isspace((unsigned char)*str)){
			return false;
		}
		str++;
	}
	return true;
}

// trim string
void trimString(char *str) {
    int length = strlen(str);
    int start = 0;
    int end = length - 1;

    // Remove leading spaces
    while (str[start] == ' ') {
        start++;
    }

    // Remove trailing spaces
    while (end > start && str[end] == ' ') {
        end--;
    }

    // Shift characters to the beginning
    for (int i = start; i <= end; i++) {
        str[i - start] = str[i];
    }

    // Null-terminate the trimmed string
    str[end - start + 1] = '\0';
}

int countPipeCharacters(const char *str) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|') {
            count++;
        }
    }
    return count;
}

void freeMemory(int argc, char **argv){
    for(int i=0;i<=argc;i++){
        free(argv[i]);
    }
    free(argv);
}

// print process status when using pipes
void printCommandStatus(pid_t mypid, siginfo_t *info) {
    
    char str[50];
	char comm[50]; // command name
	FILE * file;
	char cmd[50], state[50], ppid[50], vctx[50], nvctx[50], excode[50];
    long user, sys;
    unsigned long long int start_t, up_time;
    char firstChar;
    long hertz = sysconf(_SC_CLK_TCK);
    sprintf(str, "/proc/%d/status", (int) mypid); // get proc/status path
    fflush(stdout);
    

	file = fopen(str, "r");
	if (file == NULL) {
		printf("Error in open my proc file\n");
        fflush(stdout);
		exit(0);
	}
	// read file's fields
	if (file) {     
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file)) {
                if (strncmp(buffer, "Name", 4) == 0) {
                    strcpy(cmd, buffer + 6);
                    //fprintf(stdout, "Command name: %s\n", cmd);
                    
                }
                if (strncmp(buffer, "State", 5) == 0) {
                    strcpy(state, buffer + 7);
                    firstChar = state[0];
                    // fprintf(stdout, "State: %c\n", firstChar);    
                    
                }
                if (strncmp(buffer, "PPid", 4) == 0) {
                    strcpy(ppid, buffer + 6);
                    //fprintf(stdout, "PPid: %s\n", ppid);
                    
                }
                if (strncmp(buffer, "voluntary_ctxt_switches", 23) == 0) {
                    strcpy(vctx, buffer + 25);
                    //fprintf(stdout, "voluntary_ctxt_switches: %s\n", vctx);
                    
                }
                if (strncmp(buffer, "nonvoluntary_ctxt_switches", 26) == 0) {
                    strcpy(nvctx, buffer + 28);
                    //fprintf(stdout, "nonvoluntary_ctxt_switches: %s\n", nvctx);
                    
                }
            }
            fclose(file);
    } else {
            perror("fopen");
            exit(1);
    
    }

    char stat_path[100];
    FILE* stat_file;
    sprintf(stat_path, "/proc/%d/stat", mypid); // get proc/stat path
    fflush(stdout);
    stat_file = fopen(stat_path, "r");
    
    if (stat_file == NULL) {
        perror("Error opening stat file");
        exit(1);
    }

    // Read the fields from /proc/{pid}/stat
    if (fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld %*ld %llu", &user, &sys, &start_t) == EOF) {
        perror("Error reading utime from stat file");
        fclose(stat_file);
        exit(1);
    }
    // calculate time
    double kernel_mode_seconds_u = (double) user / hertz;
    double kernel_mode_seconds_s = (double) sys / hertz;
    double start_time_seconds = start_t / hertz;
    fclose(stat_file);

    // get uptime
    FILE* uptime_file;
    uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file == NULL) {
        perror("Error opening uptime file");
        exit(1);
    }
    fscanf(uptime_file, "%llu", &up_time);
    fclose(uptime_file);
    double real_time =  up_time - start_time_seconds;

    remove_newline(cmd);
    remove_newline(state);
    remove_newline(ppid);
    remove_newline(vctx);
    remove_newline(nvctx);
    
    int signal_number = (int) info->si_status;
    const char *signal_message = strsignal(signal_number);
    char buffer[1024];
    if (info->si_code == CLD_EXITED){
        int result = snprintf(buffer, sizeof(buffer), "(PID)%d (CMD)%s (STATE)%c (EXCODE)%d (PPID)%s (USER)%.2f (SYS)%.2f (REAL)%.2f (VCTX)%s (NVCTX)%s \n", mypid, cmd, firstChar, info->si_status, ppid, kernel_mode_seconds_u, kernel_mode_seconds_s, kernel_mode_seconds_u, vctx, nvctx);
        if (result < 0) {
            perror("snprintf");
            exit(1);
        }
        addStringToArray(buffer); // store output string to array
    }
    else{
        int result = snprintf(buffer, sizeof(buffer), "(PID)%d (CMD)%s (STATE)%c (EXSIG)%s (PPID)%s (USER)%.2f (SYS)%.2f (REAL)%.2f (VCTX)%s (NVCTX)%s \n", mypid, cmd, firstChar, signal_message, ppid, kernel_mode_seconds_u, kernel_mode_seconds_s, kernel_mode_seconds_u, vctx, nvctx);
        if (result < 0) {
            perror("snprintf");
            exit(1);
        }
        addStringToArray(buffer); // store output string to array
    }
}

// print process status when not using pipes
void printCommandStatus1(pid_t mypid, siginfo_t *info) {
    
    char str[50];
	char comm[50]; // command name
	FILE * file;
	char cmd[50], state[50], ppid[50], vctx[50], nvctx[50], excode[50];
    long user, sys;
    long long unsigned int start_t;
    double up_time;
    char firstChar;
    long hertz = sysconf(_SC_CLK_TCK);
    sprintf(str, "/proc/%d/status", (int) mypid); // get proc/status path
    fflush(stdout);
    

	file = fopen(str, "r");
	if (file == NULL) {
		printf("Error in open my proc file\n");
        fflush(stdout);
		exit(0);
	}
	// read file's fields
	if (file) {     
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file)) {
                if (strncmp(buffer, "Name", 4) == 0) {
                    strcpy(cmd, buffer + 6);
                    //fprintf(stdout, "Command name: %s\n", cmd);
                    
                }
                if (strncmp(buffer, "State", 5) == 0) {
                    strcpy(state, buffer + 7);
                    firstChar = state[0];
                    // fprintf(stdout, "State: %c\n", firstChar);    
                    
                }
                if (strncmp(buffer, "PPid", 4) == 0) {
                    strcpy(ppid, buffer + 6);
                    //fprintf(stdout, "PPid: %s\n", ppid);
                    
                }
                if (strncmp(buffer, "voluntary_ctxt_switches", 23) == 0) {
                    strcpy(vctx, buffer + 25);
                    //fprintf(stdout, "voluntary_ctxt_switches: %s\n", vctx);
                    
                }
                if (strncmp(buffer, "nonvoluntary_ctxt_switches", 26) == 0) {
                    strcpy(nvctx, buffer + 28);
                    //fprintf(stdout, "nonvoluntary_ctxt_switches: %s\n", nvctx);
                    
                }
            }
            fclose(file);
    } else {
            perror("fopen");
            exit(1);
    
    }

    char stat_path[100];
    FILE* stat_file;
    sprintf(stat_path, "/proc/%d/stat", mypid); // get proc/stat path
    fflush(stdout);
    stat_file = fopen(stat_path, "r");
    
    if (stat_file == NULL) {
        perror("Error opening stat file");
        exit(1);
    }

    
    
    // Read the fields from /proc/{pid}/stat
    if (fscanf(stat_file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld %*ld %llu", &user, &sys, &start_t) == EOF) {
        perror("Error reading utime from stat file");
        fclose(stat_file);
        exit(1);
    }
    

    double kernel_mode_seconds_u = (double) user / hertz;
    double kernel_mode_seconds_s = (double) sys / hertz;
    double start_time_seconds = (double) start_t / hertz;
    fclose(stat_file);
    
    // get uptime
    FILE* uptime_file;
    uptime_file = fopen("/proc/uptime", "r"); 
    if (uptime_file == NULL) {
        perror("Error opening uptime file");
        exit(1);
    }


    fscanf(uptime_file, "%lf ", &up_time);
    fclose(uptime_file);
    //printf("up_time: %lf\n", up_time);
    double real_time =  up_time - start_time_seconds;
    
    
    remove_newline(cmd);
    remove_newline(state);
    remove_newline(ppid);
    remove_newline(vctx);
    remove_newline(nvctx);
    
    int signal_number = (int) info->si_status;
    const char *signal_message = strsignal(signal_number);
    char buffer[1024];
    // print status
    if (info->si_code == CLD_EXITED){
        printf("(PID)%d (CMD)%s (STATE)%c (EXCODE)%d (PPID)%s (USER)%.2f (SYS)%.2f (REAL)%.2f (VCTX)%s (NVCTX)%s \n", mypid, cmd, firstChar, info->si_status, ppid, kernel_mode_seconds_u, kernel_mode_seconds_s, kernel_mode_seconds_u, vctx, nvctx);
        fflush(stdout);
    }
    else{
        printf("(PID)%d (CMD)%s (STATE)%c (EXSIG)%s (PPID)%s (USER)%.2f (SYS)%.2f (REAL)%.2f (VCTX)%s (NVCTX)%s \n", mypid, cmd, firstChar, signal_message, ppid, kernel_mode_seconds_u, kernel_mode_seconds_s, kernel_mode_seconds_u, vctx, nvctx);
        fflush(stdout);
    }
}
// execute command with pipe
void executePipeCmd(int argc, char **argv, int input_fd, int output_fd) {
    siginfo_t info;
    int pid;
    signal(SIGUSR1, sigusr1_handler);
    
    // check if command is empty
    if (strcmp(argv[0],"\0") == 0 ){
        return;
    }
    else if (strcmp(argv[0],"exit") == 0){
      	
        if (argc>1){
            printf("JCshell: \"exit\" with other arguments!!!\n");
            fflush(stdout);
            return;
        }
        printf("JCshell: Terminated\n");
        fflush(stdout);
        exit(0);
    } 
    else if (strcmp(argv[0],"cd") == 0){
        if(argv[1]){ //if directory argument exist
            char *path = argv[1];
            int retcd = chdir(path); // returns 0 on successful execution, else -1
            if ( retcd == -1 )
            {
                write(STDERR_FILENO, error_message, strlen(error_message)); // print the error message and return
            }
            //freeMemory(argc,argv);
        }
        else{
            char *path = getenv("HOME"); // to get the path to the home directory.
            int retcd = chdir(path);    // returns 0 on successful execution, else -1
            if (retcd == -1 )
            {
                write(STDERR_FILENO, error_message, strlen(error_message)); //print the error message and return
            }
            //freeMemory(argc,argv);
        }
    }
    else if (strcmp(argv[0],"pwd") == 0){
    	if(argc>1){
    		write(STDERR_FILENO, error_message, strlen(error_message)); //print the error message and return
    	}
    	else{
    		char filePath[1024];
        	getcwd(filePath,sizeof(filePath));
        	printf("%s\n",filePath);
            fflush(stdout);
    	}
    	//freeMemory(argc,argv);
    }
    else{       
       	pid_t pid = fork();
        
    
        if (pid < 0) {
            // Fork failed
            perror("fork");
            //return -1;

        } else if (pid == 0) {
        // Child process
        
            while (!received) {
                pause(); 
            }

            
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (output_fd != STDOUT_FILENO) {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }


            int ret = execvp(argv[0], argv);
            if (ret == -1) {
            
                printf("JCshell: '%s': No such file or directory\n", argv[0]);
                fflush(stdout);
                // printf("%s: Command not found\n", argv[0]);
                exit(EXIT_FAILURE);
            
            }
        } else {
            // Parent process
            int status;
        
            kill(pid, SIGUSR1); // send signal to child process

            waitid(P_PID, pid, &info, WNOWAIT | WEXITED); // wait chiled process to finish

            if (info.si_status != 1){
                printCommandStatus(pid, &info); // print status
            }
        
            waitpid(pid, &status, 0);
        
            if (WIFEXITED(status)) {
                int exitStatus = WEXITSTATUS(status);
                // printf("Child process exited normally with status code: %d\n", exitStatus);
            
            } else if (WIFSIGNALED(status)) {
                int term_signal = WTERMSIG(status);
                // printf("Child process terminated by signal: %s\n", strsignal(term_signal));
                // fflush(stdout);
            
            } else {
                printf("Child process did not terminate normally.\n");
                fflush(stdout);
            
            }
    }

    }
}       

// execute command without pipe
void executeCmd(int argc, char **argv){
    
    siginfo_t info;
    int pid;
    signal(SIGUSR1, sigusr1_handler);
    // printf("argc: %d\n",argc);
    
    if (strcmp(argv[0],"\0") == 0 ){
        return;
    }
    else if (strcmp(argv[0],"exit") == 0){
      	
        if (argc>1){
            printf("JCshell: \"exit\" with other arguments!!!\n");
            fflush(stdout);
            return;
        }
        printf("JCshell: Terminated\n");
        fflush(stdout);
        exit(0);
    } 
    else if (strcmp(argv[0],"cd") == 0){
        if(argv[1]){ //if directory argument exist
            char *path = argv[1];
            int retcd = chdir(path); // returns 0 on successful execution, else -1
            if ( retcd == -1 )
            {
                write(STDERR_FILENO, error_message, strlen(error_message)); // print the error message and return
            }
            //freeMemory(argc,argv);
        }
        else{
            char *path = getenv("HOME"); // to get the path to the home directory.
            int retcd = chdir(path);    // returns 0 on successful execution, else -1
            if (retcd == -1 )
            {
                write(STDERR_FILENO, error_message, strlen(error_message)); //print the error message and return
            }
            //freeMemory(argc,argv);
        }
    }
    else if (strcmp(argv[0],"pwd") == 0){
    	if(argc>1){
    		write(STDERR_FILENO, error_message, strlen(error_message)); //print the error message and return
    	}
    	else{
    		char filePath[1024];
        	getcwd(filePath,sizeof(filePath));
        	printf("%s\n",filePath);
            fflush(stdout);
    	}
    	//freeMemory(argc,argv);
    }
    else{       
       	pid_t pid = fork();
        
    
    if (pid < 0) {
        // Fork failed
        perror("fork");
        //return -1;

    } else if (pid == 0) {
        // Child process
        
        while (!received) {
            pause(); 
        }
        int ret = execvp(argv[0], argv);
        if (ret == -1) {
            
            printf("JCshell: '%s': No such file or directory\n", argv[0]);
            fflush(stdout);
            // printf("%s: Command not found\n", argv[0]);
            exit(EXIT_FAILURE);
            
        }
    } else {
        // Parent process
        int status;
        
        kill(pid, SIGUSR1); // send signal to child process

        waitid(P_PID, pid, &info, WNOWAIT | WEXITED); // wait chiled process to finish

        if (info.si_status != 1){
            printCommandStatus1(pid, &info); // print status
        }
        
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            // printf("Child process exited normally with status code: %d\n", exitStatus);
            
        } else if (WIFSIGNALED(status)) {
            int term_signal = WTERMSIG(status);
            //printf("Child process terminated by signal: %s\n", strsignal(term_signal));
            //fflush(stdout);
            
        } else {
            printf("Child process did not terminate normally.\n");
            fflush(stdout);
            
        }
    }

    }
}
// process inputline with pipe  -- second step      
void executePip(char **cmds,int numCmds){
    //remove '\n' from cmds
    for(int i=0;i<numCmds;i++){
        remove_newline(cmds[i]);
    }

    char *argv[numCmds][100];
    char command[numCmds][100];  // Assuming the command won't exceed 100 characters
    char arguments_1[numCmds][100]; 
    int num[numCmds];
    for (int i = 0; i < numCmds; i++){
        // get command and arguments
        sscanf(cmds[i], "%s %[^\n]", command[i], arguments_1[i]);
        
        char *token;
        token = strtok(cmds[i], " ");
        int index = 0;
        // get arguments
        while (token != NULL) {
            int len = strlen(token);
            if (token[len - 1] == '\n') {
                token[len - 1] = '\0';
            }
            argv[i][index] = malloc(strlen(token) + 1);
            strcpy(argv[i][index], token);
            index++;
            token = strtok(NULL, " ");
        }
        argv[i][index] = token;
        num[i] = index;
    }
    // prepare for pipe
    int fd[2], input_fd = STDIN_FILENO;
    for (int i = 0; i < numCmds - 1; i++) {
        if (pipe(fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        executePipeCmd(num[i], argv[i], input_fd, fd[1]); // execute command with pipe

        close(fd[1]);
        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }

        input_fd = fd[0];
    }

    executePipeCmd(num[numCmds - 1], argv[numCmds - 1], input_fd, STDOUT_FILENO); // execute command with pipe

    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }

    // wait for all child process to finish
    for (int i = 0; i < numCmds; i++) {
        wait(NULL);
    }
    // print status
    for (int i = 0; i < arraySize; i++) {
        printf("%s", stringArray[i]);
    }
    // free memory
    for (int i = 0; i < arraySize; i++) {
        free(stringArray[i]);
    }
    arraySize = 0;

    return;
}
// process inputline with pipe  -- first step
void processPipeCmd(char *line){
    //printf("now in process %s in processPipeCmd\n", line);
    //fflush(stdout);
	char **cmds = NULL;
    char *rest;
    char *token;
    char **argv1 = NULL,**argv2 = NULL;
    int argc1=0,argc2=0;
    int numCmds=0;
    int i=0;

    cmds = (char **)realloc(cmds, sizeof(char *) * 5); // remember to free this
    int pipe_count = countPipeCharacters(line); // count pipe characters

    trimString(line); // trim string
    
    if (line[0] == '|' || line[strlen(line) - 1] == '|') {
        printf("Error: Invalid input\n");
        fflush(stdout);
        return;
    }
   
    token = strtok(line, "|");
    while (token && numCmds < 5) {
        trimString(token);
        
        cmds[numCmds++] = token;
        token = strtok(NULL, "|");
    }
    // check if there are two pipe characters without command in between
    if (pipe_count !=  numCmds-1){
        printf("JCshell: should not have two | symbols without in-between command\n");
        fflush(stdout);
        return;
    }

    for(int j=0;j<numCmds;j++){
       if (cmds[j][0] == '\0') {
          printf("JCshell: should not have two | symbols without in-between command\n");
          fflush(stdout);
          return;
      }
  		// printf("cmds[%d]: '%s'\n",j, cmds[j]);
        // fflush(stdout);
  	}
    executePip(cmds,numCmds); // second step
    
}
// process inputline without pipe
void processNormalCmd(char *inputLine){
    char command[100];  // Assuming the command won't exceed 100 characters
    char arguments_1[100]; 
    char* arguments[100];

    // get command and arguments
    sscanf(inputLine, "%s %[^\n]", command, arguments_1);
    
    // free(arguments_1);
    char *token;
    token = strtok(inputLine, " ");
    
    
    
    int index = 0;
    
    while (token != NULL) {
        int len = strlen(token);
        if (token[len - 1] == '\n') {
            token[len - 1] = '\0';
        }
        arguments[index] = malloc(strlen(token) + 1);
        strcpy(arguments[index], token);
        
        index++;
        token = strtok(NULL, " ");
    }
    
    arguments[index] = token;
    
    //  execute command
    
    signal(SIGINT, sigint_handler1); // update signal handler
        
    
    executeCmd(index, arguments);
}

int main(){

    char inputLine[1024];  // Assuming the input line won't exceed 200 characters

    while(true){
        char *stringArray[100];
        if (signal(SIGINT, sigint_handler) == SIG_ERR) {
            perror("signal");
            return 1;
        }
        // Read the input line from the user
        printf("## JCshell [%d] ## ", (int) getpid());
        fflush(stdout);
    
        fgets(inputLine, sizeof(inputLine), stdin);

        // check if inputline has pipe character
        if(countPipeCharacters(inputLine) > 0){
            processPipeCmd(inputLine);
            continue;
        }
        processNormalCmd(inputLine);      
    }

    return 0;
}