#include <stdio.h>
#include <string.h>

#define API_IMPLEMENTATION
#include "api.h"

#define HOST "0.0.0.0"
#define PORT 8080
#define UNUSED(x) (void)(x)

static char* get_file_prefix(const char* path) {
    const char* last_slash = strrchr(path, '/');
    size_t prefix_len;
    char* prefix = NULL;

    if (last_slash == NULL) {
        return NULL;
    }

    prefix_len = last_slash - path;
    prefix = (char*) malloc(prefix_len + 1);
    if (prefix == NULL) {
        perror("malloc");
        return NULL;
    }

    strncpy(prefix, path, prefix_len);
    prefix[prefix_len] = '\0';

    return prefix;
}

static char* append_path(char *path, const char *name) {
    size_t buffer_len = strlen(path) + strlen(name) + 1;
    char *buffer = realloc(path, buffer_len);

    if (buffer == NULL) {
        perror("realloc");
        return NULL;
    }

    return strcat(buffer, name);
}


static char* read_file(char *name) {
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen(name, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char*) malloc((file_size + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("malloc");
        return NULL;
    }

    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = '\0';

    fclose(file);

    return buffer;
}

static char *index_html;
static API_Response callback(API_Request request) {
    API_Response response;
    UNUSED(request);

    char *buffer = read_file(index_html);
    if (buffer == NULL) {
        response.status = 500;
        response.body = "";
    } else {
        response.status = 200;
        response.body = buffer;
    }

    return response;
}

int main(int argc, char **argv) {
    char *prefix = get_file_prefix(*argv);
    char *path = append_path(prefix, "/index.html");
    if (path == NULL) {
        free(prefix);
        return -1;
    }
    index_html = path;

    struct API_Router *router = api_create();

    api_route(router, "/", METHOD_GET, *callback);

    printf("Listening on %s:%d\n", HOST, PORT);
    api_start(router, HOST, PORT);

    api_destroy(router);

    free(path);

    return 0;
}

