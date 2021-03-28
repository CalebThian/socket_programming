/*************************************************************************
    > File Name: echocli_udp.c
    > Author: Simba
    > Mail: dameng34@163.com
    > Created Time: Sun 03 Mar 2013 06:13:55 PM CST
 ************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

void echo_cli(int sock)
{
	//In echo_cli, initialize servaddr,include sin_family,sin_port, and sin_addr
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    int ret;
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
	//If get any from terminal
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
		//Send to server
        sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		
		//Receive something from server
        ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
		if (ret == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }

		//Print out what have received from server
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    close(sock);
}

int main(void)
{
	//1. Initialize socket
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
	
	//2. Call echo_cli()
    echo_cli(sock);

    return 0;
}
