#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define FILE_NAME_MAX_SIZE 512
#define BUFFER_SIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
	//Set portno from argument
	portno = atoi(argv[2]);

	//1. Initialize sockfd by socket()
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
        error("ERROR opening socket");
    
	//a. Set server(in this assignment we use 'localhost')
	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
	//b. using bzero() set "serv_addr" bit of "serv_addr" to be 0-->initialize serv_addr which is a string
	bzero((char *) &serv_addr, sizeof(serv_addr));

	//c.i. sin_family->represent protocol, AF_UNIX for Unix OS, AF_INET for via internet, and others
    serv_addr.sin_family = AF_INET;

	//c.ii. sin_addr-> represent ip addr, here via a number
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

	//c.iii.sin_port->represent port number,
    serv_addr.sin_port = htons(portno);
    
	//2.Connect(socket,server address, server address,length)
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

	//3. Create a char array to store file name
	char file_name[FILE_NAME_MAX_SIZE+1];
	bzero(file_name,sizeof(file_name));
	printf("Please Input File Name On Server.\n");
	scanf("%s",file_name);

	//4. Send the data in buffer which store the file name
	bzero(buffer,sizeof(buffer));
	strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
	//maybe this send() can't work
	send(sockfd,buffer,BUFFER_SIZE,0);

	//5. Open the file
	FILE *fp = fopen(file_name,"w");
	if(fp==NULL){
		printf("File:\t%sCannot Open To Write!\n",file_name);
		exit(1);
	}
	
	//6. Receive the data from server and store in buffer
	bzero(buffer,sizeof(BUFFER_SIZE));
	int length = 0;
	while(length = recv(sockfd,buffer,BUFFER_SIZE,0)){
		if(length<0){
			printf("Receive Data From Server %s Failed!\n",argv[1]);
			break;
		}

		int write_length = fwrite(buffer,sizeof(char),length,fp);
		if(write_length<length){
			printf("File:\t%sWrite Failed!\n",file_name);
			break;
		}
		bzero(buffer,BUFFER_SIZE);
	}
	
	printf("Receive File:\t%s From Server[%s] Finished!\n",file_name,argv[1]);
	fclose(fp);

	/*
	//3. Connection establish, write()
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    
	//4. Data sent,and some data replied, read() 
	bzero(buffer,256);
	n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    */

	//5.Close()
	close(sockfd);
    return 0;
}
