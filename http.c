#include "http.h"
#include "memcached.h"
#include <evhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The libevent httpd struct */
static struct evhttp *httpd; 

typedef struct evhttp_request* http_request_t;

/* Private function used to handle requests */
static void handler(http_request_t, void*);

/* API functions */
int start_http(char *addr, int port, event_base_t eventbase) {
	event_init();
	httpd = evhttp_new(eventbase);
	evhttp_bind_socket(httpd, addr, port);
	evhttp_set_gencb(httpd, handler, NULL);
	return 0;
}

int stop_http() {
	evhttp_free(httpd);
	return 0;
}

/* Private functions */
static void handle_get(http_request_t, void*);
static void handle_post(http_request_t, void*);

static void reply(http_request_t, const char*);
static void reply_with_search_page(http_request_t);
static void reply_with_value(http_request_t, const char*, const char*);

/* This isn't defined in evhttp.h */
#define HTTP_METHOD_NOT_ALLOWED 405

static const char *kSEARCH_PAGE = "<html><head><title>Memcache Search</head><body></body></html>";

/* 
 * The main request handler function. Routes to another function based
 * on the request type. 
 */
static void handler(http_request_t request, void *arg) {
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

/* 
 * Handler for GET requests. Requests to the root uri return a simple
 * search page. Otherwise, the rest of the uri is taken to be the key
 * being searched for.
 */
static void handle_get(http_request_t request, void *arg) {
	char *key;
	char *value;
	item *item;
	
	if (strcmp(request->uri, "/") == 0) {
		reply_with_search_page(request);
	}
	else {
		key = strdup(request->uri + 1);
		item = item_get(key, strlen(key));
		if (item != NULL) {
			value = ITEM_data(item);
		}
		else {
			value = "null";
		}
		reply_with_value(request, key, value);
	}
}

/*
 * Handler for POST requests. This function sets a value in the cache
 * based on the key and value passed in the body of this request.
 */
static void handle_post(http_request_t request, void *arg) {
	char *buffer;
	buffer = evbuffer_readline(request->input_buffer);
	printf("You sent '%s'\n", buffer);
	free(buffer);
}

/*
 * Sends a response back to the client.
 */
static void reply(http_request_t request, const char *body) {	
	struct evbuffer *buffer;
	buffer = evbuffer_new();
	evbuffer_add_printf(buffer, "%s", body);
	evhttp_send_reply(request, HTTP_OK, "OK", buffer);
	evbuffer_free(buffer);	
}

/*
 * Responds with the search page.
 */
static void reply_with_search_page(http_request_t request) {
	evhttp_add_header(request->output_headers, "Content-Type", "text/html");
	reply(request, kSEARCH_PAGE);
}

/*
 * Looks up a key in the cache and responds with the value.
 */
static void reply_with_value(http_request_t request, const char *key, const char *value) {	
	char *body;
	int body_size;
	body_size = strlen(key) + strlen(value) + strlen("{'':''}\n") + 1;
	body = (char *)malloc(body_size * sizeof(char*));
	sprintf(body, "{\"%s\":\"%s\"}\n", key, value);
	evhttp_add_header(request->output_headers, "Content-Type", "application/json");
	reply(request, body);
	free(body);
}