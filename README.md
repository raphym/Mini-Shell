#Mini shell:

The program is a mini shell we use the commands of the execvp , cd , pipes and
redirections

##global variable
- int run=0; //boolean which is running the program
- int status=0; //the status of the progam

##Details

The main start, catch the username and the hostname and if there are errors exit the program.
After this, enter into an endless loop which receive the input of the user,
send to all the function to work with the input and return to the loop until the command is exit
(there is Boolean ‘run=0’ if I want to stay and ‘run=1’ if I want to exit the program).


Into the loop: 
- Verify if the Boolean run is 0 (if not exit the program)
- Print username and hostname
- Go to the function read token


If there is something in read token so begin to check the arguments
-- if there is a $ in the beginning of an argument so print a message and return to the main.


--If the command is cd :
- Verify if the argument is well written, change the path
directory, free the memory and return to the loop of the main.
- If there are errors of writing so free the memory, print a message and return to the main.


--If the command is not cd :
- Create a process or more and execute the command by the
soon and the father will wait ( only after the father will free the memory).
- If there are errors so free the memory , print a message and exit the program.


--If there is a redirection so go to the function redirection.


--If there is a pipe so go to the function of the pipe and after verify if there are
others signs.


--If there is not signs or pipe execute the command.


--If there is a pipe so divise the tokens first command , second command .
Two process ,one for firstCommand and the second for the secondcommand

--If there is only sign so create process and execute using the signs

