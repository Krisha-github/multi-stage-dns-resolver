CS3104L Networks Laboratory
Group 41: Multi-stage DNS Resolving System

COMPONENTS
1. Client application
2. Proxy Server application
3. DNS Server application

COMMUNICATION
Client <-> Proxy Server: TCP
Proxy Server <-> DNS Server: TCP

REQUESTS
Type 1: Domain Name -> IP Address
Type 2: IP Address -> Domain Name

RESPONSES
Type 3: successful result
Type 4: entry not found in the database

CACHE
The Proxy keeps exactly three domain/IP mappings. It checks the cache before contacting DNS. Successful DNS results are inserted using FIFO replacement. A cache hit does not change the FIFO order.

COMMAND LINE
Client:
  ./client_app <Proxy IP Address> <Proxy Port>
Proxy Server:
  ./proxy_app <Proxy Port>
DNS Server:
  ./dns_app <DNS Server Port>

The Proxy takes only its listening port on the command line (same prototype as any server). The DNS Server address is read at startup from dns_config.txt in the working directory, in the form:
  <DNS_IP> <DNS_Port>
No port number is hard-coded in the source.

DATABASE
The DNS Server reads dns_records.txt from the working directory. Each line contains:
  Domain_Name IP_Address

BUILD
Linux/WSL:
  make clean
  make

Windows MinGW:
  mingw32-make clean
  mingw32-make

RUN FROM THE PROJECT DIRECTORY
Terminal 1:
  ./dns_app 9002
Terminal 2:
  ./proxy_app 9001   (dns_config.txt contains: 127.0.0.1 9002)
Terminal 3:
  ./client_app 127.0.0.1 9001

Windows:
  dns_app.exe 9002
  proxy_app.exe 9001
  client_app.exe 127.0.0.1 9001

DEMO
1. Type 1: example.com -> 93.184.216.34 (DNS lookup/cache insertion).
2. Repeat example.com -> cache hit.
3. Query openai.com and google.com, then iitg.ac.in to overflow the 3-entry cache.
4. Query example.com again to demonstrate FIFO eviction.
5. Type 2: 142.250.183.14 -> google.com.
6. Query missing.example -> Type 4.
7. Press Ctrl+C to stop servers.

MESSAGE IMPLEMENTATION
A fixed-size WireMessage is used so the three programs share the same protocol. The 32-bit type is transmitted in network byte order. send_complete/receive_complete handle TCP partial transfers.

GRACEFUL CLOSURE
The Client closes the Client-Proxy connection after every response. The Proxy closes both the Client connection and its temporary Proxy-DNS connection after each negotiation. The DNS Server closes each Proxy connection after responding. SIGINT closes the server listening socket.

ASSUMPTIONS
- The Proxy learns the DNS Server IP and port from dns_config.txt because it must initiate the second TCP stage and the server prototype allows only a port argument.
- If the DNS Server is unreachable, the Proxy reports the transport failure and closes that negotiation without fabricating a Type 4 response. Type 4 is reserved for a DNS lookup that completes and finds no matching database entry.
- The DNS database is a local text file named dns_records.txt.
- Requests are handled sequentially; Application #4 does not specify a concurrent server requirement.
