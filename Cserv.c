/*
 A simple echo server program 
 Kasidit Chanchio (kasiditchanchio@gmail.com)
*/
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_IP     "127.0.0.1"
#define SERV_PORT   18800

#define MAXLINE 100

#include <sys/select.h>

int lis_fd;
int conn_fd;
struct sockaddr_in serv_addr;

#define MAXCONN 100

int main(int argc, char *argv[]){

    int m, n, i, j;
    char line[MAXLINE];
    int conn_fd[MAXCONN];   //list of Client connect
    int conn_id[MAXCONN];   //ID of connect
    char id[500];
    int cindex = 0;

    fd_set base_rfds;
    fd_set rfds;
    int fdmax;

    lis_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);

    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(lis_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(lis_fd, 5);

    FD_ZERO(&base_rfds);
    FD_ZERO(&rfds);

    FD_SET(lis_fd, &base_rfds);
    fdmax = lis_fd;

    for(;;) {
        memcpy(&rfds, &base_rfds, sizeof(fd_set));
        if(select(fdmax+1, &rfds, NULL, NULL, NULL) < 0){
            printf("select error!\n");
            fflush(stdout);
            exit(1);
        }

        for(i = 0; i <= fdmax; i++){

            if(FD_ISSET(i, &rfds)){

                //i is listening socket
                if(i == lis_fd){

                    //accepted connect Client and Add Client in list
                    if((conn_fd[cindex] = accept(lis_fd, NULL, NULL)) < 0){
                        printf("Accept: Error occured\n");
                        exit(1);
                    }


                    printf("a new connection %d is made!\n", conn_fd[cindex]);

                    conn_id[cindex] = rand() % 1000;
                    sprintf(id, "cli-%d:> ", conn_id[cindex]);
                    // sprintf(id, "%d", conn_id[cindex]);
                    write(conn_fd[cindex], id, 100);

                    FD_SET(conn_fd[cindex] , &base_rfds); 
                    if(conn_fd[cindex] > fdmax){
                        fdmax = conn_fd[cindex];
                    }
                    cindex++; 
                } 
                else{

                    n = read(i, line, MAXLINE);

                    if(n <= 0){
                        if(n == 0){
                            printf("read: close connection %d\n", i);
                            FD_CLR(i, &base_rfds);
                            close(i);

                            for(j = 0; j <= cindex; j++){
                                if(conn_fd[j] == i){
                                    conn_fd[j] = -1;
                                }
                            }

                        }
                        else{
                            printf("read: Error occured\n");
                            exit(1);
                        }
                    }
                    else{
                        printf("line = %s with n = %d charecters\n", line, n);
                        fflush(stdout);
                        int id_anoClient = -1;

                        for(j = 0; j < cindex; j++){

                            // sprintf(id, "cli-%d:>", conn_id[cindex]);
                            // write(conn_fd[cindex], id, 100);
                            if(conn_fd[j] == i){
                                id_anoClient = conn_id[j];
                                sprintf(id, "cli-%d:> ", conn_id[j]);
                                m = write(conn_fd[j], id, n); //change 100 to MAXLINE
                                printf("(Talk) write line = %s for m = %d charecters\n", line, m);
                                fflush(stdout);
                            }
                        }

                        for(j = 0; j < cindex; j++){
                            if(conn_fd[j] != -1 && conn_fd[j] != i){
                                sprintf(id, "\ncli-%d says: %s", id_anoClient, line);
                                m = write(conn_fd[j], id, n);
                                sprintf(id, "cli-%d:> ", conn_id[j]);
                                write(conn_fd[j], id, n);

                                printf("write line = %s for m = %d charecters\n", line, m);
                                fflush(stdout);
                            }
                        }

                    }

                }
            }

        }


    }
    close(lis_fd);
}