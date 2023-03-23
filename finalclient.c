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


struct control {
   int dsn; //Data sequence number
   int ssn; //Subflow Sequence Number
   int dataSize;
   int port_no
};//32 bits length

struct packet
{
    char* data;
};



int main(int argc, char *argv[]) {
    
    //file descriptors
	int sockfd, ctrlSock , sflowsockets[3];

     

    //IP address and port details are stored here
    struct sockaddr_in ctrlAddr, client, clientaddr;
    struct sockaddr_in subflowaddr0, subflowaddr1,subflowaddr2;
    int size = sizeof(struct sockaddr);

    //subflow pipes and processes
    int sflowPipes[3][2];// stores the pipe
    int sflowProcesses[3]; // stores the processes of 3 sub flows
    int sflowfd[3];// stores the file descriptors of subflows
    int subflowsSSN[3]= {0,0,0};




    //buffer for storing data
    char* allData = (char*)malloc(993 * sizeof(char));
    char* buffer = "";
    char*  packbuffer[248][2];
    int i,b,pos = 0;

    //ctrl dss message 
    struct control*  dss = (struct control*)malloc(sizeof(struct control));

    //temporary fd
    int tempfd, temp;
      
      //data for pid
    int pid[3];

    FILE* log = fopen("log.txt", "w");

    //Create control socket and other subflow sockets
    ctrlSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ctrlSock == -1)
    {
        perror("control socket creation failed");
        exit(0);
    }
    assign(&ctrlAddr,6377);
    int c = connect(ctrlSock, (struct sockaddr *)&ctrlAddr, sizeof(struct sockaddr_in));
    if (c == -1)
    {
        perror("\nconnect failed in control\n");
        exit(0);
    }
    printf("connect successful");






    pipe(sflowPipes[0]);
    pid[0]= fork();
    if(pid[0]==0) {
 
        sflowfd[0] = socket(AF_INET, SOCK_STREAM, 0);
        assign(&subflowaddr0,63456);
        c = connect(sflowfd[0], (struct sockaddr *)&subflowaddr0, sizeof(struct sockaddr_in));
        if (c == -1)
        {
            perror("\nconnect failed\n");
            exit(0);
        }
        sflowProcesses[0] = fork();
        printf("connect successful");
        fcntl(sflowPipes[0], F_SETFL, fcntl(sflowPipes[0], F_GETFL, 0) | O_NONBLOCK);
        if (sflowProcesses[0] == 0)
        {  
            child_write(sflowfd[0], sflowPipes[0]);
        }
    }

    
    pipe(sflowPipes[1]);
    pid[1]= fork();
    if(pid[1]==0) {
        sflowfd[1] = socket(AF_INET, SOCK_STREAM, 0);    
        assign(&subflowaddr1,63457);
        c = connect(sflowfd[1], (struct sockaddr *)&subflowaddr1, sizeof(struct sockaddr_in));
        if (c == -1)
        {
            perror("\nconnect failed\n");
            exit(0);
        }
        printf("connect successful");
        child_write(sflowfd[1],sflowPipes[1] );
    }

    pipe(sflowPipes[2]);
    pid[2]= fork();
    if(pid[2]==0) { 

        sflowfd[2] = socket(AF_INET, SOCK_STREAM, 0);    
        assign(&subflowaddr2,63458);
        c = connect(sflowfd[2], (struct sockaddr *)&subflowaddr2, sizeof(struct sockaddr_in));
        if (c == -1)
        {
            perror("\nconnect failed\n");
            exit(0);
        }
        child_write(sflowfd[2],sflowPipes[2] );
        printf("connect successful");
    }

   

   
    // assigns the ipaddress and port numbers  and initialises the socket 
    
    // for(i =0; i<3; i++) {
    //     // assign(&subflowaddr[i],63456+i);
    //     int k= 63456+i;
    //     subflowaddr.sin_family = AF_INET;
    //     subflowaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //     subflowaddr.sin_port = htons(k);
    //     sflowsockets[i] =  socket(AF_INET, SOCK_STREAM, 0);
    //     // b = bind(sflowsockets[i], (struct sockaddr *)&subflowaddr, sizeof(struct sockaddr_in));
    //     // if (b == -1)
    //     // {
    //     //     perror("bind failed");
    //     //     exit(0);
    //     // }
    //     // listen(sflowsockets[i], 4);
   
    //     printf("successfully connected to socket %d", i);
    //     sflowfd[i] = connect(sflowsockets[i], (struct sockaddr *)&subflowaddr, sizeof(struct sockaddr_in));
    //     if (sflowfd[i]== -1)
    //     {
    //         perror("\nconnect failed\n");
    //         exit(0);
    //     }
       
        
    // }

   
    char* sendData = (char*)malloc(5 * sizeof(char));
	sendData[4] = '\0';


    // // generate message format from these alphabets
    if(pid[0]!=0||pid[1]!=0||pid[2]!=0){

        char dataset[63]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

        int j=0;
        int sflowIndex;
        int dsn = 0;
        char *ack;
        char *output[1024];
        char* s = (char*)malloc(50 * sizeof(char));
        for(int i= 1 ; i<=992; i++){
            if(j%4 == 0) {
                sflowIndex = ((i-1)/4)%3;
                sprintf(s,"\n%d %d  %d \n", sflowIndex, subflowsSSN[sflowIndex],dsn);
               	fputs(s, log);
               
                dss->dsn = dsn;
                dss->ssn = subflowsSSN[sflowIndex];
                if(sflowIndex == 0) {
                    dss->port_no = 63456;
                }
                else if (sflowIndex == 1 )
                {
                      dss->port_no = 63457;
                }
                else if(sflowIndex == 2) {

                     dss->port_no = 63458;
                }
            
                
                dss->dataSize = sizeof(sendData); 
                if(write(ctrlSock, dss, sizeof(dss)) < 0)
                {
                    perror("error in control write ");
                    break;
                }

                if(write(sflowPipes[sflowIndex][1], sendData, 4 * sizeof(char))<0) {
                    
                    perror("error in control write ");
                    break;
                }
                if(read(ctrlSock, ack, 3* sizeof(char))<0){

                    perror("error in control socket  ack");
                    break;
                }
                if(ack=="ack") {
                    printf("ack recieved");
                }

                subflowsSSN[sflowIndex]++;
                dsn++;
                j=0;
            }   
            j=j+1;
            sendData[j] = dataset[i%62];
            printf("%c",sendData[j] );
            
        }

       
        // strcat(&all,"\0");
        // fwrite(all, 1, strlen(all), log);
        fclose(log);
        clearSubflows(pid);
        exit(0);
    
    }

}

   

void assign(struct sockaddr_in* s, int portno) {
    s->sin_family = AF_INET;
    s->sin_addr.s_addr = inet_addr("127.0.0.1");
    s->sin_port = htons(portno);

}

acceptConnection(int sockfd, struct sockaddr_in* subflow, int size, int i ) {
    int ctrlfd = accept(sockfd, (struct sockaddr *)subflow, &size); 
    if (ctrlfd == -1)
    {
        printf("\naccept failed for subflow %d\n", i);
        exit(0);
    }
    return ctrlfd;
        
}
   

void child_write(int sflowfd, int sflowPipe[2]) {
    struct packet* pack ;
    pack->data =(char*)malloc(4 * sizeof(char));
    int size = sizeof(pack);
	while(1) {
        read(sflowPipe[1], pack, size);
		
		// if( <= 0)
		// {
		// 	printf("%s",pack->data);
		// 	perror("error in reading subflow");
		// 	break;
		// }
        close(sflowPipe[0]);
		if(write(sflowfd, pack, size) < 0)
		{
			perror("error in writing into he socket");
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


