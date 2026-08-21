#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char nome[50];
    char *argv[20];
    int argc;
} task;

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

void cmd(char* tokens[], int qtd){
    if (qtd == 0) return;
    if (strcmp(tokens[0], "run") == 0){
        printf("esse é o comando run\n");
    }

    else if (strcmp(tokens[0], "task") == 0){
        printf("esse é o comando task\n");
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
            cmd(tokens, qtd);
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