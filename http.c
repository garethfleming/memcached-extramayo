#include "http.h"
#include "memcached.h"
#include <evhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct evhttp *httpd; 

typedef struct evhttp_request* http_request;

#define HTTP_METHOD_NOT_ALLOWED 405

static void handler(http_request request, void *arg);
static void handle_get(http_request request, void *arg);
static void handle_post(http_request request, void *arg);

int start_http(char *addr, int port, event_base_t eventbase)
{
	event_init();
	httpd = evhttp_new(eventbase);
	evhttp_bind_socket(httpd, addr, port);
	evhttp_set_gencb(httpd, handler, NULL);
	return 0;
}

int stop_http()
{
	evhttp_free(httpd);
	return 0;
}

static void handler(http_request request, void *arg)
{
	if (request->kind == EVHTTP_REQ_GET) {
		handle_get(request, arg);
	}
	else if (request->kind == EVHTTP_REQ_POST) {
		handle_post(request, arg);
	}
	else {
		evhttp_send_error(request, HTTP_METHOD_NOT_ALLOWED, "Method Not Allowed");
	}
}

static void handle_get(http_request request, void *arg)
{
	struct evbuffer *buffer;
	char *key;
	char *value;
	item *item;
	
	key = strdup(request->uri + 1);
	item = item_get(key, strlen(key));
	if (item != NULL) {
		value = ITEM_data(item);
	}
	else {
		value = "null";
	}
	buffer = evbuffer_new();
	evbuffer_add_printf(buffer, "{\"%s\":\"%s\"}\n", key, value);
	free(key);
	evhttp_add_header(request->output_headers, "Content-Type", "application/json");
	evhttp_send_reply(request, HTTP_OK, "OK", buffer);
}

static void handle_post(http_request request, void *arg)
{
}