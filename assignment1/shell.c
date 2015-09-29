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
#include "cd.h"




int main(void){
    char buf[255];
    char path[255];
    while(1){
        printf("[3150 shell:%s]$ ", getcwd(path, PATH_MAX+1));
        fgets(buf, 255, stdin);
        buf[strlen(buf)-1] = '\0';

        //tokenize command
        char* token = strtok(buf, " ");
       
        if(strcmp(token, "cd") == 0){
            token = strtok(NULL, " ");
            if(token == NULL){
                printf("cd: wrong number of arguments\n");
            }
            else if(strtok(NULL, " ")!=NULL){
                printf("cd: wrong number of arguments\n");
            }else{
                cd(token);
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

        }
    }
    return 0;
}
