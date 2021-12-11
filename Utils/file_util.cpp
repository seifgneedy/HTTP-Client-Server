#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <filesystem>
#include "parser.cpp"
using namespace std;
class file_util{
private:
static const int BUFFER_SIZE = 4096;  //4kb

public:
    parser ps;
    /* 
    open file 
    read it line by line with every line : get the request in 2 parts: method file_path 
    */
    vector<vector<string>> get_requests(string path);
    /*
        create http response in format that client can understand
    */
    string create_http_response(string method_line,string body);
    /*
        create http request in the format as the server can understand it
    */
    string create_http_request(string method,string file_path);
    /*
        open the file specified in POST and returns its content
    */
    pair<char*,int> get_file_content(string file_path);
    /*
        create directories (if exists) for the specified file_path
    */ 
    void create_dirs(string file_path);
    /*
        save the content in the file path
    */
    void save_file(string path,string content);
    /*
        send message to the specified socket
    */
    void send_message(string& msg,int sock_id);
    /*
        receive message from the server to the sent requests
    */
    pair<string,string> receive_message(int sock_id);
    /*
        get the requests from the file,determine the type of every request
        ,handle the required request, send the request receive resoponse and handle it

    */
    void handle_requests(string file_path, int clntSock);
    file_util(/* args */){

    }
    ~file_util(){

    }
};
// works
vector<vector<string>> file_util::get_requests(string path){
    vector<vector<string>> requests;
    fstream req_file;
    req_file.open(path);
    string lne;
    for(int i=0;getline(req_file,lne);i++){
        vector<string> evs;
        requests.push_back(evs);
        stringstream ss(lne);
        for(int j=0;j<2;j++){
            string tmp; ss>>tmp;
            requests[i].push_back(tmp);
        }
    }
    return requests;
}
string file_util::create_http_response(string method_line,string body){
    stringstream response;
    response<<"HTTP/1.1 ";
    stringstream mth_stream(method_line);
    string method,file_path;
    mth_stream>>method>>file_path;
    if(method=="GET"){  // no body here
        // get the file if exists => respond wiht HTTP/1.1 200 OK\r\nContent-Length :%d\r\n\r\n{body ..}
        // not exist => respond wiht HTTP/1.1 404 Not Found\r\n\r\n
        ifstream ifile;
        ifile.open(file_path.substr(1));
        if(ifile){  // file exists
            response<<"200 OK\r\n";
            pair<char*,int> content=get_file_content(file_path.substr(1));
            response<<"Content-Length :"<<content.second<<"\r\n";
            response<<"\r\n";
            response.write(content.first,content.second);
            response<<"\r\n";
            delete content.first;
        }else{  // file not exists => bad request
            response<<"404 Not Found\r\n\r\n";
        }
    }else if(method=="POST"){
        // respond with HTTP/1.1 200 OK\r\n\r\n
        // and save the file in the specified place => need to get the place from method_line
        // what if the path consists of directories then the file ?
        response<<"200 OK\r\n\r\n";
        int last_dir=file_path.find_last_of('/');
        if(last_dir!=0){
            string file_dir=file_path.substr(1,last_dir-1);
            filesystem::create_directories(file_dir);
        }
        // remove the last \r\n in the body
        body.pop_back(); body.pop_back();
        save_file(file_path.substr(1),body);
    }
    return response.str();
}
void file_util::create_dirs(string file_path){
    int last_dir=file_path.find_last_of('/');
    if(last_dir!=0){
        string file_dir=file_path.substr(1,last_dir-1);
        filesystem::create_directories(file_dir);
    }
}
void file_util::save_file(string path,string content){
    ofstream ff(path);
    ff.write(content.c_str(),content.size());
    ff.close();
}
//works
string file_util::create_http_request(string method,string file_path){
    stringstream request;
    if(method=="client_get"){
        request<<"GET "<<file_path<<" HTTP/1.1\r\n"<<"\r\n";
    }else if(method == "client_post"){
        // open the file and put its content in the body of the request
        request<<"POST "<<file_path<<" HTTP/1.1\r\n";
        pair<char*,int> content = get_file_content(file_path.substr(1));
        request<<"Content-Length :"<<content.second<<"\r\n";
        request<<"\r\n";
        request.write(content.first,content.second);
        request<<"\r\n";
        // deleting file from memory
        delete content.first;
    }
    return request.str();
}
// works 
pair<char*,int> file_util::get_file_content(string file_path){
    char* input_bytes;
    ifstream infile;
    infile.open(file_path,ios::binary);
    infile.seekg(0,ios::end);
    size_t size_in_bytes=infile.tellg();
    // take care of the file is in the heap, you need to delete it
    input_bytes=new char[size_in_bytes];
    infile.seekg(0,ios::beg);
    infile.read(input_bytes,size_in_bytes);
    return {input_bytes,size_in_bytes};
}
// works
void file_util::send_message(string& msg,int sock_id){
    int len=msg.size();
    int bytes_left=msg.size();
    int total_sent=0;
    while(total_sent<len){
        ssize_t sent=send(sock_id,msg.c_str()+total_sent,bytes_left,0);
        total_sent+=sent;
        bytes_left-=sent;
    }
}

pair<string,string> file_util::receive_message(int sock_id){
    int body_received=0,con_len=0,received_bytes=0;
    char recv_buffer[BUFFER_SIZE];
    string method,body;
    received_bytes=recv(sock_id,recv_buffer,BUFFER_SIZE,0);
    if(errno==EAGAIN||errno==EWOULDBLOCK||received_bytes==0)
        return {"",""};
    pair<string,pair<string,int>> head = ps.parse_header(recv_buffer,received_bytes);
    method=head.first;
    body=head.second.first;
    con_len=head.second.second;
    body_received=body.size();
    while(body_received<con_len){
        received_bytes=recv(sock_id,recv_buffer,BUFFER_SIZE,0);
        body.insert(body.end(),recv_buffer,recv_buffer+received_bytes);
        body_received+=received_bytes;
    }
    return {method,body};
}