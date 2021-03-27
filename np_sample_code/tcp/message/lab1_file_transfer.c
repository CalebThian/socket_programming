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

void time_now_by_myself(int* y,int* m,int* d,int* hr,int* min,int* sec){
	time_t seconds = time(NULL);
	time_t minutes = seconds/60;
	seconds %= 60;
	time_t hours = minutes/60;
	minutes %= 60;
	time_t days = hours/24;
	hours %= 24;
	int start_year;
	for(start_year=1970;days>365;start_year++){
		if((start_year%4==0 && start_year%100!=0)||(start_year%100==0 && start_year%400==0))
			days-=366;
		else
			days-=365;
	}
	
	days++;
	int current_month=1;
	int month[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	if((start_year%4==0 && start_year%100!=0)||(start_year%100==0 && start_year%400==0))
		month[2]++;

	for(current_month=1;days>month[current_month-1];++current_month){
		days-=month[current_month-1];
	}

	printf("%ld/%ld/%ld %02d:%02d:%02d",start_year,current_month,days,hours,minutes,seconds);
	*y = start_year;
	*m = current_month;
	*d = days;
	*hr = hours;
	*min = minutes;
	*sec = seconds;
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
				int y,m,d,hr,min,sec;
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
						//printf("%ld/%ld/%ld %02d:%02d:%02d",y,m,d,hr,min,(int)((double)sec+(double)(clock()-start)/CLOCKS_PER_SEC));
						//time_now(&y,&m,&d,&hr,&min,&sec);
						time_now();
						gap+=0.25;
					}
					//printf("file_block_length == %d\n",file_block_length);
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
	}
}
