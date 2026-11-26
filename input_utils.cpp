#include "input_utils.h"
#include "socket_utils.h"
#include <cctype>

bool port_ok(int port) { return port >= 1 && port <= 65535; }
//Because TCP/UDP ports are represented using 16 bits

bool ipv4_ok(const std::string& ip) {
    in_addr addr{};
    return inet_pton(AF_INET, ip.c_str(), &addr) == 1;
}

bool domain_ok(const std::string& domain) {
    if (domain.empty() || domain.size() >= DNS_TEXT_SIZE) return false;
    if (domain.front() == '.' || domain.back() == '.') return false;
    std::size_t start = 0;
    while (start < domain.size()) {
        std::size_t dot = domain.find('.', start);
        std::size_t end = (dot == std::string::npos) ? domain.size() : dot;
        if (end == start || end - start > 63) return false;
        if (domain[start] == '-' || domain[end - 1] == '-') return false;
        for (std::size_t i = start; i < end; ++i) {
            unsigned char c = static_cast<unsigned char>(domain[i]);
            if (!(std::isalnum(c) || c == '-')) return false;
        }
        if (dot == std::string::npos) break;
        start = dot + 1;
    }
    return true;
}

bool lookup_key_ok(uint32_t request_type, const std::string& key) {
    if (request_type == REQ_DOMAIN_IP) return domain_ok(key);
    if (request_type == REQ_IP_DOMAIN) return ipv4_ok(key);
    return false;
}
