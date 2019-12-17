#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define MAXLINE  511
#define MAX_SOCK 1024

int maxfdp1;
int num_user = 0;
int num_chat = 0;
int clisock_list[MAX_SOCK];
char ip_list[MAX_SOCK][20];
int listen_sock;
int commcnt = 0;

void addClient(int s, struct sockaddr_in *newcliaddr);
int getmax();
void removeClient(int s);
int tcp_listen(int host, int port, int backlog);

void errquit(char *mesg) { perror(mesg); exit(1); }

time_t ct;
struct tm tm;

typedef struct command{
        char syscall[100];
        int count;
        struct command *nextcomm;
}command;

command *comm = NULL;

void printranking(){
        int i = 0;
        command temp;
        command *ptr;
        command *rank = malloc(commcnt * sizeof(command));
        ptr = comm;

        while (ptr != NULL)
        {
                strcpy(rank[i].syscall, ptr->syscall);
                rank[i].count = ptr->count;
                i++;
                ptr = ptr->nextcomm;
        }


        for(int j = 0; j < i ; j++){
                for(int k = 0 ; k < i -1 ; k++)
                {
                        if(rank[k].count<rank[k+1].count)
                        {
                                temp = rank[k];
                                rank[k] = rank[k+1];
                                rank[k+1] = temp;
                        }
                }

        }

        for(int j = 0 ; j < commcnt ; j++)
                printf("Rank[%d] = %s, count = %d \n", j + 1, rank[j].syscall, rank[j].count);


}

void printselect(char input[]){
        command *ptr;

        ptr = comm;

        while (ptr != NULL){
                if (strcmp(ptr->syscall, input) == 0){
                        printf("The number of \"%s\" is %d.\n", ptr->syscall, ptr->count);
                        break;
                }
                ptr = ptr->nextcomm;
        }
}

void *thread_function(void *arg)
{
        char input[100];

        printf("option : print ranking(r), command search(s)\n\n");
        while (1)
        {
                char bufmsg[MAXLINE + 1];
                fflush(stdin);
                fgets(bufmsg, MAXLINE, stdin);
                if (!strcmp(bufmsg, "\n")) continue;
                else if (!strcmp(bufmsg, "r\n")){
                        printf("--------------------RANKING OF COMMAND--------------------\n");
                        printranking();
                        printf("----------------------------------------------------------\n\n");
                }
                else if (!strcmp(bufmsg, "s\n")){
                        printf("-----------------------SEARCH MODE-----------------------\n");
                        printf("What is the command you're looking for? => ");
                        scanf("%s", input);
                        printselect(input);
                        printf("---------------------------------------------------------\n\n");
                }
                else
                        printf("invalid input : write again\n");
        }
}

int main(int argc, char *argv[])
{
        struct sockaddr_in cliaddr;
        char buf[MAXLINE + 1];
        int i, j, nbyte, accp_sock;
        socklen_t addrlen = sizeof(struct sockaddr_in);
        fd_set read_fds;
        pthread_t a_thread;
        char *savepos1, *savepos2;
        char *arg1;
        command *newcomm;
        command *ptr, *pptr;
        int check = 0;

        if (argc != 2)
        {
                printf("Retry!\n");
                exit(0);
        }

        listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

        pthread_create(&a_thread, NULL, thread_function, (void *)NULL);
        while (1)
        {
                FD_ZERO(&read_fds);
                FD_SET(listen_sock, &read_fds);
                for (i = 0; i < num_user; i++)
                        FD_SET(clisock_list[i], &read_fds);
                maxfdp1 = getmax() +1 ;
                if (select(maxfdp1, &read_fds, NULL, NULL, NULL) < 0)
                        errquit("select fail");

                if (FD_ISSET(listen_sock, &read_fds)) {
                        accp_sock = accept(listen_sock,(struct sockaddr*)&cliaddr, &addrlen);
                        if (accp_sock == -1) errquit("accept fail");
                        addClient(accp_sock, &cliaddr);
                        ct = time(NULL);
                        tm = *localtime(&ct);
                        write(1, "\033[0G", 4);
                        printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
                        printf("number of students = %d\n", num_user);
                }


                for (i = 0; i < num_user; i++)
                {
                        if (FD_ISSET(clisock_list[i], &read_fds))
                        {
                                num_chat++;
                                nbyte = recv(clisock_list[i], buf, MAXLINE, 0);

                                arg1 = strtok_r(buf, ";", &savepos1);
                                while (arg1 != NULL){
                                        if (arg1[0] == ' ')
                                                arg1 = strtok_r(arg1, " ", &savepos2);

                                        arg1 = strtok_r(arg1, " ", &savepos2);
                                        if (arg1 == NULL){
                                                arg1 = strtok_r(NULL, ";", &savepos1);
                                                continue;
                                        }

                                        newcomm = (command *)malloc(sizeof(command));
                                        strcpy(newcomm->syscall, arg1);
                                        newcomm->count = 1;
                                        newcomm->nextcomm = NULL;

                                        if (comm == NULL){
                                                comm = newcomm;
                                                commcnt++;
                                        }
                                        else {
                                                check = 0;
                                                ptr = comm;
                                                while (ptr != NULL){
                                                        if (strcmp(newcomm->syscall, ptr->syscall) == 0){
                                                                (ptr->count)++;
                                                                check = 1;
                                                                break;
                                                        }
                                                        pptr = ptr;
                                                        ptr = ptr->nextcomm;
                                                }
                                                if (check != 1){
                                                        pptr->nextcomm = newcomm;
                                                        commcnt++;
                                                }

                                        }
                                        arg1 = strtok_r(NULL, ";", &savepos1);
                                }
                                if (nbyte <= 0)
                                {
                                        removeClient(i);
                                        continue;
                                }


                                for (j = 0; j < num_user; j++)
                                        send(clisock_list[j], buf, nbyte, 0);

                        }
                }
        }

        return 0;
}

void addClient(int s, struct sockaddr_in *newcliaddr)
{
        char buf[20];
        inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf));
        write(1, "\033[0G", 4);
        clisock_list[num_user] = s;
        strcpy(ip_list[num_user], buf);
        num_user++;
}


void removeClient(int s)
{
        close(clisock_list[s]);
        if (s != num_user - 1)
        {
                clisock_list[s] = clisock_list[num_user - 1];
                strcpy(ip_list[s], ip_list[num_user - 1]);
        }
        num_user--;
        ct = time(NULL);
        tm = *localtime(&ct);
        write(1, "\033[0G", 4);
        printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
        printf("number of students = %d\n", num_user);

}

int getmax()
{

        int max = listen_sock;
        int i;
        for (i = 0; i < num_user; i++)
                if (clisock_list[i] > max)
                        max = clisock_list[i];
        return max;
}


int  tcp_listen(int host, int port, int backlog)
{
        int sd;
        struct sockaddr_in servaddr;

        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == -1)
        {
                perror("socket fail");
                exit(1);
        }

        bzero((char *)&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(host);
        servaddr.sin_port = htons(port);
        if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
                perror("bind fail");  exit(1);
        }

        listen(sd, backlog);
        return sd;
}
