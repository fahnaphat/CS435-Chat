/*
 A simple TCP client program 
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

#define SERV_IP		"127.0.0.1"
#define SERV_PORT 	18800

#define	MAXLINE	100

int accept_cr(int fd, struct sockaddr *addr, socklen_t *len);
int write_full(int fd, const void *buf, size_t count);
int read_full(int fd, void *buf, size_t count);

int client_shutdown_flag = 0;

int conn_fd;
struct sockaddr_in serv_addr;

int main(int argc, char *argv[]){

    int n, m;
	fd_set base_rfds, rfds; 
    int fdmax = 0; 
    char line[MAXLINE];

	conn_fd = socket(AF_INET, SOCK_STREAM, 0); 
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = inet_addr(SERV_IP);

	//The client requests a connection to the destination server.
    if (connect(conn_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
        perror("Problem in connecting to the server");
        exit(3);
    }

	FD_ZERO(&base_rfds);
	FD_ZERO(&rfds);

	FD_SET(fileno(stdin), &base_rfds); //read data from the keyboard and then add into base_rfds.
	FD_SET(conn_fd, &base_rfds);	   //add the value obtained from accepting the connection into base_rfds.

	fdmax = conn_fd;

	while(1){

		//Copy base_rfds to rfds
		memcpy(&rfds, &base_rfds, sizeof(fd_set));

		//Choose select system call for look at the least one event of interest in rfds
		if (select(fdmax+1, &rfds, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(4);
		}
	 	
	 	//Check input on keyborad whether it exists in rfds. 
	  	if(FD_ISSET(fileno(stdin), &rfds)){

	  		//Use fgets() to receive values from the keyboard and store them in the variable 'line'.
	        if(fgets(line, MAXLINE, stdin) == NULL){

	        	//If 'line' is equal to NULL, it means that EOF has been received.
			    printf("\nShutdown writing to TCP connection\n");

			    //Instruct the client to close the connection to the server using the shutdown system call.
			    shutdown(conn_fd, SHUT_WR);
			    client_shutdown_flag = 1;
		    }
		    else{

		    	//If there is data coming from the keyboard, send it to the server.
	            n = write_full(conn_fd, line, MAXLINE);
		    }
	  	}

	  	//Check if there is an event of interest occurring on conn_fd or not.
		if(FD_ISSET(conn_fd, &rfds)){

			//Read the data received from the server.
			if((m = read_full(conn_fd, line, MAXLINE)) == 0){
				if(client_shutdown_flag){
					printf("TCP connection closed after client shutdown\n");
					close(conn_fd);
					break;
				}
				else{
					printf("Error: TCP connection closed unexpectedly\n");
					exit(1);
				}
			}
			else{
				
				//Read data from the server and display it on the client-side screen.
				fputs(line, stdout);
				fflush(stdout);
			}
		}
	}
}

// -----Basic Communication Utilities-----
int accept_cr(int fd, struct sockaddr *addr, socklen_t *len){
	int ret;
repeat_accept:
        ret = accept(fd, addr, len);
        if (ret < 0) {
            if (errno == EINTR){
                goto repeat_accept;
	    	}
	    printf("accept error errno=%d fd=%d\n", errno, fd);
        }
	return ret;
}

int write_full(int fd, const void *buf, size_t count){
    ssize_t ret = 0;
    ssize_t total = 0;

    while (count) {
        ret = write(fd, buf, count);
        if (ret < 0) {
            if (errno == EINTR){
                continue;
	    	}
	    	printf("write error errno=%d fd=%d\n", errno, fd);
            return ret;
        }
        else if (ret == 0){
            return ret; 
        }

        count -= ret;
        buf += ret;
        total += ret;
    }

    return total;
}

int read_full(int fd, void *buf, size_t count){
    ssize_t ret = 0;
    ssize_t total = 0;

    while (count) {
        ret = read(fd, buf, count);
        if (ret < 0) {
            if (errno == EINTR){ 
                continue;
	    	} 
	    	printf("read error errno=%d fd=%d\n", errno, fd);
            return ret;
        }
        else if (ret == 0){
            return ret; 
        }

        count -= ret;
        buf += ret;
        total += ret;
    }

    return total;
}
