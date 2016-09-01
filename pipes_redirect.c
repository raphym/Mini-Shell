//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

//declaration of the functions
char** readTokens(FILE* stream);
void freeTokens(char** tokens);
void sigHandler(int signum);

//declaration of the private functions
int verifyDollar(char** tokens);
char** diviseLeft(char** tokens ,int location);
char** diviseRight(char** tokens, int location);
void redirection(char** tokens,int location);
void fPipe(char** tokens,int location);
int test(char** tok);
int tLocation(char** tok);
//declaration of the global variable
int run=0; //boolean which is running the program
int status=0;

//////////////////////////////////////////////////////////////////////////////////////////
								//main of the program
//////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	signal(SIGINT,sigHandler);//to catch the signals

	//To get the login and the hostName of the machine
	char hostName[1024];
	char *login;

	login=getlogin();
	gethostname(hostName,1024);

	//to verify if the login and the host name are valid
	if(login == 0)//verify login
	{
		printf("ERROR We cannot identified your login \n");
		exit(-1);//exit the main with error
	}
	if(hostName==0)//verify host name
	{
		printf("ERROR We cannot identified your host name \n");
		exit(-1);//exit the main with error
	}
	
		int code=0;
		int flag=0;

	//loop endless until the command exit
	while(run==0)//running
	{
		code=status;
		if(code==256)
			code=255;

		//Variables
		char **tok=NULL;
		int flag = 0;// flag to the $
		int error=0;// flag to chdir
		
		printf("%d %s@%s$ ",code,login,hostName);//print login and hostname
	
		//Go to Read token		
		tok=readTokens(stdin);//Send the stream stdin to the function readTokens

		if(tok!=NULL)//if the tok is not NULL
		{
			flag = verifyDollar(tok);//Function to verify if there is a Dollar


			if(flag==0)//There is not a dollar in the begining of every argument
			{
				if(strcmp(tok[0],"cd")==0 )// if the command is "CD"
				{
					if(tok[1]!=NULL  && tok[2]==NULL)//if the cmd cd is well written
					{
						error=chdir(tok[1]);
						if(error==0)//if the function worked
						{
							freeTokens(tok);//free the malloc
						}
						if(error!=0)//error to execute the cd command
						{
							printf("The folder is not exist\n");
							freeTokens(tok);// go to the function wich will free the token tok
						}
					}
					else //the cmd cd is not well written
					{
						printf("INCOMPATIBLE COMMAND OF 'cd' \n");
						freeTokens(tok);// go to the function wich will free the token tok
					}	
				}//End of CD

				else //NOT CD COMMAND
				{
					int i=0;
					int flag2=0;

					while(tok[i]!=NULL)
					{
							if(strcmp(tok[i],"|")==0)
							{
								flag2=1;
								fPipe(tok,i);
								break;
							}
						i++;
					}
					
				
					i=0;
					while(flag2==0 && tok[i]!=NULL)
					{	
						if(strcmp(tok[i],">")==0 || strcmp(tok[i],">>")==0)
							{
								flag2=1;
								redirection(tok,i);
								break;
							}
				
						else if(flag2==0 && strcmp(tok[i],"<")==0)
							{
								flag2=1;
								redirection(tok,i);
								break;
							}
						i++;
					}
			
					if(flag2==0)// if not siman or pipe
						{

							pid_t process=fork();//process
	
							if(process<0)
							{
								fprintf(stderr , "ERROR TO Create a process \n");
								run=1;
								exit(1);
							}
							if (process==0)
							{
								execvp(tok[0],tok);
								fprintf(stderr,"Your command is not valide \n");
								freeTokens(tok);
								exit(1);
							}
							if(process>0)
							{
								wait(&status);
								freeTokens(tok);
							}			
						}
				}//End of NOT CD COMMAND
			}//End if not dollar
		}//End of tok NOT NULL
	}//End of the loop of the main 		
}//End of the main

//////////////////////////////////////////////////////////////////////////////////////////
								//Function FPIPE for the tokens with pipe
//////////////////////////////////////////////////////////////////////////////////////////

