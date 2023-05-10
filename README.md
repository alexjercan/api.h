# api.h

Simple header-only library for creating an API.

## Examples

```console
cd examples
make
```

1. Curl

Basic example, this will just respond with a string that contains a simple
message for different routes.

In one terminal start the http server

```console
./curl/main
```

Then you can try to make curl requests to it

```console
curl localhost:8080/
curl localhost:8080/another
curl localhost:8080/notexist
```

2. HTML

Simple example that reads an html file from disk and sends it's contents as
response.

In one terminal start the http server

```console
./html/main
```

Then you can try to make curl requests to it or open it in browser

```console
curl localhost:8080
```
