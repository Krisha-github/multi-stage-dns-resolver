#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H
#include <cstdint>
#include <string>
bool port_ok(int port);
bool ipv4_ok(const std::string& ip);
bool domain_ok(const std::string& domain);
bool lookup_key_ok(uint32_t request_type, const std::string& key);
#endif
