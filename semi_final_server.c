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

void addClient(int s, struct sockaddr_in *newcliaddr);
int getmax();
void removeClient(int s);
int tcp_listen(int host, int port, int backlog);

void errquit(char *mesg) { perror(mesg); exit(1); }

time_t ct;
struct tm tm;

void *thread_function(void *arg)
{
        printf("option : print ranking(r), command select(s)\n");
        while (1)
        {
                char bufmsg[MAXLINE + 1];
                fgets(bufmsg, MAXLINE, stdin);
                if (!strcmp(bufmsg, "\n")) continue;
                else if (!strcmp(bufmsg, "r\n"));
                else if (!strcmp(bufmsg, "s\n"));
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
                                printf("1 :%s\n", buf);
                                strcpy(buf,"");
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
