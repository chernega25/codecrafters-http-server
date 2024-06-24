#include "http_server.hpp"

#include <fstream>
#include <zlib.h>
#include <sstream>
#include <iostream>

std::string gzip_compress(const std::string &data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("deflateInit2 failed while compressing.");
    }
    zs.next_in = (Bytef *)data.data();
    zs.avail_in = data.size();
    int ret;
    char outbuffer[32768];
    std::string outstring;
    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        ret = deflate(&zs, Z_FINISH);
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);
    deflateEnd(&zs);
    if (ret != Z_STREAM_END) {
        throw std::runtime_error("Exception during zlib compression: (" + std::to_string(ret) + ") " + zs.msg);
    }
    return outstring;
}


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

void http_server::accept_client() {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

    if (client_fd < 0)
    {
        perror("Error accepting client connection");
        return;
    }

    std::cout << "Client connected\n";

    std::string buffer(1024, '\0');
    ssize_t brecvd = 0;
    
    while (brecvd == 0) {
        brecvd = recv(client_fd, (void *)&buffer[0], buffer.capacity(), MSG_PEEK);
    }

    if (brecvd < 0) {
        perror("recv");
        std::cerr << "Error receiving message from client\n";
        
        close(client_fd);
        return;
    }

    buffer.resize(brecvd);
    brecvd = recv(client_fd, (void *)&buffer[0], buffer.capacity(), 0);

    if (brecvd < 0) {
        perror("recv");
        std::cerr << "Error receiving message from client\n";
        
        close(client_fd);
        return;
    }

    std::cout << buffer << std::endl;
    http_request request(buffer);
    std::string response = process_request(request).make_response();
    std::cout << response << std::endl;

    ssize_t bsent = send(client_fd, response.c_str(), response.length(), 0);
    
    if (bsent < 0)
    {
        std::cerr << "Error sending response to client\n";
        close(client_fd);
        return;
    }

    close(client_fd);
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
                response.body = gzip_compress(response.body);
                break;
            }
        }
    }

    return response;
}