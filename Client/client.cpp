#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include "../Util/file_util.cpp"
using namespace std;
parser ps;
file_util fu;
void dieWithSystemMessage(char * msg);
void handle_requests(string file_path, int clntSock,struct sockaddr_in * servAddr );
void handle_response(string res_head,string& body,string method,string file_path);
int main(int argc,char* argv[]){

/*  if(argc!=3){
        cout<<"wrong args, it should be \"./client server_ip port_number\""<<endl;
        exit(EXIT_FAILURE);
    }
    // getting the server IP and port
    char* servIP=argv[1];
    in_port_t servPort = atoi(argv[2]); */
    char* servIP=(char *)"127.0.0.1";
    in_port_t servPort =8080;
    // create client socket
    int clntSock;
    if((clntSock= socket(AF_INET,SOCK_STREAM,0))<0)
        dieWithSystemMessage((char *)"socket() failed");
    
    struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=inet_addr(servIP);
    servAddr.sin_port=htons(servPort);
    string file_path;
    file_path="requests.txt";
    if(connect(clntSock,(struct sockaddr *)&servAddr,sizeof(servAddr))<0)
        dieWithSystemMessage((char *)"connect() failed");
    
    handle_requests(file_path,clntSock,&servAddr);
    close(clntSock);
    return 0;
}
void handle_requests(string file_path, int clntSock,struct sockaddr_in * servAddr ){
    
    vector<vector<string>> requests=fu.get_requests(file_path);
    for(vector<string> request : requests){
        string formatted_request=fu.create_http_request(request[0],request[1]);
        fu.send_message(formatted_request,clntSock);
        // receive the response from server and check if GET(OK(save file) or 404(print in console)) or POST(no error in post)
        pair<string,string> response=fu.receive_message(clntSock);
        string res_head=response.first,body=response.second;
        handle_response(res_head,body,request[0],request[1]);
    }
} 
void handle_response(string res_head,string& body,string method,string file_path){
    // I can't think anymore continue in the morning
    cout<<res_head<<endl;
    if(method=="client_get"){
        int status;
        sscanf(res_head.c_str(),"HTTP/1.1 %d OK",&status);
        if(status==200){
            int last_dir=file_path.find_last_of('/');
            if(last_dir!=0){
                string file_dir=file_path.substr(1,last_dir-1);
                filesystem::create_directories(file_dir);
            }
            cout<<file_path.substr(last_dir+1);
            fu.save_file(file_path.substr(1),body);
        }
    }
}

void dieWithSystemMessage(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}