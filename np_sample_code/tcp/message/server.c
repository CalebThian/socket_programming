/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[BUFFER_SIZE];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

	 //1. Check port whether provided or not
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

	 //2. Initialize sockfd
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     
	 //3. Set up server addr
	 //a. Initialize serv_addr by bzero()
	 bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
	 
	 //b.i. set sin_family as AF_INET,which means via internet
	 serv_addr.sin_family = AF_INET;

	 //b.ii. set sin_addr as INADDR_ANY,which is 0.0.0.0, means listen to any ip connect to current port
     serv_addr.sin_addr.s_addr = INADDR_ANY;

	 //b.iii. set sin_port as given in argv
     serv_addr.sin_port = htons(portno);

	 //4. bind(): bind the socket with socket address,i.e. server address
     if (bind(sockfd,(struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     
	 //5. listen(): listen to the socket
	 listen(sockfd,5);

	 //6. accept(): accept the socket that requested the connection
     clilen = sizeof(cli_addr); 
	 newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");

	
	//7. Create an array to store file_name
	char file_name[FILE_NAME_MAX_SIZE+1];
	bzero(file_name,sizeof(file_name));
	//strncpy(file_name,buffer,strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE+1:strlen(buffer));
	read(newsockfd,file_name,sizeof(file_name));


	FILE *fp = fopen(file_name,"r");
	if(fp == NULL)
		printf("FILE:\t%s Not Found!\n",file_name);
	else{
		bzero(buffer,BUFFER_SIZE);
		int file_block_length = 0;
		while((file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp)) > 0){
			//Start to read the file
			printf("file_block_length == %d\n",file_block_length);
			//Send the string in the buffer to the socket, which is the client
			if(send(newsockfd,buffer,file_block_length,0) < 0){
				printf("Send File:\t%s Failed!\n",file_name);
				break;
			}

			bzero(buffer,sizeof(buffer));
		}

		fclose(fp);
		printf("File:\t%s Transfer Finished!\n",file_name);

		close(newsockfd);
	}




	 /*
	 //7. read(): Read from the socket
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     */
	 close(sockfd);
     return 0;
}
