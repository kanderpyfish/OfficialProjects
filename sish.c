#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#define _GNU_SOURCE

char *strtok_r(char *str, const char *delim, char**saveptr);
int cd(char **list);
int historyf(char **list, char **history);
int pipeit(char *line);
int exitcd(char **list);
char *readline(void);
char *builtins[] = {"exit", "cd", "history"};
char **getcommands(char *line);
int forking(char **list);
int execcmd(char **list, char **history);

int size = 1000;
int histcommands = -1;
int main(int argc, char **argv){
        int i;
        int isit = 1;
        char *line;
        char** historylist = (char**)malloc(100 * sizeof(char*));
        for(i = 0; i < 100; i++){
                historylist[i] = (char*)malloc(100 * sizeof(char));
        }
        while(isit){
                printf("sish> ");
                line = readline();
                for(i = 0; line[i] != '\0'; i++){
                        if(i >0 && line[i] == '|' && line[i - 1] != '|' && line[i + 1] != '|'){
                                histcommands++;
                                strcpy(historylist[histcommands % size], line);
                                pipeit(line);
                                free(line);
                        }
                }
                char **listofcommands = getcommands(line);
                isit = execcmd(listofcommands, historylist);

        }
}

int forking(char **list){
        pid_t pid;
        int isit;
        pid = fork();
        if(pid == 0){
                if(execvp(list[0], list) == -1){
                        perror("child process could not execute");
                        return 3;
                }
                exit(EXIT_FAILURE);
        }
        else if(pid < 0){
                perror("Forking error");
                return 3;
        }
        else{
                do{
                        waitpid(pid, &isit, WUNTRACED);
                }while(!WIFEXITED(isit) && !WIFSIGNALED(isit));
        }
        if(isit == 0){
                return 1;
        }
        return 3;
}

int pipeit(char *line){
        int i, amountcom = 0, numpipes = 0;
        pid_t pid;
        char **commands;
        for(i = 0; line[i] != '\0'; i++){
                if(i > 0){
                        if(line[i] == '|' && line[i + 1] != '|' && line[i - 1] != '|'){
                                numpipes++;
                        }
                }
        }
        int* pipes = (int*)malloc((2 * numpipes) * sizeof(int));
        char *getcom = (char*)malloc((150) * sizeof(char));
        getcom = strtok_r(line, "|", &line);
        for(i = 0; i < numpipes; i++){
                if(pipe(pipes + i * 2) < 0){
                        perror("failed to create pipe");
                        return 3;
                }
        }
        do{
                pid = fork();
                if(pid == 0){
                        if(amountcom != 0){
                                if(dup2(pipes[(amountcom - 1) * 2], 0) < 0){
                                        perror("chid didn't get input");
                                        exit(1);
                                }
                        }
                        if(amountcom != numpipes){
                                if(dup2(pipes[amountcom * 2 + 1], 1) < 0) {
                                        perror("child did not produce output");
                                        exit(1);
                                }
                        }
                        for(i = 0; i < 2 * numpipes; i++){
                                close(pipes[i]);
                        }
                        commands = getcommands(getcom);
                        execvp(commands[0], commands);
                        perror("excvp failed");
                        exit(1);
                }
                else if(pid < 0){
                        perror("failed to fork");
                        return 3;
                }
                amountcom++;
        }
        while(amountcom < numpipes + 1 && (getcom = strtok_r(NULL, "|", &line)));
        for(i = 0; i < 2 * numpipes; i++){
                close(pipes[i]);
        }
        free(pipes);
        return 1;
}


int cd(char **list){
        if(list[1] == NULL){
                perror("Argument needed");
                return 3;
        }
        else{
                if(chdir(list[1]) != 0){
                        perror("could not change directory");
                        return 3;
                }
        }
        return 1;
}

int exitcd(char **list){
        return 0;
}

int historyf(char **list, char **history){
        int exec; 
        int i= 0;
        if(list[1] != NULL){
            if(strcmp(list[1], "-c") == 0){
                memset(history, 0, 100); 
            }
            else if(strcmp(list[1], "%d") == 0){
                int offset = atoi(list[1]);
                exec = execcmd(list, history); 
            }
        }
        
        if(histcommands == 100){
                int i = 0;
                for(i = 0; i < 99; i++){
                    strcpy(history[i], history[i + 1]);
                }
            }
        else if(histcommands > 100){
            memmove(&history[0], &history[1], (histcommands - 1) * sizeof(history[0]));
        }

        
        else{
            strcpy(history[histcommands], *list);
            histcommands++;
        }

        for(i = histcommands - 1; i >= 0; i--){
            printf("%d %s \n", i, history[i]);
        }
    
        
        return 1;
}

char *readline(void){
        int buffer = size;
        int index = 0;
        char *bufferstring = malloc(sizeof(char) * buffer);
        int string;
        if(!bufferstring){
                perror("Failed to allocate memory");
                exit(EXIT_FAILURE);
        }
        while(1){
                string = getchar();
                if(string == '\0'){
                        exit(EXIT_SUCCESS);
                }
                else if(string == '\n'){
                        bufferstring[index] = '\0';
                        return bufferstring;
                }
                else {
                        bufferstring[index] = string;
                }
                index++;
        }
}

char **getcommands(char *line){
        int buffer = size;
        int index = 0;
        char **commandarray = malloc(buffer * sizeof(char*));
        char *command;

        if(!commandarray){
                perror("memory allocation error");
                exit(EXIT_FAILURE);
        }
        command = strtok(line, " \t\r\n\a");
        while(command != NULL){
                commandarray[index] = command;
                index++;
                command = strtok(NULL, " \t\r\n\a");
        }
        commandarray[index] = NULL;
        return commandarray;
}
int execcmd(char **list, char **history){
        int i;
        if(list[0] == NULL){
                return 1;
        }
        histcommands++;
        strcpy(history[histcommands % size], list[0]);
        if(list[1] != NULL){
                for(i = 1; i < size; i++){
                        if(list[i] != NULL){
                                strcat(history[(histcommands % 100)], " ");
                                strcat(history[(histcommands % 100)], list[i]);
                        }
                        else{
                                break;
                        }
                }
        }
        for(i = 0; i < (sizeof(builtins) / sizeof(char *)); i++){
                if(strcmp(list[0], builtins[i]) == 0){
                        switch(i){
                                case 0: return exitcd(list); break;
                                case 1: return cd(list); break;
                                case 2: return historyf(list, history); break;
                        }
                }
        }
        return forking(list);
}