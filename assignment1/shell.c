/*************************************************************************
	> File Name: ./assignment1/shell.c
	> Author: jacky
	> Mail: kuoyichun1102@gmail.com
	> Created Time: 二  9/29 20:29:38 2015
 ************************************************************************/

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
    char path[255];
    char* argArray[255];
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    while(1){

        fflush(stdin);
        getcwd(path, PATH_MAX+1);
        printf("[3150 shell:%s]$ ", path);
        if(fgets(buf, 255, stdin) == NULL) exit(0);
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
            pg.gl_offs = flagNum + 1;
            if(argNum > 1){
                glob(argArray[1], GLOB_DOOFFS, NULL, &pg);
                if(argNum > 2){
                    for(i = 2; i < argNum; i++){
                        glob(argArray[i], GLOB_DOOFFS | GLOB_APPEND, NULL, &pg);
                    }
                }
            }

            size_t pid;
            pid = fork();
            setenv("PATH", "/bin:/usr/bin:.",1);
            if(!pid){
                signal(SIGINT,SIG_DFL);
                signal(SIGQUIT,SIG_DFL);
                signal(SIGTERM,SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                if(pg.gl_pathc == 0){
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
                    for(i = 0; i < pg.gl_offs; i++){
                        pg.gl_pathv[i] = argArray[i];
                    }
                    execvp(argArray[0], argArray);
                    if(errno == ENOENT){
                        printf("%s: command not found\n", argArray[0]);
                        exit(0);
                    }else if(errno != ENOENT){
                        printf("%s: unknown error\n", argArray[0]);
                        exit(0);
                    }
                }
            }else{
                waitpid(pid, NULL, WUNTRACED);
            }
        }
    }
    return 0;
}
