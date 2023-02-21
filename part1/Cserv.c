/*
 A simple echo server program 
 Kasidit Chanchio (kasiditchanchio@gmail.com)
 Modify by Tharit Kaeothong 6309681804 & Naphatsawan Sukuntapree 6309681473
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

struct hnode {
    int client;
    int length;
    char *line;
    struct hnode* next;
};
struct hnode* last = NULL;
struct hnode* head = NULL;
void addatlast(int client,int length,char *line)
{
    // Initialize a new node
    struct hnode* temp;
    temp = (struct hnode*)malloc(sizeof(struct hnode));
  
    // If the new node is the
    // only node in the list
    if (last == NULL) {
        temp->line = malloc(sizeof(length));
        temp->line = memcpy(temp->line,line,length);
        temp->length = length;
        temp ->client = client;
        temp->next = temp;
        head = temp;
        last = temp;
    }
  
    // Else the new node will be the
    // last node and will contain
    // the reference of head node
    else {
        temp->line = malloc(sizeof(length));
        temp->line = memcpy(temp->line,line,length);
        temp->length = length;
        temp ->client = client;
        temp->next = last->next;
        last->next = temp;
        last = temp;

    }
}

void deletefirst()
{
    struct hnode* temp;
  
    // If list is empty
    if (last == NULL)
        printf("\nList is empty.\n");
  
    // Else last node now contains
    // reference of the second node
    // in the list because the
    // list is circular
    else {
        temp = last->next;
        last->next = temp->next;
        free(temp);
    }
}

void viewList()
{
    // If list is empty
    if (last == NULL)
        printf("\nList is empty\n");

    // Else print the list
    else {
        struct hnode* temp;
        temp = last->next;
        do {
            printf("\nClient ID = %d Length = %d Data = %s", temp->client , temp->length , temp->line);
            fflush(stdout);
            temp = temp->next;
        } while (temp != last->next);
    }
}

int main(int argc, char *argv[]){

    int m, n, i, j;
    char line[MAXLINE];
    int conn_fd[MAXCONN];   //list of Client connect
    int conn_id[MAXCONN];   //list of ID client
    char id[500];
    int cindex = 0;
    int ptrlen = 0;

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

                                ptrlen++;
                                if(ptrlen>10) //Check link list =! 10
                                    deletefirst();
                                int lenline = strlen(line);
                                addatlast(conn_id[j],lenline,line);
                                viewList();

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