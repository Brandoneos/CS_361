#define  _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include "hw2.h"

void printArray(char** array1) {//to test array of strings
    for (int i = 0; array1[i] != NULL; i++) {
        printf("%s\n", array1[i]);
    }
}

char** getargs(char* input, char*cmd) {
    int count = 0;
    
    

    char** array = malloc(10 * sizeof(char *));
    if(cmd != NULL) {
        array[count] = malloc(strlen(cmd) + 1);
        strcpy(array[count], cmd);
        count++;
    }
    if(input == NULL) {
        array[1] = NULL;
        return array;
    }
    
    char* stringtok = strtok(input, " ");
    while(stringtok != NULL) {
        if (count >= 10) {
            array = realloc(array, (count + 1) * sizeof(char *));
            if (array == NULL) {
                for (int i = 0; i < count; i++) {
                    free(array[i]);
                }
                free(array);
                return NULL;
            }
        }
        
        array[count] = malloc(strlen(stringtok) + 1);
        strcpy(array[count], stringtok);
        count++;
        stringtok = strtok(NULL, " ");
    }
    array[count] = NULL;//end with NULL
    
    return array;
}


void echo(char * input) {
    // printf("input1:%s\n",input1);
    // printf("withoutECHO:%s\n",withoutECHO);
    if(input == NULL) {
        return;
    }
    int lineNumber = 1;
    char* stringtok = strtok(input," ");
    while(stringtok != NULL) {
        printf("%d|%s\n",lineNumber,stringtok);
        lineNumber++;
        stringtok = strtok(NULL," ");
    }
    
}


void execute(char * input)
{
    char * sep = NULL;
    //for output redirection
    int file = -1;
    int file2 = -1;
    //for input redirection
    int file3 = -1;
    int file4 = -1;
    //for error redirection
    int file5 = -1;
    int file6 = -1;
    int stderrCopy = -500;
    if((sep=strstr(input, "2>"))) {//3
        *sep = 0;
        char * fileout = sep + 2;
        // printf("fileout:%s\n",fileout);
        // TODO: stderr redirect
        if(fileout != NULL) {
            file5 = open(fileout, O_WRONLY | O_CREAT | O_TRUNC,0777);//O_WRONLY = WRite only flag, same as stdout
            // O_CREAT creates the file if it doesnt exist, 0777 - gives complete access.
            if(file5 == -1) {
                printf("File opening failed\n");
            }
            stderrCopy = dup(2);
            file6 = dup2(file5, 2);// 1 represents the STDERR file descriptor number
        
            close(file5);
        }
    }
    int stdoutCopy = -500;
    int stdinCopy = -500;
    if((sep=strstr(input,">"))) {//1
        *sep = 0;
        char * fileout = sep + 1;//name of the file given to redirect output to
        // printf("sep:%s\n",sep);
        // printf("fileout:%s\n",fileout);//
        if(fileout != NULL) {
            file = open(fileout, O_WRONLY | O_CREAT | O_TRUNC,0777);//O_WRONLY = WRite only flag
            // O_CREAT creates the file if it doesnt exist, 0777 - gives complete access.
            if(file == -1) {
                printf("File opening failed\n");
            }
            stdoutCopy = dup(1);
            file2 = dup2(file, 1);// 1 representd the STDOUT file descriptor number
        
            close(file);
        }
        
    }

    if((sep=strstr(input,"<"))) {//2
        *sep = 0;
        char * filein = sep + 1;
        // printf("filein\n");
        // printf("filein:%s\n",filein);//

        if(filein != NULL) {
            file3 = open(filein, O_RDONLY ,0777);//O_RDONLY = read only flag, O_CREAT AND O_TRUNC might not be necessary
            // TODO: stdin redirect
            if(file3 == -1) {
                printf("File opening failed\n");
            }
            stdinCopy = dup(0); //dup stdin
            file4 = dup2(file3, 0); // 0 represents the stdin fd number, may have to change this variable in the future with both input and
            close(file3);
        }
        
    }

    char * cmd = strtok(input," \n");
    if(cmd == NULL) {
        fflush(stderr);
        return;
    }
    char* rest = strtok(NULL, "\0");
    
    // printf("inputAfter:%s\n",input);
    // printf("REST:%s\n",rest);
    // printf("cmd:%s\n",cmd);// /usr/bin/which cat -> /usr/bin/which
    // printf("input1:%s\n",input);
    
    if (!strcmp("echo", cmd)) {
        // TODO: built-in echo
        // printf("Echo was called\n");
        echo(rest);
        // fflush(stderr);
        fflush(stdout);
    } else {
        // TODO: execute the user's command
        // printf("cmd:%s\n",cmd);
        // printf("REST:%s\n",rest);
        int argvBool = 0;
        char** argv = getargs(rest,cmd);
        // printf("printing argv:\n");
        // printArray(argv);

        char* pathString = getenv("PATH");
        int success = 0;


        pid_t cpid = fork();
        
        // printf("CPID of p:%d\n",cpid);
        if(cpid == 0) {
            // printf("CPID of first child process:%d\n",cpid);
            // printf("Child Process 1 Started\n");
            // printf("PATH:%s\n",pathString);
            
            char* stringtok = strtok(pathString, ":");
            // printf("Tried exec for command:%s\n",cmd);
            
            if(execv(cmd, argv) == -1) {//if execv fails for regular command
                int allstatus = 1;
                int pstatus;
                
                while(stringtok != NULL) {
                    char newCommand[BSIZE];
                    char pathWithSlash[BSIZE];
                    sprintf(pathWithSlash, "%s%s", stringtok, "/");//modify stringtok to end with /
                    sprintf(newCommand, "%s%s", pathWithSlash, cmd);//or strconcat if doesnt work, adds the original commands
                    pid_t cpid1 = fork();
                    if(cpid1 == 0) {
                        // printf("CPID of 2nd child:%d\n",cpid);
                        // printf("Child Process 2 Started\n");
                        // printf("Tried exec for command1:%s\n",newCommand);
                        if(execv(newCommand, argv) == -1) {//try with each path
                            
                            success = 1;
                            // exit(1);
                            // printf("COmmand failed\n");
                        } else {
                            success = 1;
                            // exit(0);
                            printf("Success is 1\n");
                            exit(0);
                            
                        }
                        // fflush(stderr);
                    } else {
                        // printf("CPID of 2nd parent process:%d\n",cpid);
                        // printf("Parent Process 2 Started\n");
                        
                        waitpid(cpid1, &pstatus, 0);
                        // printf("pstatus:%d\n",pstatus);

                        if(pstatus == 0) {
                            exit(0);
                        }
                        
                        if(pstatus > 0) {
                            exit(1);
                        } else {
                            exit(0);
                        }
                        // exit(0);
                        // fflush(stderr);
                    }
                    // printf("newCommand:%s\n",newCommand);
                    stringtok = strtok(NULL, ":");
                    // if(success == 1) {
                    //     break;
                    // }
                    // printf("Ended while once 1 loop\n");
                }
                // printf("Ended while loop\n");
                // if(allstatus == 0) {
                //     exit(0);
                // } else {
                //     exit(1);
                // }
                exit(1);
            }
            // fflush(stderr);
            exit(0);
        } else {
            // printf("CPID of 1st parent process:%d\n",cpid);
            // printf("Parent Process 1 Started\n");
            int status;
            waitpid(cpid, &status, 0);
            // printf("status:%d\n",status);
            
            if(status > 0) {
                if(stderrCopy != -500){

                } else {
                    fprintf(stderr,"cs361sh: command not found: %s\n",cmd);
                }
                
            }
            
            // fflush(stderr);
        }
        // printArray(argv);
        // printf("cmd:%s\n",cmd);
        // printf("rest:%s\n",rest);
        
        // pid_t cpid = fork();
        // if(cpid == 0) {

        // }
        // printf("Success:%d\n",success);
        // if(success == 0) {
            
        // }
        // fprintf(stderr,"cs361sh: command not found: %s\n",cmd);
        // fflush(stderr);

        //free argv as it was malloc'd in getargs()
        
            for (int i = 0; argv[i] != NULL; i++) {
                free(argv[i]);
            }
            free(argv);
        
        
    }
    // free(input1);
    fflush(stderr);
    fflush(stdout);
    fflush(stdin);
    if(stderrCopy != -500) {
        dup2(stderrCopy,2);
    }
    if(stdinCopy != -500) {
        dup2(stdinCopy,0);
    }
    if(stdoutCopy != -500) {
        dup2(stdoutCopy,1);
    }
    
    if(file2 != -1) {
        // printf("Test\n");
        // close(file2);//might need to close the other file?
        
    }
    
    return;
}



