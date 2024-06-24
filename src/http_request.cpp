#include "http_request.hpp"

std::vector<std::string> split_message(const std::string &message, const std::string& delim) {
    std::vector<std::string> toks;
    std::stringstream ss = std::stringstream(message);
    std::string line;
    while (getline(ss, line, *delim.begin())) {
        toks.push_back(line);
        ss.ignore(delim.length() - 1);
    }
    return toks;
}

http_request::http_request(std::string request) {
    std::vector<std::string> toks = split_message(request, "\r\n");
    std::vector<std::string> path_toks = split_message(toks[0], " ");
    type = path_toks[0];
    path = split_message(path_toks[1], "/");

    size_t i = 1;
    while (toks[i] != "") {
        std::vector<std::string> split = split_message(toks[i], ": ");
        headers[split[0]] = split[1];
        ++i;
    }

    if (i + 1 < toks.size() && toks[i + 1] != "" && headers.contains("Content-Length")) {
        body = toks[i + 1];
        body.resize(std::stoi(headers["Content-Length"]));
    }
}