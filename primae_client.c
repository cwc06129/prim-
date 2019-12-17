#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX 255
#define MAXLINE 1000
#define NAME_LEN 20



int tcp_connect(int af, char *servip, unsigned short port);

void errquit(char *mesg) { perror(mesg); exit(1); }


void *prompt(char cBuf[]) {
        time_t rawtime;
        struct tm *timeinfo;
        char hBuf[MAX], uBuf[MAX], dBuf[MAX];
        char *now;
        void *ret;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        now = asctime(timeinfo);
        now[strlen(now)-1] = 0;
        gethostname(hBuf, MAX);
        getlogin_r(uBuf, MAX);
        getcwd(dBuf, MAX);
        printf("[%s]%s@%s(%s)$ ", now, hBuf, uBuf, dBuf);
        ret = fgets(cBuf, MAX, stdin);

        if(cBuf[strlen(cBuf)-1] == '\n')
                cBuf[strlen(cBuf)-1] = 0;
        return ret;

}


int main(int argc, char *argv[]){


        int s;
        fd_set read_fds;
        char cBuf[MAX];
        char *arg1, *arg2;
        char *argarr[10000];
        int argnum;
        char *savepos1, *savepos2;
        pid_t pid;
        int status;

        if (argc != 3)
        {
                printf("사용법 : %s sever_ip  port name \n", argv[0]);
                exit(0);
        }

        s = tcp_connect(AF_INET, argv[1], atoi(argv[2]));

        if (s == -1)
                errquit("tcp_connect fail");


        FD_ZERO(&read_fds);

        while(prompt(cBuf)) {

                if (send(s, cBuf, strlen(cBuf)+1, 0) < 0)
                        puts("Error : Write error on socket.");


                arg1 = strtok_r(cBuf, ";", &savepos1);
                while (arg1 != NULL){
                        if((pid = fork()) < 0) {
                                perror("fork error");
                        }
                        else if(pid == 0) {
                                if (arg1[0] == ' ')
                                        arg1 = strtok_r(arg1, " ", &savepos2);
                                strtok_r(arg1, " ", &savepos2);
                                arg2 = strtok_r(NULL, " ", &savepos2);
                                if(arg2 == NULL)
                                        execlp(arg1, arg1, (char*) 0);
                                else {
                                        argarr[0] = arg1;
                                        argnum = 1;
                                        while (arg2 != NULL){
                                                argarr[argnum++] = arg2;
                                                arg2 = strtok_r(NULL, " ", &savepos2);
                                        }
                                        if(strcmp(arg1, "cd") == 0) {
                                                chdir(arg2);
                                                _exit(0);
                                        }
                                        else {
                                                argarr[argnum] = NULL;
                                                execvp(argarr[0], argarr);
                                        }
                                }

                                perror("couldn't execute");
                        }


                        waitpid(pid, &status, 0);
                        arg1 = strtok_r(NULL, ";", &savepos1);
                }
        }


        exit(0);

}

int tcp_connect(int af, char *servip, unsigned short port)
{
        struct sockaddr_in servaddr;
        int  s;


        if ((s = socket(af, SOCK_STREAM, 0)) < 0)
                return -1;


        bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = af;
        inet_pton(AF_INET, servip, &servaddr.sin_addr);
        servaddr.sin_port = htons(port);

        if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                return -1;

        return s;

}
