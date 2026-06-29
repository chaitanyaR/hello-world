/**
 * Sovd_Server.h  –  ASAM SOVD 1.0 HTTP/REST Server API
 *
 * Implements a minimal HTTP/1.1 TCP server that exposes vehicle ECU data
 * via the ASAM SOVD 1.0 REST interface.  The server runs in a dedicated
 * background thread; all data access goes through Sovd_DataStore which
 * provides the necessary mutual exclusion.
 *
 * Default listen address: 0.0.0.0:8080
 * Base URI prefix:        /sovd/v1
 * Dashboard URI:          /dashboard  (HTML single-page app)
 *
 * Endpoints (ASAM SOVD 1.0 §8-§10):
 *   GET  /sovd/v1/                                        Capabilities
 *   GET  /sovd/v1/components                              List ECUs
 *   GET  /sovd/v1/components/{comp}                       Component info
 *   GET  /sovd/v1/components/{comp}/data-elements         All values
 *   GET  /sovd/v1/components/{comp}/data-elements/{id}    Single value
 *   PUT  /sovd/v1/components/{comp}/data-elements/{id}    Write value
 *   GET  /sovd/v1/components/{comp}/faults                Read DTCs
 *   DELETE /sovd/v1/components/{comp}/faults              Clear DTCs
 *   GET  /sovd/v1/components/{comp}/routines              List routines
 *   POST /sovd/v1/components/{comp}/routines/{id}/start   Execute routine
 */

#ifndef SOVD_SERVER_H
#define SOVD_SERVER_H

#include "Std_Types.h"
#include "Sovd_Types.h"

/* =========================================================================
 * Defaults
 * ====================================================================== */
#define SOVD_DEFAULT_HTTP_PORT     ((uint16)8080u)
#define SOVD_DEFAULT_BACKLOG       ((uint8)8u)
#define SOVD_HTTP_RECV_BUF_SIZE    4096u
#define SOVD_HTTP_RESP_BUF_SIZE    8192u

/* =========================================================================
 * API
 * ====================================================================== */

/**
 * Initialise and start the SOVD HTTP server.
 * Binds to 0.0.0.0:config->httpPort, starts the server accept thread.
 * Returns E_OK on success, E_NOT_OK on socket or thread error.
 */
Std_ReturnType Sovd_Server_Init(const Sovd_ServerConfigType *config);

/**
 * Gracefully shut down the server; waits for the accept thread to exit.
 * Call from main before NodeTransport_Deinit().
 */
void Sovd_Server_Deinit(void);

#endif /* SOVD_SERVER_H */
