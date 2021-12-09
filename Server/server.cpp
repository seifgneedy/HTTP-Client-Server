#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include "client_handler.cpp"
using namespace std;
// maximum outstanding connection requests
static const int MAXPENDING = 10; 
// instance from client_handler used to handle client request
client_handler handler;
void dieWithSystemMessage(char* msg);
void thread_func(int clntSock);
int main(int argc,char* argv[]){
    // take arguments and don't forget -pthread
    /* 
    if(argc != 2){
        cout<<"wrong args, it should be \"./server port_number\""<<endl;
        exit(EXIT_FAILURE);
    }
    // get server IP 
    in_port_t servPort = atoi(argv[1]); */
    in_port_t servPort=8080;
    // create socket for incoming connections
    int servSock;
    if((servSock= socket(AF_INET,SOCK_STREAM,0))<0)
        dieWithSystemMessage((char *)"socket() failed");
    // construct local address structure 
    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=INADDR_ANY;
    servAddr.sin_port=htons(servPort);
    // bind servAddr to the socket
    if(bind(servSock,(struct sockaddr *)&servAddr,sizeof(servAddr))<0)
        dieWithSystemMessage((char *)"bind() failed");
    if(listen(servSock,MAXPENDING)<0)
        dieWithSystemMessage((char *)"listent() failed");
    cout<<"Server is listening on port "<<servPort<<endl;
    // construct client address and length
    struct sockaddr_in clntAddr;
    socklen_t clntAddrlen=sizeof(clntAddr);

    while(1){
        if(handler.clients_count < handler.MAX_THREADS){
            // waiting for a client to connect
            // accept() (creates a new file descriptor for an incoming connection)
            int clntSock=accept(servSock,(struct sockaddr *) &clntAddr,&clntAddrlen);
            if(clntSock<0)
                dieWithSystemMessage((char *)"accept() failed");
            char clntName[INET_ADDRSTRLEN]; // String to contain client address
            if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,sizeof(clntName)) != NULL)
                printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
            else
                puts("Unable to get client address");
            // here we should attach thread to work with incoming client request 
            // used <thread> in c++11 
            handler.clients_count++;
            thread cl_thread(thread_func,clntSock);
            cl_thread.detach();            
        }
        else{
            cout<<"reached maximum number of clients"<<endl;
             // wait for 50ms as client will be disconnected due to timeout
            timeval tv;
            tv.tv_usec=50000;   tv.tv_sec=0;
            select(FD_SETSIZE,NULL,NULL,NULL,&tv);
        }
    }
    return 0;
}
void thread_func(int clntSock){
    // handle client request
    cout<<handler.clients_count<<endl;
    handler.handle(clntSock);
    // decrease number of clients as this client finished
    handler.clients_count--;
    cout<<handler.clients_count<<endl;
}
void dieWithSystemMessage(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}