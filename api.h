#ifndef API_H_
#define API_H_

#include <stddef.h>

// TODO: Create struct for API
// TODO: api route method, url params, query params, etc.

void api_create();
void api_route(const char* path, void (*callback)(void));
void api_start(const char* addr, size_t port);

#endif // API_H_

#ifdef API_IMPLEMENTATION

#endif // API_IMPLEMENTATION
