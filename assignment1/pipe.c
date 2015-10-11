/*************************************************************************
	> File Name: pipe.c
	> Author: jacky
	> Mail: kuoyichun1102@gmail.com
	> Created Time: 日 10/11 13:59:46 2015
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<glob.h>
#include<stdlib.h>




int main(){
    char cmd[] = "ls -l | cat | wc";
    glob_t pg[100];
    int pipeCount = 0; 
    char* token = strtok(cmd, " ");
    while(token != NULL){
        size_t i = 0;
        pg[pipeCount].gl_offs = 1;
        pg[pipeCount].gl_pathc = 0;
        char* argArr[100];
        while(token[0] != '|'){
            argArr[i] = token;
            if(token[0] == '-'){
                pg[pipeCount].gl_offs++;
            }
            i++;
            token = strtok(NULL, " ");
            if(token == NULL) break;
        }
        if(token == NULL) break;

        size_t j;
        if(pg[pipeCount].gl_offs > 1){
            glob(pg[pipeCount].gl_pathv[1], GLOB_DOOFFS | GLOB_NOCHECKS, NULL, &pg[pipeCount]);
            if(pg[pipeCount].gl_offs > 2){
                for(j = 2; j < i; j++){
                    glob(pg[pipeCount].gl_pathv[j], GLOB_DOOFFS | GLOB_APPEND | GLOB_NOCHECKS, NULL, &pg[pipeCount]);
                }
            }
        }

        for(j = 0; j < pg[pipeCount].gl_offs; j++){
            pg[pipeCount].gl_pathv[j] = argArr[j];
        }

        token = strtok(NULL, " ");
        //pg[pipeCount].gl_pathv[pg[pipeCount].gl_pathc + pg[pipeCount].gl_offs] = NULL;
        pipeCount++;
    }

    setenv("PATH", "/bin:/usr/bin:.",1);
    
    int i = 0;
    pid_t pid[100];
    int pfd[2];
    pipe(pfd);
    if(!(pid[0] = fork())){
        close(pfd[0]);
        dup2(pfd[1], 1);
        close(pfd[1]);
        execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
    }

    for(i = 1; i < pipeCount; i++){
        if(!(pid[i] = fork())){
            dup2(pfd[0], 0);
            close(pfd[0]);
            dup2(pfd[1], 1);
            close(pfd[1]);
            execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
        }
    }

     if(!(pid[i] = fork())){
        close(pfd[1]);
        dup2(pfd[0], 10);
        close(pfd[0]);
        execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
    }

    for(int i = 0; i < pipeCount + 1; i++){
        waitpid(pid[i], NULL, WUNTRACED);
    }

    return 0;

}
