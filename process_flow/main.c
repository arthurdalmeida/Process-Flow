#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


typedef struct {
    char nome[50];
    char *argv[20];
    int argc;
} task;

task tasks[50];
int qtdDeTasks = 0;

typedef struct {
    int id;
    pid_t pid;
    char nome[50];
    int ativo;
} job;

job jobs[50];
int qtdJobs = 0;

void execProcesso(char* argv[]){
    pid_t pid;

    pid = fork();

    if (pid<0){
        printf("Erro no fork\n");
        return;
    }

    else if (pid == 0){
        execvp(argv[0], argv);
        printf("Erro ao executar programa\n");
        exit(1);
    }

    else {
        int status;
        waitpid(pid, &status, 0);
    }
}


pid_t processoParallel(char* argv[]){
    pid_t pid;

    pid = fork();

    if (pid<0){
        printf("Erro no fork\n");
        return -1;
    }

    else if (pid == 0){
        execvp(argv[0], argv);
        printf("Erro, nao foi possivel executae o programa\n");
        exit(1);
    }

    else {
        printf("iniciou o processo filho. PID: %d\n", pid);
        return pid;
    }
}


int parse(char* linha, char* tokens[]){
    int cont = 0;
    char* token = strtok(linha, " \t\r\n");

    while (token != NULL && cont < 20){
        tokens[cont] = token;
        cont++;
        token = strtok(NULL, " \t\r\n");
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

        else if(strcmp(tokens[1], "pipe") == 0){

            int qtdPipe = qtd -2;
            int posicao[20];
            pid_t pids[20];
            int fd[20][2];

            for (int i=0; i<qtdPipe; i++){
                posicao[i] = -1; 

                for (int j=0; j<qtdDeTasks; j++){
                    if (strcmp(tokens[i+2], tasks[j].nome) == 0){
                        posicao[i] = j;
                        break;
                    }
                }

                if (posicao[i] == -1){
                    printf("a tarefa nao foi encontrada\n");
                    return;
                }
            }

            for (int i=0; i<qtdPipe- 1; i++){
                if (pipe(fd[i]) < 0){
                    printf("deu erro no pipe");
                    return;
                }
            }

            for (int i=0; i<qtdPipe; i++){
                pids[i] = fork();

                if (pids[i] < 0){
                    printf("nao fez fork");
                    return;
                }

                else if(pids[i] == 0){
                    if (i>0){
                        dup2(fd[i-1][0], 0);
                    }

                    if (i< qtdPipe-1){
                        dup2(fd[i][1], 1);
                    }

                    for (int j=0; j<qtdPipe-1; j++){
                        close(fd[j][0]);
                        close(fd[j][1]);
                    }

                    execvp(tasks[posicao[i]].argv[0], tasks[posicao[i]].argv);
                    printf("erro no programa\n");
                    exit(1);
                }
            }

            for (int i=0; i<qtdPipe-1; i++){
                close(fd[i][0]);
                close(fd[i][1]);
            }

            for (int i=0; i<qtdPipe; i++){
                waitpid(pids[i], NULL, 0);
            }
        }
        else{
            for (int i=0; i<qtdDeTasks; i++){
            if (strcmp(tokens[1], tasks[i].nome) == 0){
                execProcesso(tasks[i].argv);
                printf("achasse a tarefa\n");
                }
            }
        }
    }
    else if(strcmp(tokens[0], "output") == 0){
        if (qtd < 3){
            printf("Erro- informe a tarefa e qual é o arquivo\n");
            return;
        }

        int arquivo = open(tokens[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (arquivo < 0){
            printf("Não foi possível abrir o arquivo\n");
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
        if (qtd < 3){
            printf("Erro- informe a tarefa e qual é o arquivo\n");
            return;
        }

        int arquivo = open(tokens[2], O_RDONLY);

        if (arquivo <0){
            printf("Não foi possível abrir o arquivo\n");
            return;
        } 
        

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
        if (qtd < 3){
            printf("Erro- informe a tarefa e qual é o arquivo\n");
            return;
        }

        int arquivo = open(tokens[2], O_WRONLY | O_CREAT | O_APPEND, 0644);

        if (arquivo < 0){
            printf("Não foi possível abrir o arquivo\n");
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

    else if(strcmp(tokens[0], "workdir") == 0){
        if (qtd < 2){
            printf("Erro. O diretorio nao foi informad\n");
            return;
        }

        if (chdir(tokens[1]) != 0){
            printf("Erro. Diretorio invalido\n");
            return;
        }

        printf("Diretorio foi alterado\n");
    }

    else if(strcmp(tokens[0], "start") == 0){
        if (qtd <2){
            printf("Erro. Informe a tarefa\n");
            return;
        }

        for (int i=0; i<qtdDeTasks; i++){
            if (strcmp(tokens[1], tasks[i].nome) == 0){
                pid_t pid = fork();

                if (pid<0){
                    printf("deu erro no fork\n");
                    return;
                }

                if (pid == 0){
                    execvp(tasks[i].argv[0], tasks[i].argv);
                    exit(1);
                }

                if (pid>0){
                    jobs[qtdJobs].id = qtdJobs + 1;
                    jobs[qtdJobs].pid = pid;
                    jobs[qtdJobs].ativo = 1;
                    strcpy(jobs[qtdJobs].nome, tasks[i].nome);

                    printf("[%d]- %d\n", jobs[qtdJobs].id, pid);
                    qtdJobs++;
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

    else if(strcmp(tokens[0], "jobs") == 0){
        for (int i=0; i<qtdJobs; i++){
            if (jobs[i].ativo == 1){
                pid_t resultado = waitpid(jobs[i].pid, NULL, WNOHANG);

                if (resultado == jobs[i].pid){
                    jobs[i].ativo = 0;
                }
                else if (resultado == 0){
                    printf("[%d]- %d %s\n", jobs[i].id, jobs[i].pid, jobs[i].nome);
                }
            }
        }
    }

    else if(strcmp(tokens[0], "wait") == 0){
        if (qtd <2){
            printf("Erro. Informe o job\n");
            return;
        }

        int id = atoi(tokens[1]);

        for (int i=0; i<qtdJobs; i++){
            if (jobs[i].id == id){
                if(jobs[i].ativo == 1){
                    waitpid(jobs[i].pid, NULL, 0);
                    jobs[i].ativo = 0;
                }
            }
        }
    }
    
    else if (strcmp(tokens[0], "exit") == 0){
        return;
    }

    else{
        printf("esse comando não existe");
    }
}

int main(int argc, char* argv[]){
    FILE *entrada;

    if (argc == 1){
        entrada = stdin;
    }

    else if(argc == 2){
        entrada = fopen(argv[1], "r");

        if (entrada == NULL){
            printf("Nao abriu o workflow\n");
            return 1;
        }
    }

    else{
        printf("Erro. O número de argumentoe é invalido");
        return 1;
    }

    char linhaDigitada[200];
    char* tokens[20];

    while (1){
        if (argc == 1){
            printf("processflow> ");
            fflush(stdout);
        }
    

        if (fgets(linhaDigitada, 200, entrada) == NULL){
            break;
        }

        if (argc == 2){
            printf("%s", linhaDigitada);
            fflush(stdout);
        }

        int qtd = parse(linhaDigitada, tokens);

        if (qtd == 0){
            continue;
        }

        checar(tokens, qtd);
                
        if (strcmp(tokens[0], "exit") == 0){
            printf("Programa encerrado\n");
            break;
        }
    }

    if (entrada != stdin) fclose(entrada);

    return 0;
}