void fPipe(char** tok,int location)
{
	char** firstCmd=NULL;
	char** secondCmd=NULL;
	pid_t soon1;
	pid_t soon2;

	
    	firstCmd=diviseLeft(tok,location);//first command
		secondCmd=diviseRight(tok,location);//second command
		freeTokens(tok);// free the token
	
		//Creation of the pipe
		int pipe_descs[2];
		if (pipe(pipe_descs) == -1)
		{
			perror("cannot open pipe\n");
			exit(EXIT_FAILURE) ;
		}
		//SOON1
		soon1=fork();
		if(soon1 == -1)
		{
			printf("ERROR TO CREATE THE SOON\n");
			exit(1);
		}
		if(soon1==0)//the soon1
		{	
			int t=-1;
			t=test(firstCmd);
			close(pipe_descs[0]);
			dup2(pipe_descs[1],STDOUT_FILENO);
						
			if(t==1)
			{
				int fd=5;
				int k = tLocation(firstCmd);
				char path[2048];
				char *addToPath;//to add the input to the path
				getcwd(path,sizeof(path));//to obtain the path
				strcat(path,"/");//to add '/' to the path
				addToPath=firstCmd[k+1];
				strcat(path,addToPath);//to add the name of the new file from the user input
		
				fd = open(path, O_RDONLY |S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);
				
				if(fd==-1)
					{
						 fprintf(stderr,"Your file doesn't exist\n");
						 exit(1);	
					}

					dup2(fd,STDIN_FILENO);
					firstCmd[k]=NULL;
					execvp(firstCmd[0],firstCmd);
					printf("ERROR ! Wrong command\n");
					exit(1);			
			}
			else
				{
					execvp(firstCmd[0],firstCmd);
					printf("ERROR ! Wrong command\n");
					exit(1);
				}
		}

		if(soon1 != 0)
		{	
			soon2=fork();
			if(soon2 == -1)
			{
				printf("ERROR TO CREATE THE SOON\n");
				exit(1);
			}			
		}	

		if(soon2==0)//the soon2
		{	
			int t=-1;
			t=test(secondCmd);
			close(pipe_descs[1]);
			dup2(pipe_descs[0],STDIN_FILENO);
			
			if(t==2)
			{
				int fd=5;
				char path[2048];
				int k = tLocation(secondCmd);
				
	
				char *addToPath;//to add the input to the path
				getcwd(path,sizeof(path));//to obtain the path
				strcat(path,"/");//to add '/' to the path
				addToPath=secondCmd[k+1];
				strcat(path,addToPath);//to add the name of the new file
	
				
					if(strcmp(secondCmd[k],">")==0) // >
						{
							fd = open(path,O_WRONLY|O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);
						}
					else
						{ //>>
							fd = open(path, O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);
						}
	
						
					if(fd==-1)//If there is an error to create the file
						{
							fprintf(stderr , "ERROR TO CREATE THE NEW FILE\n");
							exit(1);
						}
						else 
						{
							dup2(fd,STDOUT_FILENO);//change the output of the command to be in the file and not in the screen.
							secondCmd[k]=NULL;
							execvp(secondCmd[0],secondCmd);
							printf("ERROR ! Wrong command\n");
							exit(1);
						}
		    }
			else 
				{
					execvp(secondCmd[0],secondCmd);
					printf("ERROR ! Wrong command\n");
					exit(1);
				}		
		}
			close(pipe_descs[1]);
			close(pipe_descs[0]);
			waitpid(soon1,&status,0);
			waitpid(soon2,&status,0);	

		if(soon1!=0 && soon2!=0)
			{
			 	freeTokens(firstCmd);
				freeTokens(secondCmd);
			}

}

//////////////////////////////////////////////////////////////////////////////////////////
								//Function TLocation: to find the location of the sign
