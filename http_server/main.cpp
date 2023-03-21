#include <cstdlib>
#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int GetFileSize(const std::string& file_path) {
    std::ifstream file(file_path.c_str(), std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file" << std::endl;
        return -1;
    }
    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    file.close();
    return file_size;
}

int main(int argc, char *argv[]) {
    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    const struct sockaddr_in server_info = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = htonl(INADDR_ANY),
    };

    assert(bind(listenfd, (sockaddr *)&server_info, sizeof(server_info)) == 0);

    assert(listen(listenfd, 32) == 0);

    while (true) {
        const int connectfd = accept(listenfd, nullptr, nullptr);

        pid_t pid = fork();

        assert(pid != -1);

        if (pid == 0) {
            char buffer[BUFFER_SIZE] = {0};

            int len = read(connectfd, buffer, BUFFER_SIZE);
            const std::string request(std::move(buffer), len);

            std::string file_path = std::string("index.html");


            char a[100] = "hello world!\n";
            std::string reply = std::string("HTTP/1.1 200 OK\nContent-Type:text/html; charset=utf-8\nContent-Length: ") + std::to_string(GetFileSize(file_path)) + std::string("\n\n");
            write(connectfd, reply.c_str(), reply.length());

            char write_buffer[BUFFER_SIZE] = {0};
            std::ifstream file(file_path.c_str(), std::ios::in);
            while(!file.eof()) {
                file.read(write_buffer, BUFFER_SIZE);
                int read_size = file.gcount();
                write(connectfd, write_buffer, read_size);
            }
            file.close();

            close(connectfd);
            return 0;
        }

    }

    return 0;
}