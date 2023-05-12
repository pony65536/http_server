#include <cstdlib>
#include <unordered_map>
#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define DEFAULT_PATH "/index.html"
#define FOLDER_ABSOLUTE_PATH "test_path"

int GetFileSize(const std::string &file_path)
{
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

struct request_content
{
    std::string function;
    std::string path;
    std::string protocol;
    std::unordered_map<std::string, std::string> headers;
};

int SendText(const int connectfd, const std::string &file_path)
{

    std::unordered_map<std::string, std::string> file_type{
        {"html", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"}};

    std::string reply = std::string("HTTP/1.1 200 OK\nContent-Type:text/html; charset=utf-8\nContent-Length: ") + std::to_string(GetFileSize(file_path)) + std::string("\n\n");
    write(connectfd, reply.c_str(), reply.length());

    char write_buffer[BUFFER_SIZE] = {0};
    std::ifstream file(file_path.c_str(), std::ios::in);
    while (!file.eof())
    {
        file.read(write_buffer, BUFFER_SIZE);
        int read_size = file.gcount();
        write(connectfd, write_buffer, read_size);
    }
    file.close();
    return 0;
}

request_content AnalysisRequest(std::string request) {
    std::stringstream ssin(request);
    std::string line;
    std::string relative_path;

    request_content r;
    std::getline(ssin, r.function, ' ');
    std::getline(ssin, relative_path, ' ');
    std::getline(ssin, r.protocol, '\n');

    relative_path =  relative_path == "/" ? DEFAULT_PATH : relative_path;
    r.path = FOLDER_ABSOLUTE_PATH + relative_path;

    while (std::getline(ssin, line, '\n'))
    {
        if (line == "") 
            break;
        
        std::stringstream line_stream(line);
        std::string k;
        std::string v;
        std::getline(line_stream, k, ':');
        std::getline(line_stream, v, '\n');
        r.headers.insert({k, v});
    }
    return r;
}

int main(int argc, char *argv[])
{
    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    const struct sockaddr_in server_info = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = htonl(INADDR_ANY),
    };

    assert(bind(listenfd, (sockaddr *)&server_info, sizeof(server_info)) == 0);

    assert(listen(listenfd, 32) == 0);

    while (true)
    {
        const int connectfd = accept(listenfd, nullptr, nullptr);

        pid_t pid = fork();

        assert(pid != -1);

        if (pid == 0)
        {
            char read_buffer[BUFFER_SIZE] = {0};

            int len = read(connectfd, read_buffer, BUFFER_SIZE);
            const std::string request(std::move(read_buffer), len);
            //std::cout << request << std::endl;

            request_content r = AnalysisRequest(request);

            SendText(connectfd, r.path);

            close(connectfd);
            return 0;
        }
    }

    return 0;
}