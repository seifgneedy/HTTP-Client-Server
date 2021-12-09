#include <atomic>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "../Util/file_util.cpp"
using namespace std;
class client_handler{
private:
    static const int BUFFER_SIZE = 4096;  //4kb
    static const int MAX_TIMEOUT = 15;
    file_util fu;
    parser ps;
public:
    // maximum number of threads/clients
    static const int MAX_THREADS = 20;
    // atomic value to prevent race condition
    atomic<int> clients_count;
    void handle(int clntSock);
    // set timeout for the new client to make connection persistent but in limited time
    // according to the number of clients already in the server now.
    void set_timeout(int clntSock);
    // the estimated timeout based on the heuristic min => 1sec , max = 15 sec
    int estimated_timeout(){
        return  MAX_TIMEOUT -   (MAX_TIMEOUT-1) * ( (double)(clients_count-1)/MAX_THREADS);
    }
    client_handler(/* args */);
    ~client_handler();
};
void client_handler::handle(int clntSock){
    // try a new idea
    string body="";
    while(true){
        set_timeout(clntSock);
        pair<string,string> request=fu.receive_message(clntSock);
        string method=request.first;
        body=request.second;
        if(method==""&&body=="")
            break;
        // make response according to the method
        string response=fu.create_http_response(method,body);
        fu.send_message(response,clntSock);
    }
    close(clntSock);
}

void client_handler::set_timeout(int clntSock){
        struct timeval tv;
        tv.tv_sec = estimated_timeout();
        tv.tv_usec = 0;
        setsockopt(clntSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }
client_handler::client_handler(){
    clients_count=0;
}

client_handler::~client_handler(){
}
