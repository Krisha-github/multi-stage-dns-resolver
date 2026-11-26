#include "socket_utils.h"
#include <cstring>

bool start_socket_library() {
#ifdef _WIN32
    WSADATA wsa{};
    return WSAStartup(MAKEWORD(2,2), &wsa) == 0;
#else
    return true;
#endif
}

void stop_socket_library() {
#ifdef _WIN32
    WSACleanup();
#endif
}

bool send_complete(socket_handle sock, const void* data, std::size_t bytes) {
    const char* p = static_cast<const char*>(data);
    while (bytes) {
#ifdef _WIN32
        int n = send(sock, p, static_cast<int>(bytes), 0);
#else
        ssize_t n = send(sock, p, bytes, 0);
#endif
        if (n <= 0) return false;
        p += n;
        bytes -= static_cast<std::size_t>(n);
    }
    return true;
}

bool receive_complete(socket_handle sock, void* data, std::size_t bytes) {
    char* p = static_cast<char*>(data);
    while (bytes) {
#ifdef _WIN32
        int n = recv(sock, p, static_cast<int>(bytes), 0);
#else
        ssize_t n = recv(sock, p, bytes, 0);
#endif
        if (n <= 0) return false;
        p += n;
        bytes -= static_cast<std::size_t>(n);
    }
    return true;
}

bool send_message(socket_handle sock, uint32_t type, const std::string& text) {
    if (text.size() >= DNS_TEXT_SIZE) return false;
    WireMessage msg{};
    msg.type = htonl(type);
    std::memcpy(msg.text, text.c_str(), text.size() + 1);
    return send_complete(sock, &msg, sizeof(msg));
}

bool receive_message(socket_handle sock, WireMessage& msg) {
    if (!receive_complete(sock, &msg, sizeof(msg))) return false;
    msg.type = ntohl(msg.type);
    msg.text[DNS_TEXT_SIZE - 1] = '\0';
    return true;
}

socket_handle make_tcp_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

bool prepare_listener(socket_handle sock, uint16_t port) {
    int reuse = 1;
#ifdef _WIN32
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse)) != 0) return false;
#else
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) return false;
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) return false;
    return listen(sock, 5) == 0;
}

socket_handle connect_to_host(const std::string& ip, uint16_t port) {
    socket_handle sock = make_tcp_socket();
    if (sock == INVALID_SOCKET_HANDLE) return INVALID_SOCKET_HANDLE;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1 ||
        connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        CLOSE_SOCKET(sock);
        return INVALID_SOCKET_HANDLE;
    }
    return sock;
}
