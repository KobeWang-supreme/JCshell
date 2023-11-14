# JCshell

The program accepts a single command or a job that consists of a sequence of commands linked
together with pipes(|) and executes the corresponding command(s) with the given argumentlist(s).
2. The program can locate and execute any valid program (i.e., compiled programs) by giving an
absolute path (starting with /) or a relative path (starting with ./ or ../) or by searching directories
under the $PATH environment variable.
3. The program should be terminated by the built-in exit command but it cannot be terminated by
the Cltr-c key or the SIGINT signal.
4. After the submitted command/job terminated, the program prints the running statistics of all
terminated command(s) and waits for the next command/job from the user.


Like a traditional shell program, when the JCshell process is ready to accept input, it displays a
prompt and waits for input from the user. The prompt should consist of the process ID of the JCshell.

    ## JCshell [60] ##


After accepting a line of input from the user, it parses the input line to extract the command name(s)
and the associated argument list(s), and then creates the child process(es) to execute the command(s).
We can assume that the command line is upper bound by 1024 characters with 30 strings at maximum
(including the | symbols).
When the child process(es) is/are running, JCshell should wait for the child process(es) to terminate
before displaying the prompt for accepting the next input from the user.


When executing a job (with multiple commands), JCshell wait for all processes to terminate
before displaying the statistics. The order of the output should reflect the termination order of those
child processes. Here is an example with three commands joined by two pipes:

    ## JCshell [261] ## cat cpu-mechanisms.txt | grep trap | wc -w
456

(PID)305 (CMD)cat (STATE)Z (EXCODE)0 (PPID)261 (USER)0.00 (SYS)0.00 (VCTX)11
(NVCTX)0

(PID)306 (CMD)grep (STATE)Z (EXCODE)0 (PPID)261 (USER)0.00 (SYS)0.00 (VCTX)4
(NVCTX)0

(PID)307 (CMD)wc (STATE)Z (EXCODE)0 (PPID)261 (USER)0.00 (SYS)0.00 (VCTX)4
(NVCTX)0
