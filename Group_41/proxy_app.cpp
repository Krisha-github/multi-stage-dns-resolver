// Group 41 - Application #4: Proxy Server
// Usage: ./proxy_app <Proxy Port>
// The DNS Server address is read from dns_config.txt ("<DNS_IP> <DNS_Port>")
#include "socket_utils.h"
#include "input_utils.h"
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace {
constexpr int CACHE_LIMIT = 3;
struct Mapping { bool used=false; std::string domain; std::string ip; };
Mapping cache[CACHE_LIMIT];
int replace_slot = 0;
socket_handle listener = INVALID_SOCKET_HANDLE;

void stop_listener(int) { if (listener != INVALID_SOCKET_HANDLE) CLOSE_SOCKET(listener); std::_Exit(0); }
int find_mapping(uint32_t type, const std::string& key) {
    for (int i=0;i<CACHE_LIMIT;++i) if (cache[i].used &&
        ((type==REQ_DOMAIN_IP && cache[i].domain==key) || (type==REQ_IP_DOMAIN && cache[i].ip==key))) return i;
    return -1;
}
void put_mapping(const std::string& domain, const std::string& ip) {
    for (int i=0;i<CACHE_LIMIT;++i) if (cache[i].used && cache[i].domain==domain && cache[i].ip==ip) return;
    cache[replace_slot].used = true;
    cache[replace_slot].domain = domain;
    cache[replace_slot].ip = ip;
    replace_slot = (replace_slot + 1) % CACHE_LIMIT;
}
bool ask_dns(const std::string& ip, uint16_t port, const WireMessage& request, WireMessage& response) {
    socket_handle sock = connect_to_host(ip, port);
    if (sock == INVALID_SOCKET_HANDLE) return false;
    bool ok = send_message(sock, request.type, request.text) && receive_message(sock, response);
    shutdown(sock, SHUTDOWN_BOTH); CLOSE_SOCKET(sock); return ok;
}
bool load_dns_config(std::string& ip, int& port) {
    std::ifstream file("dns_config.txt");
    return static_cast<bool>(file >> ip >> port);
}
}

int main(int argc, char* argv[]) {
    if (!start_socket_library()) return 1;
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <Proxy Port>\n";
        stop_socket_library(); return 1;
    }
    int proxy_port=std::atoi(argv[1]), dns_port=0;
    std::string dns_ip;
    if (!load_dns_config(dns_ip, dns_port)) {
        std::cerr << "Cannot read dns_config.txt (expected: <DNS_IP> <DNS_Port>).\n";
        stop_socket_library(); return 1;
    }
    if (!port_ok(proxy_port) || !port_ok(dns_port) || !ipv4_ok(dns_ip)) {
        std::cerr << "Invalid Proxy port or DNS configuration.\n"; stop_socket_library(); return 1;
    }
#ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
#endif
    std::signal(SIGINT, stop_listener);
    listener=make_tcp_socket();
    if (listener==INVALID_SOCKET_HANDLE || !prepare_listener(listener, static_cast<uint16_t>(proxy_port))) {
        std::cerr << "Could not start Proxy Server.\n"; if(listener!=INVALID_SOCKET_HANDLE) CLOSE_SOCKET(listener); stop_socket_library(); return 1;
    }
    std::cout << "Proxy Server listening on " << proxy_port << "; DNS=" << dns_ip << ":" << dns_port << std::endl;

    while(true) {
        sockaddr_in peer{};
#ifdef _WIN32
        int len=sizeof(peer);
#else
        socklen_t len=sizeof(peer);
#endif
        socket_handle client=accept(listener,reinterpret_cast<sockaddr*>(&peer),&len);
        if(client==INVALID_SOCKET_HANDLE) continue;
        WireMessage req{};
        if(!receive_message(client,req)) { CLOSE_SOCKET(client); continue; }
        std::string key=req.text;
        if(!lookup_key_ok(req.type,key)) {
            send_message(client,RESP_ERROR,"entry not found in the database");
        } else {
            int hit=find_mapping(req.type,key);
            if(hit>=0) {
                std::string answer=(req.type==REQ_DOMAIN_IP)?cache[hit].ip:cache[hit].domain;
                std::cout << "CACHE HIT: " << key << " -> " << answer << std::endl;
                send_message(client,RESP_OK,answer);
            } else {
                std::cout << "CACHE MISS: " << key << std::endl;
                WireMessage dns_response{};
                if(!ask_dns(dns_ip,static_cast<uint16_t>(dns_port),req,dns_response)) {
                    // A transport failure is not the same as "entry not found".
                    // Type 4 is reserved for a successful DNS lookup with no match.
                    // Close this negotiation without fabricating a Type 4 result.
                    std::cerr << "DNS server unreachable; closing this negotiation.\n";
                } else {
                    if(dns_response.type==RESP_OK) {
                        if(req.type==REQ_DOMAIN_IP) put_mapping(key,dns_response.text);
                        else put_mapping(dns_response.text,key);
                    }
                    send_message(client,dns_response.type,dns_response.text);
                }
            }
        }
        shutdown(client,SHUTDOWN_BOTH); CLOSE_SOCKET(client);
    }
}
