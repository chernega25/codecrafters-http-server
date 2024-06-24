#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "http_request.hpp"
#include "http_response.hpp"

struct http_server {
    std::string directory;
    int server_fd;

    int create_socket();
    int accept_client();
    http_response process_request(const http_request& request);

    inline http_server(std::string _directory = "") : directory(_directory) {}

    inline ~http_server() {
        close(server_fd);
    };
};

#endif //HTTP_SERVER_HPP