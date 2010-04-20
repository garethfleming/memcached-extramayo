/*
 * Basic HTTP interface to a memcached instance.
 */

#ifndef _HTTP_H
#define _HTTP_H

typedef struct event_base* event_base_t;

/* Start the HTTP server, binding to the specified address and port */ 
int start_http(char *addr, int port, event_base_t eventbase);

/* Stop the HTTP server */
int stop_http(void);

#endif 