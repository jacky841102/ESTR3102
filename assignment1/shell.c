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
    char path[PATH_MAX+1];
    char* argArr[PATH_MAX];
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
            if(strtok(NULL, " ")!=NULL){
                printf("exit: wrong number of arguments\n");
            }else{
                exit(0);
            }
        }else if(strcmp(token, "fg") == 0){

        }else if(strcmp(token, "jobs") == 0){

        }else{
             glob_t pg[100];
             int pipeCount = 0;
             while(token != NULL){
                size_t i = 0;
                pg[pipeCount].gl_offs = 0;
                pg[pipeCount].gl_pathc = 0;
                pg[pipeCount].gl_matchc = 0;
                
                while(token[0] != '|'){
                    argArr[i] = token;
                    i++;
                    token = strtok(NULL, " ");
                    if(token == NULL) break;
                }
        
        
                size_t j;
                glob(argArr[0], GLOB_NOCHECK, NULL, &pg[pipeCount]);
                if(i > 1){
                    for(j = 1; j < i; j++){
                        glob(argArr[j], GLOB_APPEND | GLOB_NOCHECK, NULL, &pg[pipeCount]);
                    }
                }
                
                pipeCount++;
                if(token == NULL) break;
                token = strtok(NULL, " ");
            }
    
            setenv("PATH", "/bin:/usr/bin:.",1);
            

            if(pipeCount > 1){

                pid_t pid[100];
                int pfd[100][2];
                
                int i = 0;
    
                for(i = 0; i < pipeCount -1; i++){
                    pipe(pfd[i]);
                }


                if(!(pid[0] = fork())){
                    dup2(pfd[0][1], 1);
                    for(i = 0; i < pipeCount - 1; i++){
                        close(pfd[i][0]);
                        close(pfd[i][1]);
                    }
                    execvp(pg[0].gl_pathv[0], pg[0].gl_pathv);
                }

                for(i = 1; i < pipeCount - 1; i++){
                    if(!(pid[i] = fork())){
                        dup2(pfd[i-1][0], 0);
                        dup2(pfd[i][1], 1);
                        int j;
                        for(j = 0; j < pipeCount - 1; j++){
                            close(pfd[j][0]);
                            close(pfd[j][1]);
                        }
                        execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
                    }
                }

                if(!(pid[i] = fork())){
                    dup2(pfd[i-1][0], 0);
                    int j;
                    for(j = 0; j < pipeCount - 1; j++){
                        close(pfd[j][0]);
                        close(pfd[j][1]);
                    }
                    execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
                }

                for(i = 0; i < pipeCount - 1; i++){
                    close(pfd[i][0]);
                    close(pfd[i][1]);
                }
    
                for(i = 0; i < pipeCount; i++){
                    waitpid(pid[i], NULL, WUNTRACED);
                }

            }else{
                int pid;
                if(!(pid = fork())){
                     execvp(pg[0].gl_pathv[0], pg[0].gl_pathv);
                }else{
                    waitpid(pid, NULL, WUNTRACED);
                }
            }
        }
    }
}
