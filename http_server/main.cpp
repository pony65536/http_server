#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 50000

int main(int argc, char *argv[]) {
    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    const struct sockaddr_in server_info = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = htonl(INADDR_ANY),
    };

    bind(listenfd, (sockaddr *)&server_info, sizeof(server_info));

    listen(listenfd, 32);

    const int connectfd = accept(listenfd, nullptr, nullptr);

    char read_buffer[BUFFER_SIZE] = {0};

    read(connectfd, read_buffer, BUFFER_SIZE);

    std::cout << read_buffer;

    return 0;
}