#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

struct http_request{
    std::vector<std::string> path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    http_request(std::string request);
};

#endif //HTTP_REQUEST_HPP