#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define T_BUFFER_SIZE 256
#define FILE_NAME_MAX_SIZE 512
#define BUFFER_SIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void time_now(){
	char t_buf[T_BUFFER_SIZE]={0};
	time_t rawtime = time(NULL);
	struct tm *ptm = localtime(&rawtime);
	strftime(t_buf,T_BUFFER_SIZE,"%Y/%m/%d %X",ptm);
	puts(t_buf);
}

int main(int argc, char *argv[])
{
    if(strcmp(argv[1],"tcp")==0){
		if(strcmp(argv[2],"recv")==0){
			int sockfd, portno, n;
			struct sockaddr_in serv_addr;
			struct hostent *server;
			char buffer[BUFFER_SIZE];
			char file_name_copy[FILE_NAME_MAX_SIZE];
			if(argc < 5) {
	       		fprintf(stderr,"usage %s tcp recv ip port\n", argv[0]);
	       		exit(0);
			}
	    
	    	//Set portno from argument
			portno = atoi(argv[4]);

            //1. Initialize sockfd by socket()
	    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    	if (sockfd < 0) 
				error("ERROR opening socket");
	    
			//a. Set server(in this assignment we use 'localhost')
			server = gethostbyname(argv[3]);
	    	if (server == NULL) {
				fprintf(stderr,"ERROR, no such host\n");
				exit(0);
	   		}
	    
			//b. using bzero() set "serv_addr" bit of "serv_addr" to be 0-->initialize serv_addr which is a string
			bzero((char *) &serv_addr, sizeof(serv_addr));

			//c.i. sin_family->represent protocol, AF_UNIX for Unix OS, AF_INET for via internet, and others
	    	serv_addr.sin_family = AF_INET;

			//c.ii. sin_addr-> represent ip addr, here via a number
	    	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);

			//c.iii.sin_port->represent port number,
	    	serv_addr.sin_port = htons(portno);
	    
			//2.Connect(socket,server address, server address,length)
			if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
				error("ERROR connecting");

			//3.a. Create a char array to store file name
			char file_name[FILE_NAME_MAX_SIZE+1];
			bzero(file_name,sizeof(file_name));
			read(sockfd,file_name,BUFFER_SIZE);
			
			//3.b. Inform server that file name receive
			bzero(buffer,sizeof(BUFFER_SIZE));
			strcpy(buffer,"File name received!");
			write(sockfd,buffer,BUFFER_SIZE);

			//3.b. Set new file name
			bzero(file_name_copy,sizeof(file_name_copy));
			strcat(file_name_copy,"copy");
			strcat(file_name_copy,strchr(file_name,'.'));
			printf("The new file name is %s\n",file_name_copy);

			//4. Open the file
			FILE *fp = fopen(file_name_copy,"w");
			if(fp==NULL){
				printf("File:\t%sCannot Open To Write!\n",file_name);
				exit(1);
			}
	
			//5. Receive the data from server and store in buffer
			bzero(buffer,sizeof(BUFFER_SIZE));
			int length = 0;
			while(length = read(sockfd,buffer,BUFFER_SIZE)){
				if(length<0){
					//printf("Receive Data From Server %s Failed!\n",argv[1]);
					continue;
				}
				int write_length = fwrite(buffer,sizeof(char),length,fp);
				if(write_length<length){
					printf("File:\t%sWrite Failed!\n",file_name_copy);
					break;
				}
				bzero(buffer,BUFFER_SIZE);
			}
	
			printf("Receive File:\t%s From Server[%s] Finished!\n",file_name,argv[4]);
			fclose(fp);
	    
			//6.Close()
			close(sockfd);
	    	return 0;
		}else if(strcmp(argv[2],"send")==0){
			
			int sockfd, newsockfd, portno;
     		socklen_t clilen;
     		char buffer[BUFFER_SIZE];
     		struct sockaddr_in serv_addr, cli_addr;
     		int n,flen,ftotal;

			//1. Check port whether provided or not
     		if (argc < 6) {
         		fprintf(stderr,"usage %s tcp send ip port filename\n",argv[0]);
         		exit(1);
     		}

	 		//2. Initialize sockfd
     		sockfd = socket(AF_INET, SOCK_STREAM, 0);
     		if (sockfd < 0)
        		error("ERROR opening socket");
     
	 		//3. Set up server addr
	 		//a. Initialize serv_addr by bzero()
	 		bzero((char *) &serv_addr, sizeof(serv_addr));
    	 	portno = atoi(argv[4]);
	 
	 		//b.i. set sin_family as AF_INET,which means via internet
	 		serv_addr.sin_family = AF_INET;

	 		//b.ii. set sin_addr as INADDR_ANY,which is 0.0.0.0, means listen to any ip connect to current port
     		serv_addr.sin_addr.s_addr = INADDR_ANY;

	 		//b.iii. set sin_port as given in argv
     		serv_addr.sin_port = htons(portno);

	 		//4. bind(): bind the socket with socket address,i.e. server address
     		if (bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
             	error("ERROR on binding");
     
	 		//5. listen(): listen to the socket
	 		listen(sockfd,5);

	 		//6. accept(): accept the socket that requested the connection
     		clilen = sizeof(cli_addr); 
	 		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
     		if (newsockfd < 0)
          		error("ERROR on accept");

	
			//7. Create an array to store file_name and send it
			char file_name[FILE_NAME_MAX_SIZE+1];
			bzero(file_name,sizeof(file_name));
			strncpy(file_name,argv[5],strlen(argv[5])>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE+1:strlen(argv[5]));
			write(newsockfd,file_name,sizeof(file_name));
			
			bzero(buffer,sizeof(buffer));
			read(newsockfd,buffer,sizeof(buffer));
			printf("Receive message:%s\n",buffer);

			FILE *fp = fopen(file_name,"r");
			if(fp == NULL)
				printf("FILE:\t%s Not Found!\n",file_name);
			else{
				//Calculate the filesize
				fseek(fp,0L,SEEK_END);
				flen = ftell(fp);
				fseek(fp,0L,SEEK_SET);
				
				bzero(buffer,BUFFER_SIZE);
				int file_block_length = 0;
				double percent= .0;
				double gap = 0.25;

				//Calculate time_now
				printf("0%% ");
				time_now();
				clock_t start = clock();
				while((file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp)) > 0){
					//Start to read the file
					ftotal += file_block_length;
					if((double)ftotal/(double)flen>gap){
						printf("%d%% ",(int)(gap*100));
						time_now();
						gap+=0.25;
					}
					//Send the string in the buffer to the socket, which is the client
					if(write(newsockfd,buffer,file_block_length) < 0){
						printf("Send File:\t%s Failed!\n",file_name);
						break;
					}
					bzero(buffer,sizeof(buffer));
				}
				clock_t end = clock();
				printf("\nTotal trans time: %lfms\n",(double)(end-start)*1000/CLOCKS_PER_SEC);
				printf("file size : %.1fMB\n",(double)flen/1000/1000);
				fclose(fp);
				printf("File:\t%s Transfer Finished!\n",file_name);

				close(newsockfd);
			}
	 		close(sockfd);
     		return 0;
		}
	}else if(strcmp(argv[1],"udp")==0){
		//1. Initialize socket(common)
		int sock;
		if((sock = socket(PF_INET,SOCK_DGRAM,0))<0)
			error("socket error");

		//Send or recv
		if(strcmp(argv[2],"send")==0){
			//2. Set up servaddr
			struct sockaddr_in servaddr;
			memset(&servaddr,0,sizeof(servaddr));
			//2.A-C. Set sin_family,sin_port and sin_addr
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(atoi(argv[4]));
			servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

			//3.bind() with the servaddr
			if(bind(sock,(struct sockaddr*)&servaddr,sizeof(servaddr)))
					error("bind error");
			
			//4.Declare sockaddr_in and other varibles using for tranfering file
			char buffer[BUFFER_SIZE];
			char file_name[FILE_NAME_MAX_SIZE];
			struct sockaddr_in peeraddr;
			socklen_t peerlen;
			int n,flen,ftotal;
					
			//5. Set the name of file
			bzero(file_name,sizeof(file_name));
			strncpy(file_name,argv[5],strlen(argv[5])>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE+1:strlen(argv[5]));
				
			//6. Transfer the file if any client is ready build
			while(1){
				// Initialize peerlen,recvbuf
				peerlen = sizeof(peeraddr);
				memset(buffer,0,sizeof(buffer));

				//Receive message if there is any
				n = recvfrom(sock,buffer,sizeof(buffer),0,
						(struct sockaddr *)&peeraddr, &peerlen);
				if(n==-1){
					if(errno == EINTR)
						continue;
					error("recvfrom error");
				}else{
					FILE *fp = fopen(file_name,"r");
					if(fp == NULL)
						printf("FILE:\t%s Not Found!\n",file_name);
					else{
						//Calculate the filesize
						fseek(fp,0L,SEEK_END);
						flen = ftell(fp);
						fseek(fp,0L,SEEK_SET);
					
						char inform_text[] = "Start transfering ";
						bzero(buffer,sizeof(BUFFER_SIZE));
						strncpy(buffer,inform_text,strlen(inform_text));
						strcat(buffer,file_name);
						sendto(sock,buffer,sizeof(buffer),0,
								(struct sockaddr*)&peeraddr,peerlen);
						printf("%s\n",buffer);
						
						//Receive the new file name
						bzero(buffer,sizeof(buffer));
						recvfrom(sock,buffer,sizeof(buffer),0,
								(struct sockaddr *)&peeraddr, &peerlen);
							fputs(buffer,stdout);
						
						bzero(buffer,sizeof(buffer));
						bzero(buffer,BUFFER_SIZE);
						int file_block_length = 0;
						double percent= .0;
						double gap = 0.25;

						//Calculate time_now
						printf("\n0%% ");
						time_now();
						clock_t start = clock();
						while((file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp)) > 0){
						//Start to read the file
							ftotal += file_block_length;
							if((double)ftotal/(double)flen>gap){
								printf("%d%% ",(int)(gap*100));
								time_now();
								gap+=0.25;
							}
						//Send the string in the buffer to the socket, which is the client
							if(sendto(sock,buffer,file_block_length,0,(struct sockaddr *)&peeraddr,peerlen) < 0){
								printf("Send File:\t%s Failed!\n",file_name);
								break;
							}
							bzero(buffer,sizeof(buffer));
						}
						clock_t end = clock();
						printf("\nTotal trans time: %lfms\n",(double)(end-start)*1000/CLOCKS_PER_SEC);
						printf("file size : %.1fMB\n",(double)flen/1000/1000);
						fclose(fp);
						printf("File:\t%s Transfer Finished!\n",file_name);
						
						//End transfer file
						bzero(buffer,sizeof(buffer));
						sendto(sock,buffer,0,0,(struct sockaddr *)&peeraddr,peerlen);

						//Receive success transfer file size and calculate file loss rate
						recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&peeraddr,&peerlen);
						int fsuc = atoi(buffer);
						printf("File Loss Rate = %.2lf%%\n",(double)fsuc/(double)flen*100);

						close(sock);
						break;
					}
				}
			}
		}else if(strcmp(argv[2],"recv")==0){
			//2. Initialize servaddr and some other variables
			struct hostent *server;
			server = gethostbyname(argv[3]);
	    	if (server == NULL) {
				fprintf(stderr,"ERROR, no such host\n");
				exit(0);
	   		}

			struct sockaddr_in servaddr;
			memset(&servaddr,0,sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(atoi(argv[4]));
	    	bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr,server->h_length);
		
			int ret,n;
			char sendbuf[BUFFER_SIZE] = {0};
			char recvbuf[BUFFER_SIZE] = {0};
			char file_name[FILE_NAME_MAX_SIZE+1]={0};
			printf("Please enter any string if ready to receive file...\n");
				
			//If get any from terminal,especially "Ready"
			while(n=fgets(sendbuf,sizeof(sendbuf),stdin) != NULL){
				sendto(sock,sendbuf,strlen(sendbuf),0,(struct sockaddr*)&servaddr,sizeof(servaddr));
				
				//Receive file name from server
				ret = recvfrom(sock,recvbuf,sizeof(recvbuf),0,NULL,NULL);
				if(ret == -1){
					if(errno == EINTR)
						continue;
					error("recvfrom");
				}
				
				printf("%s\n",recvbuf);

				//Send the new file name
				strcpy(file_name,"copy");
				strcat(file_name,strchr(recvbuf,'.'));

				memset(sendbuf,0,sizeof(sendbuf));
				strcpy(sendbuf,"Receive the file name,the new file name will be ");
				strcat(sendbuf,file_name);

				printf("%s\n",sendbuf);
				sendto(sock,sendbuf,strlen(sendbuf),0,
							(struct sockaddr*)&servaddr,sizeof(servaddr));
				bzero(sendbuf,sizeof(sendbuf));
				bzero(recvbuf,sizeof(recvbuf));


				//Open the file to write
				FILE *fp = fopen(file_name,"w");
				if(fp==NULL){
					printf("File:\t%sCannot Open To Write!\n",file_name);
					exit(1);
				}
		
				//5. Receive the data from server and store in buffer
				int length = 0;
				while(length = recvfrom(sock,recvbuf,BUFFER_SIZE,0,NULL,NULL)){
					if(length<=0){
						break;
					}
					int write_length = fwrite(recvbuf,sizeof(char),length,fp);
					if(write_length<length){
						printf("File:\t%sWrite Failed!\n",file_name);
						break;
					}
					bzero(recvbuf,BUFFER_SIZE);
				}
				printf("Successfully receive the file!\n");	

				//Send final file size
				fseek(fp,0L,SEEK_SET);
				fseek(fp,0L,SEEK_END);
				int flen = ftell(fp);
				bzero(sendbuf,BUFFER_SIZE);
				sprintf(sendbuf,"%d",flen);
				sendto(sock,sendbuf,BUFFER_SIZE,0,
						(struct sockaddr*)&servaddr,sizeof(servaddr));

				fclose(fp);
			
				//6.Close()
				close(sock);
				break;
			}
		}
		return 0;
	}
}
