#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    pid_t pid;

    pid = fork();

    if (pid<0){
        printf("Erro no fork\n");
        return 1;
    }

    else if (pid == 0){
        printf("fiz o fork, sou filho\n");
        execlp("/bin/ls", "ls", "-1", NULL);
        printf("erro ao executar programa\n");
        return 1;
    }

    else {
        printf("fiz o fork, sou o pai\n");
        int status;
        waitpid(pid, &status, 0);
        printf("o filho terminou de rodar");
    }
    return 0;
}