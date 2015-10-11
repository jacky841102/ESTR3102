#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <glob.h>
#include <signal.h>



int main(void){
    char buf[PATH_MAX+1];
    char path[PATH_MAX+1];
    char* argArray[PATH_MAX];
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    while(1){
        getcwd(path, PATH_MAX+1);
        printf("[3150 shell:%s]$ ", path);
        if(fgets(buf, 256, stdin) == NULL) exit(0);
        buf[strlen(buf)-1] = '\0';
        if(buf[0] == '\0') continue;

        char* token = strtok(buf, " ");
        if(strcmp(token, "cd") == 0){
            token = strtok(NULL, " ");
            if(token == NULL || strtok(NULL, " ")!=NULL){
                printf("cd: wrong number of arguments\n");
            }else{
                 if(chdir(token) == -1){
                    printf("%s: cannot change directory\n", token);
                }
            }
        }else if(strcmp(token, "exit") == 0){
            if(strtok(NULL, " ")!=NULL){
                printf("exit: wrong number of arguments\n");
            }else{
                exit(0);
            }
        }else if(strcmp(token, "fg") == 0){

        }else if(strcmp(token, "jobs") == 0){

        }else{
            size_t i = 0, flagNum = 0;
            glob_t pg;
            pg.gl_offs = 0;
	        pg.gl_pathc = 0;
            while(token != NULL){
                argArray[i] = token;
                if(token[0] == '-'){
                    flagNum++;
                }
                token = strtok(NULL, " ");
                i++;
            }
            
            size_t argNum = i;

            size_t pid;
            pid = fork();
            setenv("PATH", "/bin:/usr/bin:.",1);
            if(!pid){
                signal(SIGINT,SIG_DFL);
                signal(SIGQUIT,SIG_DFL);
                signal(SIGTERM,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                    argArray[argNum] = NULL;
                    execvp(argArray[0], argArray);
                    if(errno == ENOENT){
                        printf("%s: command not found\n", argArray[0]);
                        exit(0);
                    }else if(errno != ENOENT){
                        printf("%s: unknown error\n", argArray[0]);
                        exit(0);
                    }
               
            }else{
                waitpid(pid, NULL, WUNTRACED);
            }
        }
    }
    return 0;
}
