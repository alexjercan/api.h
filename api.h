#ifndef API_H_
#define API_H_

typedef enum  {
    METHOD_GET = 0,
    METHOD_POST,

    METHODS_COUNT,
    METHOD_UNDEFINED = -1,
} API_Method;

typedef struct API_Request {
    char *raw;

    API_Method method;
    const char *path;
} API_Request;

typedef struct API_Response {
    unsigned short status;
    char *body;
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
#include <stdarg.h>

#define BUF_SIZE 2048

typedef struct API_Route {
    const char *path;
    API_Method method;
    API_Callback callback;
} API_Route;

typedef struct API_Router {
    API_Route *items;
    size_t count;
    size_t capacity;

    API_Callback route404;
} API_Router;

static void handle_connection(API_Router *router, int s_fd);
static int read_request(int c_fd, API_Request *request);
static API_Response build_response(API_Router *router, API_Request request);
static int write_response(int c_fd, API_Response response);
static API_Method parse_method(char *buffer);
static char* make_message(const char *fmt, ...);
static API_Response route404(API_Request request);

API_Router* api_create() {
    API_Router *router = malloc(sizeof(API_Router));

    router->items = NULL;
    router->count = 0;
    router->capacity = 0;

    router->route404 = route404;

    return router;
}

int api_route(API_Router *router, const char *path, API_Method method, API_Callback callback) {
    API_Route route = { .path = path, .method = method, .callback = callback };

    if (router->count >= router->capacity) {
        size_t new_capacity = router->capacity * 2;
        API_Route *new_items = NULL;

        if (new_capacity == 0) {
            new_capacity = 32;
        }

        new_items = realloc(router->items, new_capacity * sizeof(API_Route));
        if (new_items == NULL) {
            perror("realloc");
            return -1;
        }

        router->items = new_items;
        router->capacity = new_capacity;
    }

    router->items[router->count++] = route;

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
        handle_connection(router, s_fd);
    }

    result = close(s_fd);
    if (result == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

void api_destroy(struct API_Router *router) {
    free(router->items);
    free(router);
}

static void handle_connection(API_Router *router, int s_fd) {
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

    response = build_response(router, request);

    free(request.raw);

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
    char *token = NULL;

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

    request->raw = buffer;

    token = strsep(&buffer, " ");
    request->method = parse_method(token);

    token = strsep(&buffer, " ");
    request->path = token;

    return 0;
}

static API_Response build_response(API_Router *router, API_Request request) {
    for (size_t i = 0; i < router->count; ++i) {
        API_Route route = router->items[i];
        if (strcmp(route.path, request.path) == 0 && route.method == request.method) {
            return route.callback(request);
        }
    }

    return router->route404(request);
}

static int write_response(int c_fd, API_Response response) {
    char *buffer = NULL;
    size_t buffer_size, buffer_len;

    buffer = make_message( "HTTP/1.1 %d\n\n%s", response.status, response.body);
    if (buffer == NULL) {
        perror("make_message");
        return -1;
    }
    buffer_len = strlen(buffer);
    buffer_size = 0;

    do {
        ssize_t n_bytes = write(c_fd, buffer + buffer_size, buffer_len);

        if (n_bytes == -1) {
            return -1;
        }

        buffer_size += n_bytes;
        buffer_len -= n_bytes;
    } while (buffer_len > 0);

    free(buffer);

    return 0;
}

static API_Method parse_method(char *buffer) {
    if (strcmp(buffer, "GET") == 0) {
        return METHOD_GET;
    }

    if (strcmp(buffer, "POST") == 0) {
        return METHOD_POST;
    }

    return METHOD_UNDEFINED;
}

static char* make_message(const char *fmt, ...) {
    int n = 0;
    size_t size = 0;
    char *p = NULL;
    va_list ap;

    va_start(ap, fmt);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (n < 0) {
        return NULL;
    }

    size = (size_t) n + 1;
    p = malloc(size);
    if (p == NULL) {
        return NULL;
    }

    va_start(ap, fmt);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (n < 0) {
        free(p);
        return NULL;
    }

    return p;
}

static API_Response route404(API_Request request) {
    API_Response response = { .status = 404, .body = "Not found" };

    return response;
}

#endif // API_IMPLEMENTATION
