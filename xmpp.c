#include "xmpp.h"
#include <strophe.h>

/* The connection to the XMPP server */
static xmpp_conn_t *conn;

/* Private functions */
static void conn_handler(xmpp_conn_t *const, const xmpp_conn_event_t, const int error,
                         xmpp_stream_error_t *const, void *const);

/* API functions */
int start_xmpp(const char *jid, const char *password) {
	xmpp_ctx_t *ctx;
	
	/* Initialise the xmpp library */
	xmpp_initialize();
	
	/* Create a context and connection */
	ctx = xmpp_ctx_new(NULL, NULL);
	conn = xmpp_conn_new(ctx);
	
	/* Set the authentication information */
	xmpp_conn_set_jid(conn, jid);
	xmpp_conn_set_pass(conn, password);
	
	/* Attempt to connect to the XMPP server */
	xmpp_connect_client(conn, NULL, 0, conn_handler, ctx);
	
    return 0;
}

int stop_xmpp(void) {
	xmpp_disconnect(conn);
	xmpp_conn_release(conn);
	xmpp_shutdown();
    return 0;
}

static void conn_handler(xmpp_conn_t *const connection, const xmpp_conn_event_t status, const int error,
                         xmpp_stream_error_t *const stream_errir, void *const user_data) {
	
}
