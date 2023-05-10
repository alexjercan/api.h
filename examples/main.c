#include <stdio.h>

#define API_IMPLEMENTATION
#include "api.h"

#define HOST "0.0.0.0"
#define PORT 8080
#define UNUSED(x) (void)(x)

API_Response callback(API_Request request) {
    API_Response response = { .status = 200, .body = "Hello" };
    UNUSED(request);

    return response;
}

API_Response another(API_Request request) {
    API_Response response = { .status = 200, .body = "Another" };
    UNUSED(request);

    return response;
}

int main() {
    struct API_Router *router = api_create();

    api_route(router, "/", METHOD_GET, *callback);
    api_route(router, "/another", METHOD_GET, *another);

    printf("Listening on %s:%d\n", HOST, PORT);
    api_start(router, HOST, PORT);

    api_destroy(router);

    return 0;
}
