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
    int conn_id[MAXCONN];   //list of ID client
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

        //Copy base_rfds to rfds
        memcpy(&rfds, &base_rfds, sizeof(fd_set));

        //Choose select system call for look at the least one event of interest in rfds
        if(select(fdmax+1, &rfds, NULL, NULL, NULL) < 0) {
            printf("select error!\n");
            fflush(stdout);
            exit(1);
        }

        for(i = 0; i <= fdmax; i++) {

            //Check the value of i whether it exists in rfds. 
            //Return 1 when the value of i exists in rfds.
            if(FD_ISSET(i, &rfds)){

                //if i is listening socket
                if(i == lis_fd){

                    //accepted connect Client and Add Client in list
                    if((conn_fd[cindex] = accept(lis_fd, NULL, NULL)) < 0){
                        printf("Accept: Error occured\n");
                        exit(1);
                    }

                    //Randomly assign an ID to the client that has been connected.
                    conn_id[cindex] = rand() % 1000;

                    printf("A new client (ID:%d) has connected!\n", conn_id[cindex]);

                    //Send a message with the ID number to the client immediately after a successful connection.
                    sprintf(id, "cli-%d:> ", conn_id[cindex]);
                    write(conn_fd[cindex], id, 100);

                    //Set the value obtained from opening a new socket and store it in the base rfds.
                    FD_SET(conn_fd[cindex] , &base_rfds); 
                    if(conn_fd[cindex] > fdmax){
                        fdmax = conn_fd[cindex];
                    }
                    cindex++; 
                } 
                else{

                    //In the case of an Active socket.
                    //Read the data sent by the client.
                    n = read(i, line, MAXLINE);

                    if(n <= 0){
                        if(n == 0){
                            //EOF case
                            printf("read: close connection %d ", i);
                            FD_CLR(i, &base_rfds);
                            close(i);

                            //The client has ended the conversation. Similar to logging out.
                            for(j = 0; j <= cindex; j++){
                                if(conn_fd[j] == i){
                                    printf("(Client (ID:%d) Logout)\n", conn_id[j]);
                                    conn_fd[j] = -1;
                                    conn_id[j] = -1;
                                }
                            }

                        }
                        else{
                            printf("read: Error occured\n");
                            exit(1);
                        }
                    }
                    else{

                        //Print the data that has been read from the client. (the server side)
                        printf("line = %s with n = %d charecters\n", line, n);
                        fflush(stdout);

                        //The variable that specifies which client has sent data to the server.
                        int id_anoClient = -1;

                        //Loop to find the client who has sent data.
                        for(j = 0; j < cindex; j++){
                            if(conn_fd[j] == i){

                                id_anoClient = conn_id[j];      //Assign the client ID to id_anoClient.

                                //Send the ID back to the client who sent the message.
                                sprintf(id, "cli-%d:> ", conn_id[j]);
                                m = write(conn_fd[j], id, n);

                                //Print the message sent by the client (display on the server side).
                                printf("(Talk) write line = %s for m = %d charecters\n", line, m);
                                fflush(stdout);
                            }
                        }

                        //Loop to find the remaining clients to send the message.
                        for(j = 0; j < cindex; j++){

                            //However, do not send the message back to the client who sent the message to the server.
                            if(conn_fd[j] != -1 && conn_fd[j] != i){

                                //Send the received message to other clients.
                                sprintf(id, "\ncli-%d says: %s", id_anoClient, line);
                                m = write(conn_fd[j], id, n);

                                //Send the ID back to the client.
                                sprintf(id, "cli-%d:> ", conn_id[j]);
                                write(conn_fd[j], id, n);

                                //Print the message sent by the client (display on the server side).
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