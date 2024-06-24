#include "http_response.hpp"

void http_response::add_header(std::string name, std::string value) {
    if (headers.contains(name)) {
        headers[name].push_back(value);
    } else {
        headers[name] = std::vector<std::string>(1, value);
    }
}

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
        add_header("Content-Length", std::to_string(body.length()));
    }

    for (const auto& header : headers) {
        response += header.first + ": ";
        for (auto i = 0; i < header.second.size(); ++i) {
            response += header.second[i];
            if (i < header.second.size() - 1) {
                response += ", ";
            }
        }
        response += "\r\n";
    }

    response += "\r\n" + body;

    return response;
}