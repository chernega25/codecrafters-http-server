#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

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
  
  int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

  if (client_fd < 0)
  {
    std::cerr << "error handling client connection\n";
    close(server_fd);
    return 1;
  }

  std::cout << "Client connected\n";

  std::string request(1024, '\0');
  ssize_t brecvd = recv(client_fd, (void *)&request[0], request.max_size(), 0);
  
  if (brecvd < 0)
  {
    std::cerr << "error receiving message from client\n";
    close(client_fd);
    close(server_fd);
    return 1;
  }

  std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";

  auto pos = request.find('\r'); //GET /echo/abc HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n
  auto echo  = request.substr(10, pos - 19);
  std::cout << echo << std::endl;

  response += std::to_string(echo.length()) + "\r\n\r\n" + echo;

  ssize_t bsent = send(client_fd, response.c_str(), response.length(), 0);
  
  if (bsent < 0)
  {
    std::cerr << "error sending response to client\n";
    close(client_fd);
    close(server_fd);
    return 1;
  }

  close(client_fd);
  close(server_fd);
  return 0;
}
