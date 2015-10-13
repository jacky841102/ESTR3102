/*************************************************************************
	> File Name: mole.c
	> Author: jacky
	> Mail: kuoyichun1102@gmail.com
	> Created Time: 二 10/13 21:14:33 2015
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
int main(){
    
    
    printf("mole");
    printf(" 影分身之術!! ");
    
    if(fork()){
        wait(NULL);
        printf("i am parent\n");
        return 0;
    }
    else{
        printf("i am child\n");
        return 0;
    }
}
