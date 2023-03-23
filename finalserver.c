#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h> 
#include <signal.h>
#include <errno.h>
#define MSGSIZE 5

struct control {
   int dsn; //Data sequence number
   int ssn; //Subflow Sequence Number
   int dataSize;
   int port_no;
};//32 bits length

struct packet
{
    char* data;
};



int main(int argc, char *argv[]) {
    
    //file descriptors
	int sockfd, ctrlfd;

     

    //IP address and port details are stored here
    struct sockaddr_in ctrlAddr, client;
    struct sockaddr_in* subflowaddr[3];
    int size = sizeof(struct sockaddr_in);

    //subflow pipes and processes
    int sflowPipes[3][2];// stores the pipe
    int sflowProcesses[3]; // stores the processes of 3 sub flows
    int sflowfd[3];// stores the file descriptors of subflows


    //buffer for storing data
    char* allData = (char*)malloc(993 * sizeof(char));
    char* buffer = "";
    int i, pos = 0;


    //packet details
    struct packet* pack = (struct packet*)malloc(sizeof(struct packet));
    pack->data =(char*)malloc(4 * sizeof(char));
    


    //temporary fd
    int tempfd, temp;

    //data for pid
    int pid[3];




    //Create control socket ,bind  and listen
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket creation failed");
        exit(0);
    }
    assign(&ctrlAddr, 6377);
    //creating bind and checking for errors
    int b = bind(sockfd, (struct sockaddr *)&ctrlAddr, sizeof(struct sockaddr));
    if (b == -1)
    {
        perror("bind failed");
        exit(0);
    }
    listen(sockfd, 4);
    //accept control connection and other connections from subflows
    ctrlfd = accept(sockfd, (struct sockaddr *)&ctrlAddr, &size);
	if (ctrlfd  == -1)
	{
		perror("\naccept failed\n");
		exit(0);
	}
	printf("control connection 1 is set up sccessfully");
   
    

   pipe(sflowPipes[0]);
   pid[0]= fork();
   if(pid[0]==0) {
       close(sflowPipes[0][1]);
        int ctrlSock0 = socket(AF_INET, SOCK_STREAM, 0);
        assign(&subflowaddr[0],63456);
        b = bind(ctrlSock0, (struct sockaddr *)&subflowaddr[0], sizeof(struct sockaddr));
        if (b == -1)
        {
            perror("bind failed");
            exit(0);
        }
        listen(ctrlSock0, 1);
        fcntl(sflowPipes[0][0], F_SETFL, fcntl(sflowPipes[0], F_GETFL, 0) | O_NONBLOCK);
        
        sflowfd[0] = accept(ctrlSock0, (struct sockaddr *)&client, &size);
        if ( sflowfd[0] == -1)
        {
            perror("\naccept failed\n");
            exit(0);
        }
        printf("control connection2 is set up sccessfully");
   }
  

   
   pipe(sflowPipes[1]);
   pid[1]= fork();
   if(pid[1]==0) {
        close(sflowPipes[1][1]);
        int ctrlSock1 = socket(AF_INET, SOCK_STREAM, 0);
        assign(&subflowaddr[1],63457);
        b = bind(ctrlSock1, (struct sockaddr *)&subflowaddr[1], sizeof(struct sockaddr));
        if (b == -1)
        {
            perror("bind failed");
            exit(0);
        }
        listen(ctrlSock1, 1);
        sflowfd[1] = accept(ctrlSock1, (struct sockaddr *)&client, &size);
        
        if ( sflowfd[1] == -1)
        {
            perror("\naccept failed\n");
            exit(0);
        }
        printf("control connection3 is set up sccessfully");
        fcntl(sflowPipes[1][0], F_SETFL, fcntl(sflowPipes[0], F_GETFL, 0) | O_NONBLOCK);
      
         
        child_write(sflowfd[1], sflowPipes[i]);
   }

    





    
    pipe(sflowPipes[2]);
    pid[2]= fork();
    if(pid[2]==0) {
        close(sflowPipes[2][1]);
        int ctrlSock2 = socket(AF_INET, SOCK_STREAM, 0);
        assign(&subflowaddr[2],63458);
        b = bind(ctrlSock2, (struct sockaddr *)&subflowaddr[2], sizeof(struct sockaddr));
        if (b == -1)
        {
            perror("bind failed");
            exit(0);
        }
        listen(ctrlSock2, 1);
        sflowfd[2]  = accept(ctrlSock2, (struct sockaddr *)&client, &size);
        if (ctrlfd == -1)
        {
            perror("\naccept failed\n");
            exit(0);
        }
        printf("control connection3is set up sccessfully");
        if(pipe(sflowPipes[2])< 0) {
            perror("error in pipe");
            exit(0);
        }
        fcntl(sflowPipes[2][0], F_SETFL, fcntl(sflowPipes[0], F_GETFL, 0) | O_NONBLOCK);
        
    
        sflowProcesses[2] = fork();
        if (sflowProcesses[2] == 0)
        {  
            child_write(sflowfd[2], sflowPipes[2]);
        }


    }



















 


    
    //accept all the remaining subflow connections
    // for(i=0; i<3; i++) {
    //     // assign(&subflowaddr[i],63456+i);
    //     int sock = socket(AF_INET, SOCK_STREAM, 0);
    //     int k = 63456+i;
    //     // subflowaddr[i]->sin_family = AF_INET;
    //     // subflowaddr[i]->sin_addr.s_addr = inet_addr("127.0.0.1");
    //     // subflowaddr[i]->sin_port = htons(k);
    //     subflowaddr.sin_family = AF_INET;
    //     subflowaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //     subflowaddr.sin_port = htons(k);
    //        b = bind(sock, (struct sockaddr *)&subflowaddr, sizeof(subflowaddr));
    //     if (b == -1)
    //     {
    //         perror("bind failed");
    //         exit(0);
    //     }
    //     listen(sock, 4);

        
    //     int c = accept(sock, (struct sockaddr *)&subflowaddr, sizeof(subflowaddr));
    //     if (c == -1)
    //     {
    //         printf("\connect failed for subflow %d\n", i);
    //         exit(0);
    //     }
    //     printf("connect to subflows");

    
    // }
        


    //continuously read the data from the controlflow signal
     struct control*  dss = (struct control*)malloc(sizeof(struct control));
    
    while(1) {
        // bzero(pack,sizeof(pack));
        // temp= recvfrom(ctrlfd,&dss, sizeof(dss),0 ,(struct sockaddr *)&client, size);
        // if(temp<0)
        printf("\nasd");
        if(read(ctrlfd, dss, sizeof(struct control)) < 0) {
            perror("error in control connection");
            break;
        }

        if(write(ctrlfd, "ack", 3 * sizeof(char)) < 0){
             perror("error in control connection");
            break;
        }


    if(pipe(sflowPipes[0])< 0) {
        perror("error in pipe");
        exit(0);
    }
   
    sflowProcesses[0] = fork();
    if (sflowProcesses[0] == 0)
    {  
        child_write(sflowfd[0], sflowPipes[i]);
    }
		{
			perror("ack send");
			break;
		}
        int portno = dss->port_no;

        for(i=0; i<3 ; i++) {
            if(subflowaddr[i]->sin_port == portno) {
                if(read(sflowPipes[i][0],pack , sizeof(pack)) <= 0)
                {
                    perror("Read Subflow");
                    break;
                }
              
                if(sizeof(pack)==dss->dataSize) {
                    for(i = 0; i < 4; i++){
                        allData[dss->dsn * 4 + i] = pack->data[i];
                    }
                }
                else {
                    printf("corrupted packet");                   
                }
            
            }
        }
		

		printf("%s recieved\n", pack->data);		
      
        
    }
    printf("Result String:\n%s\n", allData);
	clearSubflows(sflowProcesses);
    close(ctrlfd);
    exit(0);
}




   

void child_write(int sflowfd, int sflowPipe[2]) {
    struct packet* pack ;
    pack->data =(char*)malloc(4 * sizeof(char));
    int size = sizeof(pack);
	while(1) {
		
		if(read(sflowfd, pack, size) <0)
		{
			printf("%s",pack->data);
			perror("error in reading subflow");
			break;
		}
        close(sflowPipe[0]);
		if(write(sflowPipe[1], pack, size) < 0)
		{
			perror("error in sflowPipe");
			break;
		}
		
	}
 	close(sflowfd);
	exit(0);
}

void clearSubflows(int process[3]) {
	for(int i = 0; i < 3; i++)
	{
		kill(process[i], SIGKILL);
	}
}


void assign(struct sockaddr_in* s, int portno) {
    s->sin_family = AF_INET;
    s->sin_addr.s_addr = inet_addr("127.0.0.1");
    s->sin_port = htons(portno);

}