//////////////////////////////////////////////////////////////////////////////////////////
int tLocation(char** tok)
{
	int i=0;
	while(tok[i]!=NULL)
	{
		if(strcmp(tok[i],"<")==0 ||strcmp(tok[i],">")==0 || strcmp(tok[i],">>")==0 )
			{
				return i;
				break;
			}
		i++;
	}
return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
								//Function Test // to verify if there is a sign
//////////////////////////////////////////////////////////////////////////////////////////
int test(char** tok)
{
	int i=0;
	while(tok[i]!=NULL)
	{
		if(strcmp(tok[i],"<")==0)
			{	
			  return 1;				
			}
		if(strcmp(tok[i],">")==0 || strcmp(tok[i],">>")==0 )
			{	
			  return 2;				
			}	
		i++;
	}
return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
									//Function readTokens
//////////////////////////////////////////////////////////////////////////////////////////
char** readTokens(FILE* stream)//function which will read and will parse the input
{
	//variables
	int  counter;//a counter
	int  numWords;//how many words 
	char input[511];//for the input of the user before the parse
	char temp[511];//for a copy of the input 

	//scanner
	fgets(input, sizeof (input),stream);//to scan the input of the user


	//copy the input into the temp
	strcpy(temp,input);

	//First use of StrTok to know how many words there are
	char *a =strtok(temp," \n");

	numWords=0;//init the number of words
	while(a!=NULL)
	{
		a =strtok(NULL," \n");
		numWords++;
	}

	if(numWords==0)//if there is not words
		return NULL;//Stop the function and go back to the main

	//Construction of the array of char*
	char **array = (char**)malloc((numWords+1)*sizeof(char*));

	if(array==NULL)
	{
		printf("ERROR ALLOCATION in the function 'readtokens' \n");
		run=1; //exit of the loop of the main
		return NULL;//Stop the function
	}
	//Second use of StrTok to parse the words and fill the array
	counter=1;
	char *b =strtok(input," \n");
	array[0]=b;
	while(b!=NULL)
	{
		b =strtok(NULL," \n");
		array[counter]=b;
		counter++;
	}	
	array[numWords]=NULL;

	//Verify if the command is exit
	if(numWords==1 && strcmp("exit", array[0]) == 0)//if exit
	{
		run=1;//to exit from the loop of the main
		free(array);//free the array
		return NULL;//Stop the function
	}

	else
		return array; // all is ok so return to the main with the array
}



//////////////////////////////////////////////////////////////////////////////////////////
									//Function VerifyDollar // verify dollar
//////////////////////////////////////////////////////////////////////////////////////////


int verifyDollar(char** tokens)
{
	int i=1;//pointer
	int flag =0;//indicator flag
	while(tokens[i]!=NULL)//loop in the tokens
	{
		if(tokens[i][0]=='$')//if there is a dollar in the begining of an argument
		{
			freeTokens(tokens);// go to the function wich will free the token tok
			flag=1;
			printf("Our minishell cannot handle the $ sign for the argument : %d\n",i);
			break;//Stop the loop
		}
		i++;
	}

	return flag;
}


//////////////////////////////////////////////////////////////////////////////////////////
									//Function freeTokens // free the memory
//////////////////////////////////////////////////////////////////////////////////////////
void freeTokens(char** tokens)//Function to free the memory
{
	if(tokens==NULL)//If there is anything to free
		{	
			return;
		}
	else
		{
			free(tokens);//free the **array
		}
}


//////////////////////////////////////////////////////////////////////////////////////////
									//Function sigHandler // for the signal 
//////////////////////////////////////////////////////////////////////////////////////////

void sigHandler(int signum)//function to catch the signal
{

}


//////////////////////////////////////////////////////////////////////////////////////////
									//diviseLeft // return an array for firstCommand
//////////////////////////////////////////////////////////////////////////////////////////

char** diviseLeft(char** tok,int location)
{	int i=0;
	int j=location;
	
	while(i<j)
		{
			i++;
		}

char** tokensLeft = malloc((i+1)*sizeof(char*));
i=0;
	while(i<j)
		{	
			tokensLeft[i]=tok[i];
			i++;
		}
		tokensLeft[j]=NULL;
return tokensLeft;

}

//////////////////////////////////////////////////////////////////////////////////////////
									//diviseRight // return an array for secondCommand
//////////////////////////////////////////////////////////////////////////////////////////

char** diviseRight(char** tok,int location)
{	
	printf("LOCATION: %d\n",location);
	int i=0;
	int j=location;
	j++;

		while(tok[j]!=NULL)
			{
				i++;
				j++;
			}

char** tokensRight = malloc((i+1)*sizeof(char*));
i=0;
j=location+1;

		while(tok[j]!=NULL)
		{
			tokensRight[i]=tok[j];
			i++;
			j++;
		}
		tokensRight[i]=NULL;
return tokensRight;
}

//////////////////////////////////////////////////////////////////////////////////////////
								//Function Redirection // for the sign redirection
//////////////////////////////////////////////////////////////////////////////////////////
void redirection(char** tok,int location)
{
	int i=location;
	int fd=5;
	int fd2=5;
	char path[2048];
	char path2[2048];
	char *addToPath;//to add the input to the path
	getcwd(path,sizeof(path));//to obtain the path
	getcwd(path2,sizeof(path));//to obtain the path
	strcat(path,"/");//to add '/' to the path
	strcat(path2,"/");//to add '/' to the path2
	
	if(tok[i+1]==NULL)
	{
		fprintf(stderr , "Your input doesn't correct \n");
		freeTokens(tok);
		return;
	}
		addToPath=tok[i+1];
		strcat(path,addToPath);//to add the name of the new file from the user input

	pid_t process=fork();//process
	if(process==0)

	{
		if((strcmp(tok[i],">")==0 || strcmp(tok[i],">>")==0) && tok[i+1]!=NULL && tok[i+2]==NULL)
			{			
				if(strcmp(tok[i],">")==0) // >
					{
						fd = open(path,O_WRONLY|O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);
					}
				else
					{ //>>
						fd = open(path, O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);
					}

					
					if(fd==-1)//If there is an error to create the file
					{
						fprintf(stderr , "ERROR TO CREATE THE NEW FILE\n");
						exit(1);
					}
					else 
					{
						dup2(fd,STDOUT_FILENO);//change the output of the command to be in the file and not in the screen.
						tok[i]=NULL;
						execvp(tok[0],tok);
						printf("ERROR ! wrong command\n");
						exit(1);
					}
					
			}
	
		else if(strcmp(tok[i],"<")==0 && tok[i+1]!=NULL && tok[i+2]==NULL)
			{
				fd = open(path, O_RDONLY |S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);
				
				if(fd==-1)
					{
					 fprintf(stderr,"Your file doesn't exist\n");
						exit(1);	
					}

				dup2(fd,STDIN_FILENO);
				tok[i]=NULL;
				execvp(tok[0],tok);
				printf("ERROR ! wrong command\n");	
				exit(1);
			}

		else if(strcmp(tok[i],"<")==0 && tok[i+1]!=NULL && ( strcmp(tok[i+2],">")==0 || strcmp(tok[i+2],">>")==0 ) && tok[i+3]!=NULL && tok[i+4]==NULL)
				{
					addToPath=tok[i+3];
					strcat(path2,addToPath);//to add the name of the new file for the output

					if(strcmp(tok[i+2],">")==0 )
						{
						//erase and writte
						fd = open(path2,O_WRONLY|O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);//open or create the folder for the output
						}
					else 
						{
						//write
						fd = open(path2,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);//open or create the folder for the output
						}

					fd2 = open(path, O_RDONLY |S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,0777);//open the folder to read the input
					if(fd==-1)
					{
						fprintf(stderr , "ERROR TO create the folder\n");
						exit(1);
					}
					
					
					if(fd2==-1)
					{
						fprintf(stderr , "ERROR TO open the folder \n");
						exit(1);
					}
					
						dup2(fd2,STDIN_FILENO);
						dup2(fd,STDOUT_FILENO);
						tok[i]=NULL;
						tok[i+2]=NULL;
						execvp(tok[0],tok);	
						printf("ERROR ! wrong command\n");
						exit(1);				
						
				}

		else if(tok[i+3]==NULL)
					{
						fprintf(stderr , "Your input doesn't correct \n");
						freeTokens(tok);
						exit(1);
					}
		
	}

		if(process>0)
			{
				wait(&status);
				freeTokens(tok);
			}
	
		if(process<0)
			{
				fprintf(stderr, "ERROR TO CREAT PROCESS\n");
				freeTokens(tok);
				exit(1);
			}


}
//////////////////////////////////////////////////////////////////////////////////////////
									//END OF THE CODE
//////////////////////////////////////////////////////////////////////////////////////////

