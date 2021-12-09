#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <filesystem>
static const int MAX_TIMEOUT = 15;
static const int MAX_THREADS = 20;
using namespace std;
void editAddr(struct sockaddr_in * servAddr);
pair<char*,int> get_file_content(string file_path);
string create_http_request(string method,string file_path);
pair<string,pair<string,int>> test_parsing(string received);
void save_file(string path,string content);
string create_http_response(string method_line,string body);
int main(){
    for(int i=0;i<=MAX_THREADS;i++){
        timeval tv;
        tv.tv_usec=0;
        //TODO: heuristic tv.tv_sec=?
        tv.tv_sec=  MAX_TIMEOUT -  (MAX_TIMEOUT-1) * ( (double)(i-1)/MAX_THREADS);
        cout<<tv.tv_sec<<endl;
    }
    int j=3;
    do{
        cout<<j<<endl;
        if(j==2)
            break;
        j--;
    }while(j>0);

/*    vector<vector<string>> requests;
    fstream req_file;
    req_file.open("/run/media/seif/CSED/CSED/CSED 2023/3rd Year/1st Semester/Network/Assignments/HTTP-Client-Server/requests.txt");
    string lne;
    for(int i=0;getline(req_file,lne);i++){
        vector<string> evs;
        requests.push_back(evs);
        stringstream ss(lne);
        for(int j=0;j<2;j++){
            string tmp; ss>>tmp;
            requests[i].push_back(tmp);
            cout<<requests[i][j]<<endl;
        }
    } */
    // try * and & 
/*  struct sockaddr_in servAddr;
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    servAddr.sin_port=htons(8080);
    editAddr(&servAddr);
    cout<<servAddr.sin_port<<endl;*/
    // check transfer file 
/*pair<char*,int> content=get_file_content("Client/Bridge.png");
    ofstream ff("Bridge.png");
    ff.write(content.first,content.second);
    ff.close(); */
    string received =create_http_request("client_get","/Client/play.html");
    // cout<<received.substr(received.find("\r\n\r\n")+4);
    pair<string,pair<string,int>> res=test_parsing(received);
    cout<<res.first<<endl;
    cout<<res.second.first<<endl;
    cout<<res.second.second<<endl;
    create_http_response(res.first,res.second.first);
    cout<<endl;
    string fl="/test_dir/2/3/4/f.txt";
    string rl=fl;
    int last_dir=fl.find_last_of('/');
    fl=fl.substr(1,last_dir-1);
    cout<<fl<<endl;
    filesystem::create_directories(fl);
    save_file(rl.substr(1),"I want to play football");
    return 0;
}
void editAddr(struct sockaddr_in * servAddr){
    servAddr->sin_port=1232;
}

pair<char*,int> get_file_content(string file_path){
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

string create_http_request(string method,string file_path){
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

        delete content.first;
    }
    return request.str();
}

pair<string,pair<string,int>> test_parsing(string received){
    string recvd(received);
    int end_header=recvd.find("\r\n\r\n");
    string method=recvd.substr(0,recvd.find("\r\n"));
    string rest_body=recvd.substr(end_header+4);
    int con_len=0;
    string line;
    istringstream stream(recvd);
    while(getline(stream,line,'\r')){
        fstream lne_stream(line);
        cout<<line;
        if(line.find("Content-Length :")!=line.npos){
            sscanf(line.c_str(),"\nContent-Length :%d",&con_len);
            break;
        }
    }
    return {method,{rest_body,con_len}};
}

void save_file(string path,string content){
    ofstream ff(path);
    ff.write(content.c_str(),content.size());
    ff.close();
}

string create_http_response(string method_line,string body){
    stringstream response;
    stringstream mth_stream(method_line);
    string method,file_path;
    mth_stream>>method>>file_path;
    cout<<method<<endl<<file_path;
    if(method_line.find("GET")!=method_line.npos){
        // get the file if exists => respond wiht HTTP/1.1 200 OK\r\nContent-Length :%d\r\n\r\n{body ..}
        // not exist => respond wiht HTTP/1.1 404 Not Found\r\n
    }else if(method_line.find("POST")!=method_line.npos){
        // respond with HTTP/1.1 200 OK\r\n\r\n
        // and save the file in the specified place => need to get the place from method_line

    }
    return response.str();
}