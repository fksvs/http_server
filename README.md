HTTP Server
---

For now it can send only a small data, but you can change it from source code.

*Go to function `send_data(int client_sock)` to change the data* 

#### compile:
    gcc http_server.c -o http_server

### usage:
    ./http_server -a (host address) -p (host port)

>open your browser and go to the specified host address-port
