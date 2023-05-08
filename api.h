#ifndef API_H_
#define API_H_

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct {
    const char *addr;
    uint16_t port;
} API;

API api_create(const char *addr, size_t port);
int api_route(API api, const char *path, void (*callback)(void));
int api_start(API api);

#endif // API_H_

#ifdef API_IMPLEMENTATION

API api_create(const char *addr, size_t port) {
    API api;

    api.addr = addr;
    api.port = port;

    return api;
}

int api_start(API api) {
    int s_fd, result;
    struct sockaddr_in s_addr;

    result = socket(AF_INET, SOCK_STREAM, 0);
    if (result == -1) {
        perror("socket");
        return result;
    }
    s_fd = result;

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(api.port);
    result = inet_pton(AF_INET, api.addr, &s_addr.sin_addr);
    if (result == -1) {
        perror("inet_pton");
        close(s_fd);
        return result;
    }

    result = bind(s_fd, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if (result == -1) {
        perror("bind");
        close(s_fd);
        return result;
    }

    result = listen(s_fd, 5);
    if (result == -1) {
        perror("listen");
        close(s_fd);
        return result;
    }

    while (1)
    {
        int c_fd, result;
        struct sockaddr_in c_addr;
        socklen_t c_addr_size;

        c_addr_size = sizeof(c_addr);
        result = accept(s_fd, (struct sockaddr *)&c_addr, &c_addr_size);
        if (result == -1) {
            perror("accept");
            continue;
        }
        c_fd = result;

        {
            char buf[2048];
            char *response = "HTTP/1.1 200 OK\n\nHi";
            memset(buf, 0, 2048);
            read(c_fd, buf, sizeof(buf));
            printf("From client:\n%s\n", buf);

            printf("From server:\n%s\n", response);
            write(c_fd, response, strlen(response));
        }

        close(c_fd);
    }

    return close(s_fd);
}

#endif // API_IMPLEMENTATION
