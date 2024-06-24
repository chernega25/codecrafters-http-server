#include "http_response.hpp"

std::string http_response::make_response() {
    std::string response = "HTTP/1.1 ";
    
    std::string status;
    switch (code) {
        case 200: status = "200 OK"; break;
        case 201: status = "201 Created"; break;
        case 404: status = "404 Not Found"; break;
        default:;
    }

    response += status + "\r\n";

    if (!body.empty()) {
        headers["Content-Length"] = std::to_string(body.length());
    }

    for (const auto& header : headers) {
        response += header.first + ": " + header.second + "\r\n";
    }

    response += "\r\n" + body;

    return response;
}