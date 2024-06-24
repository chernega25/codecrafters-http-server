// #include <thread>

#include "http_server.hpp"

const int MAX_CONNECTIONS = 10;

int main(int argc, char **argv) {
  	// Flush after every std::cout / std::cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	std::string directory = "";
	for (auto i = 0; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--directory") {
            if (i + 1 < argc) {
                directory = argv[i + 1];
                ++i;
            } else {
                std::cerr << "--directory option requires an argument.\n";
                return 1;
            }
        }
    }


	http_server server(directory);
	if (server.create_socket()) return 1;

	int connections = MAX_CONNECTIONS;

	// std::vector<std::thread> threads;
	while(true) {
		server.accept_client();
	}

	return 0;
}
