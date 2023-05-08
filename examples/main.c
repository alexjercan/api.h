#include <stdio.h>

#define API_IMPLEMENTATION
#include "api.h"

int main() {
    API api = api_create("0.0.0.0", 8080);

    printf("Listening on %s:%hu\n", api.addr, api.port);
    return api_start(api);
}
