#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <cstddef>
#include <cstdint>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_handle = SOCKET;
const socket_handle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#define CLOSE_SOCKET closesocket
#define SHUTDOWN_BOTH SD_BOTH
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using socket_handle = int;
const socket_handle INVALID_SOCKET_HANDLE = -1;
#define CLOSE_SOCKET close
#define SHUTDOWN_BOTH SHUT_RDWR
#endif

constexpr std::size_t DNS_TEXT_SIZE = 256;
constexpr uint32_t REQ_DOMAIN_IP = 1;
constexpr uint32_t REQ_IP_DOMAIN = 2;
constexpr uint32_t RESP_OK = 3;
constexpr uint32_t RESP_ERROR = 4;

struct WireMessage {
    uint32_t type;
    char text[DNS_TEXT_SIZE];
};

bool start_socket_library();
void stop_socket_library();
bool send_complete(socket_handle sock, const void* data, std::size_t bytes);
bool receive_complete(socket_handle sock, void* data, std::size_t bytes);
bool send_message(socket_handle sock, uint32_t type, const std::string& text);
bool receive_message(socket_handle sock, WireMessage& msg);
socket_handle make_tcp_socket();
bool prepare_listener(socket_handle sock, uint16_t port);
socket_handle connect_to_host(const std::string& ip, uint16_t port);

#endif
