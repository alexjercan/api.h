#include <stdio.h>

#define API_IMPLEMENTATION
#include "api.h"

#define HOST "0.0.0.0"
#define PORT 8080

struct API_Response callback(struct API_Request) {

}

int main() {
    struct API_Router *router = api_create();

    api_route(router, "/", METHOD_GET, *callback);

    printf("Listening on %s:%hu\n", HOST, PORT);
    return api_start(router, HOST, PORT);
}
