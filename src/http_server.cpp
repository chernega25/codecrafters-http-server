#include "http_server.hpp"

#include <fstream>

int http_server::create_socket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }
  
    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }
  
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);
  
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }
  
    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    return 0;
}

int http_server::accept_client() {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

    if (client_fd < 0)
    {
        perror("Error accepting client connection");
        close(server_fd);
        return 1;
    }

    std::cout << "Client connected\n";

    std::string buffer(1024, '\0');
    ssize_t brecvd = recv(client_fd, (void *)&buffer[0], buffer.max_size(), 0);
    
    if (brecvd < 0)
    {
        std::cerr << "Error receiving message from client\n";
        close(client_fd);
        close(server_fd);
        return 1;
    }

    http_request request(buffer);
    std::string response = process_request(request).make_response();

    ssize_t bsent = send(client_fd, response.c_str(), response.length(), 0);
    
    if (bsent < 0)
    {
        std::cerr << "Error sending response to client\n";
        close(client_fd);
        close(server_fd);
        return 1;
    }

    close(client_fd);
    return 0;
}

http_response http_server::process_request(const http_request& request) {
    http_response response;

    if (request.path.size() == 1) {
        response.code = 200;
    } else if (request.path[1] == "echo") {
        response.code = 200;
        response.add_header("Content-Type", "text/plain");
        response.body = request.path[2];
    } else if (request.path[1] == "user-agent") {
        response.code = 200;
        response.add_header("Content-Type", "text/plain");
        response.body = request.headers.at("User-Agent")[0];
    } else if (request.path[1] == "files") {
        std::string path = directory + request.path[2];

        if (request.type == "GET") {
            std::ifstream file(path);
            if (file.is_open()) {
                response.code = 200;
                response.add_header("Content-Type", "application/octet-stream");
                std::stringstream buffer;
                buffer << file.rdbuf();
                response.body = buffer.str();
                file.close();
            } else {
                response.code = 404;
            }
        } else {
            std::ofstream file(path);
            file << request.body;
            file.close();
            response.code = 201;
        }

    } else {
        response.code = 404;
    }

    auto enc_it = request.headers.find("Accept-Encoding");
    if (enc_it != request.headers.end() && response.code != 404) {
        for (const auto& value : enc_it->second) {
            if (value == "gzip") {
                response.add_header("Content-Encoding", "gzip");
                break;
            }
        }
    }

    return response;
}