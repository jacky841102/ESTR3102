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
    char cmd[] = "ls -l | cat | wc | wc ";
    glob_t pg[100];
    int pipeCount = 0;
    char* token = strtok(cmd, " ");
    while(token != NULL){
        size_t i = 0;
        pg[pipeCount].gl_offs = 0;
        pg[pipeCount].gl_pathc = 0;
        pg[pipeCount].gl_matchc = 0;
        char* argArr[100];
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
        for(j = 0; j < i; j++){
            printf("%s\n", pg[pipeCount].gl_pathv[j]);
        }
        
        //token = strtok(NULL, " ");
        pipeCount++;
        if(token == NULL) break;
        token = strtok(NULL, " ");
    }
    
    setenv("PATH", "/bin:/usr/bin:.",1);
    
    pid_t pid[100];
    int pfd[100][2];
    
    
    printf("##############\n");
    
    
    int i = 0;
    
    for(i = 0; i < pipeCount; i++){
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
            for(i = 0; i < pipeCount - 1; i++){
                close(pfd[i][0]);
                close(pfd[i][1]);
            }
            execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
        }
    }

    if(!(pid[i] = fork())){
        dup2(pfd[i-1][0], 0);
        for(i = 0; i < pipeCount - 1; i++){
        close(pfd[i][0]);
        close(pfd[i][1]);
    }
        execvp(pg[i].gl_pathv[0], pg[i].gl_pathv);
    }

    for(i = 0; i < pipeCount - 1; i++){
        close(pfd[i][0]);
        close(pfd[i][1]);
    }
    
   
    waitpid(-1, NULL, 0);
    
    
    
    return 0;
    
}




