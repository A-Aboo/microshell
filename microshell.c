#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void msg(char *s){
    while (*s){
        if (write(2,s++,1) !=1 )exit(1);
    }
}

void fatal(void){
    msg("error: fatal\n");
    exit(1);
}

void cd(char **cmd , int n){
    if (n != 2){
        msg("error: cd: bad arguments\n");
    }else if (chdir(cmd[1]) < 0){
        msg("error: cd: cannot change directory to ");
        msg(cmd[1]);
        msg("\n");
    }
}

void run (char **argv, char **env , int *in , int piped){
    int fd[2];
    pid_t pid;
    if (piped && pipe(fd)< 0)fatal();
    pid = fork();
    if (pid < 0)fatal();
    if (pid == 0){
        if (piped && close(fd[0]) < 0) fatal();
        if (*in !=-1 && dup2(*in , 0) < 0) fatal();
        if (*in !=-1 && *in != 0 && close(*in)<0)fatal();
        if (piped && dup2(fd[1],1)< 0)fatal();
        if (piped && fd[1] != 1 && close(fd[1]) < 0)fatal();

        execve(argv[0] , argv , env );
        msg("error: cannot execute ");
        msg(argv[0]);
        msg("\n");
        exit(1);
    }

    if (*in != -1 && close(*in)<0)fatal();
    *in = -1;
    if (piped){
        if (close(fd[1]) < 0)fatal();
        *in = fd[0];
    }
}

int main(int argc, char **argv , char **env){
    int i =1, in =-1 , n =0, j , piped;
    while (i < argc ){
        j = i;
        while (j < argc && strcmp(argv[j] , "|") && strcmp(argv[j], ";"))j++;
        if (j == i){i++; continue;}

        piped = j < argc && !strcmp(argv[j] , "|");
        argv[j] = NULL;

        if (!strcmp(argv[i] , "cd")){
            cd(argv +i , j -i );
        }else{
            run(argv+i , env , &in, piped);
            n ++;
        }

        if (!piped){
            while (n  >0){
                if (waitpid(-1 , NULL , 0) <0)fatal();
                n--;
            }
        }
        i = j+1;
    }
    return 0;
}