void pipeline(char * head, char * tail)
{
    // TODO
}


void sequence(char * head, char * tail)
{
    char* line = head;
    char *sep = tail;
    char* newLine;
    // *sep = 0;
    // sep++;
    execute(head);
        while((newLine = strstr(sep,";"))) {
            *newLine = 0;
            newLine++;
            // printf("newline:%s\n",newLine);
            // printf("sep:%s\n",sep);
            execute(sep);
            
            sep = newLine;
            // printf("sepAfterExec:%s\n",sep);
            // sequence(line,sep);
        }
        
        
        //sequence last portion


        // TODO: sequence
        //call sequence function
    
    // printf("seplast:%s\n",sep);
    execute(sep);



    
    fflush(stderr);
}


void run_cmd(char * line)
{
    char *sep = 0;

    if((sep=strstr(line,";"))) {
        // printf("Sequence\n");
        *sep = 0;//separates the 1st command from the rest
        sep++;
        // printf("Head:%s\n",line);
        // printf("Tail:%s\n",sep);
        sequence(line,sep);
        // TODO: sequence
        //call sequence function
    }
    else if((sep=strstr(line,"|"))) {
        printf("Pipeline found\n");
        // *sep = 0;

        // TODO: pipeline
        //call pipeline function
    }
    else {
        // TODO: executing the user's command
        // printf("line:%s\n",line);
        execute(line);
        // fflush(stdout);
        // printf("done cmd\n");
    }
}


int main(int argc, char * argv[])
{
    printf("cs361sh> ");
    fflush(stdout);

    // handy copies of original file descriptors

    int orig_in = dup(0);
    int orig_out = dup(1);
    int orig_err = dup(2);

    char line[BSIZE];

    while(read_line(STDIN_FILENO, line, BSIZE) > 0)  {

        // make sure line ends with null
        line[strlen(line)-1] = '\0';

        run_cmd(line);

        // TODO: possibly fix redirection related changes

        printf("cs361sh> ");
        fflush(stdout);
   }

    printf("\n");
    fflush(stdout);
    fflush(stderr);
    return 0;
}


/*
    read_line will read one line from stdin
    and  store that  line in  buf.  It will
    include the trailing newline in buf.
*/

int read_line(int fd, char buf[], int max)
{
    if (max <= 0)
        return 0;

    int i = 0;
    while (i < max - 1) {
        char c;
        size_t amt = read(fd, &c, 1);
        if (amt != 1)
            break;
        buf[i++] = c;
        if (c == '\n')
            break;
    }
    buf[i] = '\0';

    return i;
}
