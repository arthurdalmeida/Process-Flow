#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void execProcesso(char* argv[]){
    pid_t pid;

    pid = fork();

    if (pid<0){
        printf("Erro no fork\n");
        return;
    }

    else if (pid == 0){
        printf("fiz o fork, sou filho\n");
        execvp(argv[0], argv);
        printf("erro ao executar programa\n");
        return;
    }

    else {
        printf("fiz o fork, sou o pai\n");
        int status;
        waitpid(pid, &status, 0);
        printf("o filho terminou de rodar\n");
    }
}