#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace std;
class parser{
private:
    /* data */
public:
    // parse header and return <request_line,<rest of body,content-length>>
    pair<string,pair<string,int>> parse_header(char* received,int rec_sz);
    parser(/* args */);
    ~parser();
};


pair<string,pair<string,int>> parser::parse_header(char* received,int rec_sz){
    string recvd="";
    recvd.insert(recvd.end(),received,received+rec_sz);
    int end_header=recvd.find("\r\n\r\n");
    string method=recvd.substr(0,recvd.find("\r\n"));
    string rest_body=recvd.substr(end_header+4);
    int con_len=0;
    string line;
    istringstream stream(recvd);
    while(getline(stream,line,'\r')){
        fstream lne_stream(line);
        if(line.find("Content-Length :")!=line.npos){
            sscanf(line.c_str(),"\nContent-Length :%d",&con_len);
            break;
        }
    }
    return {method,{rest_body,con_len}};
}


parser::parser(/* args */){
}

parser::~parser(){
}
