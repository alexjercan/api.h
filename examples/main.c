#include <stdio.h>

#define API_IMPLEMENTATION
#include "api.h"

#define HOST "0.0.0.0"
#define PORT 8080

API_Response callback(API_Request request) {
    API_Response response = { .status = 200, .body = "Hello" };

    return response;
}

API_Response another(API_Request request) {
    API_Response response = { .status = 200, .body = "Another" };

    return response;
}

int main() {
    struct API_Router *router = api_create();

    api_route(router, "/", METHOD_GET, *callback);
    api_route(router, "/another", METHOD_GET, *another);

    printf("Listening on %s:%hu\n", HOST, PORT);
    return api_start(router, HOST, PORT);
}
