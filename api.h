#ifndef API_H_
#define API_H_

typedef enum  {
    METHOD_GET = 0,
    METHO_POST,

    METHODS_COUNT,
    METHOD_UNDEFINED = -1,
} API_Method;

typedef struct API_Request {
    char *buffer;
} API_Request;

typedef struct API_Response {
    char *buffer;
} API_Response;

struct API_Router;

typedef struct API_Response (*API_Callback)(struct API_Request);

struct API_Router* api_create();
int api_route(struct API_Router *router, const char *path, API_Method method, API_Callback callback);
int api_start(struct API_Router *router, const char *addr, unsigned short port);
void api_destroy(struct API_Router *router);

#endif // API_H_

#ifdef API_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 2048

typedef struct API_Router {

} API_Router;

static void handle_connection(int s_fd);
static int read_request(int c_fd, API_Request *request);
static API_Response build_response(API_Request request);
static int write_response(int c_fd, API_Response response);

API_Router* api_create() {
    API_Router *router = malloc(sizeof(API_Router));

    return router;
}

int api_route(API_Router *router, const char *path, API_Method method, API_Callback callback) {
    return 0;
}

int api_start(API_Router *router, const char *addr, uint16_t port) {
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
    s_addr.sin_port = htons(port);
    result = inet_pton(AF_INET, addr, &s_addr.sin_addr);
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

    // TODO: Should change from 5 to some settings
    result = listen(s_fd, 5);
    if (result == -1) {
        perror("listen");
        close(s_fd);
        return result;
    }

    // TODO: How to do gracefull shutdown?
    while (1) {
        handle_connection(s_fd);
    }

    result = close(s_fd);
    if (result == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

static void handle_connection(int s_fd) {
    int c_fd, result;
    struct sockaddr_in c_addr;
    socklen_t c_addr_size;
    API_Request request;
    API_Response response;

    c_addr_size = sizeof(c_addr);
    result = accept(s_fd, (struct sockaddr *)&c_addr, &c_addr_size);
    if (result == -1) {
        return perror("accept");
    }
    c_fd = result;

    result = read_request(c_fd, &request);
    if (result == -1) {
        return perror("read_request");
    }

    response = build_response(request);

    result = write_response(c_fd, response);
    if (result == -1) {
        return perror("write_response");
    }

    result = close(c_fd);
    if (result == -1) {
        return perror("close");
    }
}

static int read_request(int c_fd, API_Request *request) {
    char buf[BUF_SIZE];
    char *buffer = malloc(sizeof(char));
    size_t buffer_size = 0;

    if (buffer == NULL) {
        return -1;
    }
    buffer[0] = '\0';

    do {
        ssize_t n_bytes = 0;
        char *new_buffer = NULL;

        memset(buf, 0, BUF_SIZE);
        n_bytes = read(c_fd, buf, sizeof(buf));

        if (n_bytes == -1) {
            if (buffer != NULL) {
                free(buffer);
            }
            return -1;
        }

        if (n_bytes == 0) {
            break;
        }

        new_buffer = realloc(buffer, n_bytes + 1);
        if (new_buffer == NULL) {
            free(buffer);
            return -1;
        }
        buffer = new_buffer;

        memcpy(buffer + buffer_size, buf, n_bytes);
        buffer_size += n_bytes;

        if (buffer[buffer_size - 3] == '\n' && buffer[buffer_size - 1] == '\n') {
            break;
        }
    } while(1);

    buffer[buffer_size] = '\0';
    request->buffer = buffer;

    return 0;
}

static API_Response build_response(API_Request request) {
    // TODO: make this great again
    API_Response response;
    char *buffer = "HTTP/1.1 200 OK\n\nHi";

    response.buffer = buffer;

    return response;
}

static int write_response(int c_fd, API_Response response) {
    char *buffer = response.buffer;
    size_t buffer_size = strlen(buffer);

    do {
        ssize_t n_bytes = write(c_fd, buffer, buffer_size);

        if (n_bytes == -1) {
            return -1;
        }

        buffer += n_bytes;
        buffer_size -= n_bytes;
    } while (buffer_size > 0);

    return 0;
}

#endif // API_IMPLEMENTATION
