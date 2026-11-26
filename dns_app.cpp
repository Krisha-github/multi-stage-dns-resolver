// Group 41 - Application #4: DNS Server
// Usage: ./dns_app <DNS Server Port>
#include "socket_utils.h"
#include "input_utils.h"
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace {
socket_handle listener=INVALID_SOCKET_HANDLE;
const char* DATABASE="dns_records.txt";
void stop_dns(int){ if(listener!=INVALID_SOCKET_HANDLE) CLOSE_SOCKET(listener); std::_Exit(0); }
bool resolve(uint32_t type,const std::string& key,std::string& answer){
    std::ifstream file(DATABASE);
    if(!file) return false;
    std::string domain,ip;
    while(file>>domain>>ip){
        if(type==REQ_DOMAIN_IP && domain==key){answer=ip;return true;}
        if(type==REQ_IP_DOMAIN && ip==key){answer=domain;return true;}
    }
    return false;
}
}
int main(int argc,char* argv[]){
    if(!start_socket_library()) return 1;
    if(argc!=2){std::cerr<<"Usage: "<<argv[0]<<" <DNS Server Port>\n";stop_socket_library();return 1;}
    int port=std::atoi(argv[1]);
    if(!port_ok(port)){std::cerr<<"Invalid DNS Server port.\n";stop_socket_library();return 1;}
#ifndef _WIN32
    std::signal(SIGPIPE,SIG_IGN);
#endif
    std::signal(SIGINT,stop_dns);
    listener=make_tcp_socket();
    if(listener==INVALID_SOCKET_HANDLE || !prepare_listener(listener,static_cast<uint16_t>(port))){
        std::cerr<<"Could not start DNS Server.\n"; if(listener!=INVALID_SOCKET_HANDLE) CLOSE_SOCKET(listener);stop_socket_library();return 1;
    }
    std::cout<<"DNS Server listening on "<<port<<"; database="<<DATABASE<<std::endl;
    while(true){
        sockaddr_in peer{};
#ifdef _WIN32
        int len=sizeof(peer);
#else
        socklen_t len=sizeof(peer);
#endif
        socket_handle client=accept(listener,reinterpret_cast<sockaddr*>(&peer),&len);
        if(client==INVALID_SOCKET_HANDLE) continue;
        WireMessage req{};
        if(receive_message(client,req)){
            std::string key=req.text,answer;
            if(lookup_key_ok(req.type,key) && resolve(req.type,key,answer)) send_message(client,RESP_OK,answer);
            else send_message(client,RESP_ERROR,"entry not found in the database");
        }
        shutdown(client,SHUTDOWN_BOTH); CLOSE_SOCKET(client);
    }
}
