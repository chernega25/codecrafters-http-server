#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <sstream>
#include <unordered_map>

std::vector<std::string> split_message(const std::string &message, const std::string& delim) {
  std::vector<std::string> toks;
  std::stringstream ss = std::stringstream{message};
  std::string line;
  while (getline(ss, line, *delim.begin())) {
    toks.push_back(line);
    ss.ignore(delim.length() - 1);
  }
  return toks;
}

struct http_request{
  std::string path;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  http_request(std::string request) {
    auto toks = split_message(request, "\r\n");
    auto path_toks = split_message(toks[0], " ");
    path = path_toks[1];

    size_t i = 1;
    while (toks[i] != "") {
      auto split = split_message(toks[i], ": ");
      headers[split[0]] = split[1];
      ++i;
    }

    body = toks[i + 1];
  }
};

const int MAX_CONNECTIONS = 10;

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Uncomment this block to pass the first stage
  //
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";

  int connections = MAX_CONNECTIONS;

  while(connections--) {
  
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

    if (client_fd < 0)
    {
      std::cerr << "error handling client connection\n";
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
    auto split_path = split_message(request.path, "/");
    std::string response;
    if (request.path == "/") {
      response = "HTTP/1.1 200 OK\r\n\r\n";
    } else if (split_path[1] == "echo") {
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(split_path[2].length()) + "\r\n\r\n" + split_path[2];
    } else if (split_path[1] == "user-agent") {
      auto user_agent = request.headers["User-Agent"];
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(user_agent.length()) + "\r\n\r\n" + user_agent;
    } else {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }

    ssize_t bsent = send(client_fd, response.c_str(), response.length(), 0);
    
    if (bsent < 0)
    {
      std::cerr << "Error sending response to client\n";
      close(client_fd);
      close(server_fd);
      return 1;
    }

    close(client_fd);
  }
  
  close(server_fd);
  return 0;
}
