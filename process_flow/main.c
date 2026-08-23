#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void execProcesso(char* argv[]);
pid_t processoParallel(char* argv[]);

typedef struct {
    char nome[50];
    char *argv[20];
    int argc;
} task;

task tasks[50];
int qtdDeTasks = 0;

int parse(char* linha, char* tokens[]){
    int cont = 0;
    char* token = strtok(linha, " \n");

    while (token != NULL && cont < 20){
        tokens[cont] = token;
        cont++;
        token = strtok(NULL, " \n");
    }
    return cont;
}

void checar(char* tokens[], int qtd){
    if (qtd == 0) return;
    if (strcmp(tokens[0], "run") == 0){
        if (qtd<2){
            printf("nao tem tarefa\n");
            return;
        }
        if (strcmp(tokens[1], "sequential") == 0){
            for (int i=2; i<qtd; i++){
                for (int j=0; j<qtdDeTasks; j++){
                    if (strcmp(tasks[j].nome, tokens[i]) == 0){
                        execProcesso(tasks[j].argv);
                    }   
                }
            }
        }

        else if(strcmp(tokens[1], "parallel") == 0){
            pid_t pids[20];
            int qtdPID = 0;

            for (int i=2; i<qtd; i++){
                for (int j=0; j<qtdDeTasks; j++){
                    if (strcmp(tasks[j].nome, tokens[i]) == 0){
                        pids[qtdPID] = processoParallel(tasks[j].argv);

                        if (pids[qtdPID] != -1){
                            qtdPID++;
                        }
                    }
                }
            }
            for (int i=0; i<qtdPID; i++){
                waitpid(pids[i], NULL, 0);
            }
        }

    
        else{
            for (int i=0; i<qtdDeTasks; i++){
            if (strcmp(tokens[1], tasks[i].nome) == 0){
                execProcesso(tasks[i].argv);
                printf("achasse a tarefa");
                }
            }
        }
    }
    else if(strcmp(tokens[0], "output") == 0){
        int arquivo = open(tokens[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (arquivo < 0){
            return;
        }

        for (int i=0; i<qtdDeTasks; i++){
            if(strcmp(tokens[1], tasks[i].nome)== 0){
                pid_t pid = fork();

                if (pid <0){
                    printf("erro no fork");
                }

                else if (pid == 0){
                    dup2(arquivo, 1);
                    close(arquivo);
                    execvp(tasks[i].argv[0], tasks[i].argv);
                }

                else{
                    close(arquivo);
                    waitpid(pid, NULL, 0);
                }
            }
        }
    }

    else if(strcmp(tokens[0], "input") == 0){
        int arquivo = open(tokens[2], O_RDONLY);

        if (arquivo <0) return;

        for (int i=0; i<qtdDeTasks; i++){
            if(strcmp(tokens[1], tasks[i].nome)== 0){
                pid_t pid = fork();

                if (pid < 0){
                    printf("erro no fork\n");
                }

                else if(pid == 0){
                    dup2(arquivo, 0);
                    close(arquivo);
                    execvp(tasks[i].argv[0], tasks[i].argv);
                }

                else{
                    close(arquivo);
                    waitpid(pid, NULL, 0);
                }
            }
        }
    }

    else if(strcmp(tokens[0], "append") == 0){
        int arquivo = open(tokens[2], O_WRONLY | O_CREAT | O_APPEND, 0644);

        if (arquivo < 0){
            return;
        }

        for (int i=0; i<qtdDeTasks; i++){
            if(strcmp(tokens[1], tasks[i].nome)== 0){
                pid_t pid = fork();

                if (pid <0){
                    printf("erro no fork");
                }

                else if (pid == 0){
                    dup2(arquivo, 1);
                    close(arquivo);
                    execvp(tasks[i].argv[0], tasks[i].argv);
                }

                else{
                    close(arquivo);
                    waitpid(pid, NULL, 0);
                }
            }
        }
    }

    else if (strcmp(tokens[0], "task") == 0){
        if (qtd<3){
            return;
        }
        strcpy(tasks[qtdDeTasks].nome, tokens[1]);

        int j = 0;
        for (int i=2; i<qtd; i++){
            tasks[qtdDeTasks].argv[j] = malloc(strlen(tokens[i]) + 1);
            strcpy(tasks[qtdDeTasks].argv[j], tokens[i]);
            j++;
        }

        tasks[qtdDeTasks].argc = j;
        tasks[qtdDeTasks].argv[j] = NULL;
        qtdDeTasks++;
    }
    
    else if (strcmp(tokens[0], "exit") == 0){
        printf("esse é o comando exit\n");
    }

    else{
        printf("esse comando não existe");
    }
}

int main(){
    char linhaDigitada[200];
    char* tokens[20];

    while (fgets(linhaDigitada, 200, stdin) != NULL){
        int qtd = parse(linhaDigitada, tokens);

        if (qtd > 0){
            checar(tokens, qtd);
        }
        else{
            continue;
        }
                
        if (strcmp(tokens[0], "exit") == 0){
            printf("programa encerrado");
            break;
        }
    }
    return 0;
}