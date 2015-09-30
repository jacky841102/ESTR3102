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
    char buf[255];
    char path[255];
    char* argArray[100];
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    while(1){

        printf("[3150 shell:%s]$ ", getcwd(path, PATH_MAX+1));
        fgets(buf, 255, stdin);
        buf[strlen(buf)-1] = '\0';
        
       

        //tokenize command
        if(strlen(buf) == 0) continue;
        char* token = strtok(buf, " ");
        if(strcmp(token, "cd") == 0){
            token = strtok(NULL, " ");
            if(token == NULL || strtok(NULL, " ")!=NULL){
                printf("cd: wrong number of arguments\n");
            }else{
                 if(chdir(token) == -1){
                    printf("[%s]: cannot change directory\n", token);
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
                    if(execvp(argArray[0], argArray) == -1){
                        printf("%s: command not found\n", argArray[0]);
                    }
                }else{
                    for(i = 0; i < pg.gl_offs; i++){
                        pg.gl_pathv[i] = argArray[i];
                    }
                    if(execvp(argArray[0], pg.gl_pathv) == -1){
                        printf("%s: command not found\n", argArray[0]);
                    }
                }
            }else{
                waitpid(pid, NULL, WUNTRACED);
            }
        }
    }
    return 0;
}
