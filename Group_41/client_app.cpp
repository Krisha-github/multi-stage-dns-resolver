// Group 41 - Application #4: Client
// Usage: ./client_app <Proxy IP Address> <Proxy Port>
#include "socket_utils.h"
#include "input_utils.h"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (!start_socket_library()) return 1;
    //On Windows, Winsock must be initialized before using socket
    // APIs. On Linux it isn't necessary, so the function simply returns true.
    
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Proxy IP Address> <Proxy Port>\n";
        stop_socket_library(); return 1;
    }

    int proxy_port = std::atoi(argv[2]);
    std::string proxy_ip = argv[1];
    
    //Validate Proxy IP and port
    if (!ipv4_ok(proxy_ip) || !port_ok(proxy_port)) {
        std::cerr << "Invalid Proxy IP address or port.\n";
        stop_socket_library(); return 1;
    }

    while (true) {  //Keep allowing the user to perform DNS lookups until they choose Exit.
        std::cout << "\n1. Domain -> IP\n2. IP -> Domain\n3. Exit\nChoice: ";
        int choice;
        if (!(std::cin >> choice)) break;
//If input itself fails e.g. stream enters a failed state,the loop breaks.

        if (choice == 3) break;
        if (choice != 1 && choice != 2) { std::cout << "Invalid choice.\n"; continue; }
        std::string key;
        std::cout << (choice == 1 ? "Enter domain: " : "Enter IP: ");
        
        std::cin >> key;//stores what the user wants to search

        uint32_t type = static_cast<uint32_t>(choice);
        //Convert choice into request type: int → uint32_t

        //Validate the lookup itself
        if (!lookup_key_ok(type, key)) { std::cout << "Invalid lookup input.\n"; continue; }

        socket_handle sock = connect_to_host(proxy_ip, static_cast<uint16_t>(proxy_port));
        //client creates a TCP connection proxy server, each lookup gets a new TCP connection

        if (sock == INVALID_SOCKET_HANDLE) { std::cerr << "Cannot connect to Proxy Server.\n"; continue; }
        //Check whether connection succeeded

        //Send the request
        bool ok = send_message(sock, type, key);
        //send_message() creates:
        //WireMessage msg{};
        //Then:msg.type = htonl(type) converts the integer 
        //into network byte order.memcpy(msg.text, ...)copies google.com into the message.
        // Then:send_complete(sock, &msg, sizeof(msg));
        //sends the entire WireMessage over TCP.

        WireMessage response{};//creates an empty structure where the server's response will be stored
        
        //Because if sending failed:ok = false there's no point trying to receive.
        if (ok) ok = receive_message(sock, response);
        if (!ok) std::cerr << "Communication failure.\n";

        else if (response.type == RESP_OK) std::cout << "Response Type 3: " << response.text << "\n";
        else if (response.type == RESP_ERROR) std::cout << "Response Type 4: " << response.text << "\n";
        else std::cout << "Unexpected response type: " << response.type << "\n";

        //Close the connection
        shutdown(sock, SHUTDOWN_BOTH);
        CLOSE_SOCKET(sock);
    }
    stop_socket_library();
    return 0;
}
