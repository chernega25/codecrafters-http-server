#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>
#include <vector>

struct http_response {
    int code;
    std::unordered_map<std::string, std::vector<std::string>> headers;
    std::string body;

    void add_header(std::string name, std::string value);
    std::string make_response();
};

#endif // HTTP_RESPONSE_HPP