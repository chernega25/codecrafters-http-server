#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>

struct http_response {
    int code;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string make_response();
};

#endif // HTTP_RESPONSE_HPP