/**
 * Sovd_Server.c  –  ASAM SOVD 1.0 HTTP/1.1 REST Server
 *
 * Minimal, zero-dependency HTTP server built on raw TCP sockets.  Uses the
 * same Platform.h socket abstraction as NodeTransport so it compiles on
 * Linux, Windows (Winsock2), and QNX without modification.
 *
 * Threading model: one persistent accept thread; one detached thread per
 * incoming connection (connection is closed after the response).
 *
 * JSON generation: direct snprintf – no external library required.
 */

#include "Sovd_Server.h"
#include "Sovd_DataStore.h"
#include "Sovd_Types.h"
#include "Platform.h"      /* socket types, sleep, network init */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Platform socket helpers for TCP (additive to Platform.h UDP helpers)
 * ====================================================================== */
#ifdef _WIN32
#  define SOCKOPT_CAST(p)   ((const char *)(p))
   static DWORD WINAPI AcceptThread(LPVOID arg);
   static DWORD WINAPI ConnThread(LPVOID arg);
#else
   static void *AcceptThread(void *arg);
   static void *ConnThread(void *arg);
#  define SOCKOPT_CAST(p)   (p)
#endif

/* =========================================================================
 * Internal state
 * ====================================================================== */
static Platform_Socket_t  g_srv_fd  = PLATFORM_INVALID_SOCKET;
static volatile int        g_running = 0;

#ifdef _WIN32
static HANDLE g_accept_thread;
#else
static pthread_t g_accept_thread;
#endif

/* =========================================================================
 * HTTP helpers
 * ====================================================================== */
static void send_response(Platform_Socket_t fd,
                           int status_code, const char *reason,
                           const char *content_type,
                           const char *body, int body_len)
{
    char hdr[512];
    int  hdr_len;

    if (body == NULL) body_len = 0;

    hdr_len = snprintf(hdr, (int)sizeof(hdr),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, PUT, POST, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, reason,
        content_type ? content_type : "text/plain",
        body_len);

    (void)send(fd, PLATFORM_SBUF(hdr),   PLATFORM_SLEN(hdr_len), 0);
    if (body_len > 0) {
        (void)send(fd, PLATFORM_SBUF(body), PLATFORM_SLEN(body_len), 0);
    }
}

static void send_json(Platform_Socket_t fd, int status, const char *json)
{
    send_response(fd, status,
                  (status == 200) ? "OK"       :
                  (status == 201) ? "Created"  :
                  (status == 204) ? "No Content" :
                  (status == 400) ? "Bad Request" :
                  (status == 404) ? "Not Found" :
                  (status == 405) ? "Method Not Allowed" : "Error",
                  "application/json",
                  json, (int)strlen(json));
}

/* =========================================================================
 * Receive full HTTP request into buffer
 * ====================================================================== */
static int recv_request(Platform_Socket_t fd, char *buf, int buf_len)
{
    int  total = 0;
    int  n;
    char *end_of_hdr;

    while (total < buf_len - 1) {
        n = (int)recv(fd, PLATFORM_RBUF(buf + total), (size_t)(buf_len - 1 - total), 0);
        if (n <= 0) break;
        total += n;
        buf[total] = '\0';

        end_of_hdr = strstr(buf, "\r\n\r\n");
        if (end_of_hdr) {
            /* For bodyless methods: done */
            if ((strncmp(buf, "GET",     3) == 0) ||
                (strncmp(buf, "DELETE",  6) == 0) ||
                (strncmp(buf, "OPTIONS", 7) == 0)) {
                break;
            }
            /* For PUT/POST: wait for Content-Length bytes after headers */
            {
                char *cl = strstr(buf, "Content-Length:");
                if (cl) {
                    int content_len  = atoi(cl + 15);
                    int headers_end  = (int)(end_of_hdr - buf) + 4;
                    if (total >= headers_end + content_len) break;
                } else {
                    break;
                }
            }
        }
    }
    return total;
}

/* =========================================================================
 * Parse request line: extracts method, path into caller-provided buffers
 * ====================================================================== */
static void parse_request_line(const char *req,
                                char *method, int method_len,
                                char *path,   int path_len)
{
    int i = 0, j = 0;
    method[0] = path[0] = '\0';

    while (req[i] && req[i] != ' ' && j < method_len - 1)
        method[j++] = req[i++];
    method[j] = '\0';

    while (req[i] == ' ') i++;

    j = 0;
    while (req[i] && req[i] != ' ' && req[i] != '\r' && j < path_len - 1)
        path[j++] = req[i++];
    path[j] = '\0';
}

/* =========================================================================
 * Parse body for {"value": N}  (covers integers, floats, booleans)
 * ====================================================================== */
static int parse_json_value(const char *req, double *out_val)
{
    const char *body = strstr(req, "\r\n\r\n");
    const char *p;

    if (!body) return -1;
    body += 4;

    p = strstr(body, "\"value\"");
    if (!p) return -1;
    p += 7;
    while (*p == ' ' || *p == ':' || *p == '\t') p++;

    if (strncmp(p, "true", 4) == 0)  { *out_val = 1.0; return 0; }
    if (strncmp(p, "false", 5) == 0) { *out_val = 0.0; return 0; }
    if (*p == '"') { /* quoted number or string */ p++; }

    *out_val = atof(p);
    return 0;
}

/* =========================================================================
 * Path segment splitter
 * ====================================================================== */
#define PATH_SEG_MAX   10
#define PATH_SEG_LEN   64

typedef struct {
    char seg[PATH_SEG_MAX][PATH_SEG_LEN];
    int  n;
} PathSegs;

static void split_path(const char *path, PathSegs *ps)
{
    const char *p = path;
    int i, j;

    ps->n = 0;
    if (*p == '/') p++;

    while (*p && ps->n < PATH_SEG_MAX) {
        j = 0;
        i = ps->n;
        while (*p && *p != '/' && j < PATH_SEG_LEN - 1)
            ps->seg[i][j++] = *p++;
        ps->seg[i][j] = '\0';
        ps->n++;
        if (*p == '/') p++;
    }
}

/* =========================================================================
 * JSON builder helpers
 * ====================================================================== */
#define RESP_SZ  SOVD_HTTP_RESP_BUF_SIZE

static int jbool(uint8 v) { return (int)(v != 0u ? 1 : 0); }

/* --- ADAS data-elements ------------------------------------------------- */
static void json_adas_elements(char *buf, int len)
{
    Sovd_AdasData_t d;
    static const char *modes[] = {"off","highway","city","parking"};
    const char *mode_str;

    Sovd_DataStore_GetAdas(&d);
    mode_str = modes[d.adas_mode < 4u ? d.adas_mode : 0u];

    snprintf(buf, (size_t)len,
        "{\"data-elements\":["
        "{\"id\":\"lane_departure_active\",\"name\":\"Lane Departure Active\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read\",\"value\":%s},"
        "{\"id\":\"collision_warn_level\",\"name\":\"Collision Warning Level\","
          "\"unit\":null,\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"camera_online\",\"name\":\"Front Camera Online\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read\",\"value\":%s},"
        "{\"id\":\"radar_distance_m\",\"name\":\"Radar Object Distance\","
          "\"unit\":\"m\",\"dataType\":\"float32\",\"access\":\"read\",\"value\":%.1f},"
        "{\"id\":\"adas_mode\",\"name\":\"ADAS Operating Mode\","
          "\"unit\":null,\"dataType\":\"uint8\",\"access\":\"read-write\","
          "\"value\":%u,\"valueString\":\"%s\"},"
        "{\"id\":\"fusion_confidence_pct\",\"name\":\"Sensor Fusion Confidence\","
          "\"unit\":\"%%\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u}"
        "]}",
        jbool(d.lane_departure_active) ? "true" : "false",
        d.collision_warn_level,
        jbool(d.camera_online) ? "true" : "false",
        (double)d.radar_distance_m,
        d.adas_mode, mode_str,
        d.fusion_confidence_pct);
}

/* --- ECM data-elements -------------------------------------------------- */
static void json_ecm_elements(char *buf, int len)
{
    Sovd_EcmData_t d;
    Sovd_DataStore_GetEcm(&d);

    snprintf(buf, (size_t)len,
        "{\"data-elements\":["
        "{\"id\":\"engine_rpm\",\"name\":\"Engine Speed\","
          "\"unit\":\"rpm\",\"dataType\":\"uint16\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"coolant_temp_c\",\"name\":\"Coolant Temperature\","
          "\"unit\":\"C\",\"dataType\":\"sint8\",\"access\":\"read\",\"value\":%d},"
        "{\"id\":\"throttle_pos_pct\",\"name\":\"Throttle Position\","
          "\"unit\":\"%%\",\"dataType\":\"uint8\",\"access\":\"read-write\",\"value\":%u},"
        "{\"id\":\"fuel_level_pct\",\"name\":\"Fuel Tank Level\","
          "\"unit\":\"%%\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"engine_running\",\"name\":\"Engine Running\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read\",\"value\":%s},"
        "{\"id\":\"map_kpa\",\"name\":\"Manifold Absolute Pressure\","
          "\"unit\":\"kPa\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u}"
        "]}",
        d.engine_rpm,
        (int)d.coolant_temp_c,
        d.throttle_pos_pct,
        d.fuel_level_pct,
        d.engine_running ? "true" : "false",
        d.map_kpa);
}

/* --- TCU data-elements -------------------------------------------------- */
static void json_tcu_elements(char *buf, int len)
{
    Sovd_TcuData_t d;
    static const char *gears[]  = {"P","R","N","D","M1","M2","M3","M4","M5"};
    static const char *smodes[] = {"automatic","manual","sport"};
    const char *gear_str;
    const char *smode_str;

    Sovd_DataStore_GetTcu(&d);
    gear_str  = gears [d.gear_position < 9u ? d.gear_position : 3u];
    smode_str = smodes[d.shift_mode   < 3u ? d.shift_mode   : 0u];

    snprintf(buf, (size_t)len,
        "{\"data-elements\":["
        "{\"id\":\"gear_position\",\"name\":\"Gear Position\","
          "\"unit\":null,\"dataType\":\"uint8\",\"access\":\"read-write\","
          "\"value\":%u,\"valueString\":\"%s\"},"
        "{\"id\":\"trans_temp_c\",\"name\":\"ATF Temperature\","
          "\"unit\":\"C\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"shift_mode\",\"name\":\"Shift Mode\","
          "\"unit\":null,\"dataType\":\"uint8\",\"access\":\"read-write\","
          "\"value\":%u,\"valueString\":\"%s\"},"
        "{\"id\":\"torque_converter_slip\",\"name\":\"Torque Converter Slip\","
          "\"unit\":\"%%\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"limp_home_active\",\"name\":\"Limp-Home Mode\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read\",\"value\":%s}"
        "]}",
        d.gear_position, gear_str,
        d.trans_temp_c,
        d.shift_mode, smode_str,
        d.torque_converter_slip,
        d.limp_home_active ? "true" : "false");
}

/* --- BCM data-elements -------------------------------------------------- */
static void json_bcm_elements(char *buf, int len)
{
    Sovd_BcmData_t d;
    Sovd_DataStore_GetBcm(&d);

    snprintf(buf, (size_t)len,
        "{\"data-elements\":["
        "{\"id\":\"door_fl_locked\",\"name\":\"Front-Left Door Locked\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read-write\",\"value\":%s},"
        "{\"id\":\"door_fr_locked\",\"name\":\"Front-Right Door Locked\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read-write\",\"value\":%s},"
        "{\"id\":\"door_rl_locked\",\"name\":\"Rear-Left Door Locked\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read-write\",\"value\":%s},"
        "{\"id\":\"door_rr_locked\",\"name\":\"Rear-Right Door Locked\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read-write\",\"value\":%s},"
        "{\"id\":\"headlights_on\",\"name\":\"Headlights\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read-write\",\"value\":%s},"
        "{\"id\":\"hazard_on\",\"name\":\"Hazard Flasher\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read-write\",\"value\":%s},"
        "{\"id\":\"ambient_lux\",\"name\":\"Ambient Light Level\","
          "\"unit\":\"lux\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u}"
        "]}",
        d.door_fl_locked ? "true" : "false",
        d.door_fr_locked ? "true" : "false",
        d.door_rl_locked ? "true" : "false",
        d.door_rr_locked ? "true" : "false",
        d.headlights_on  ? "true" : "false",
        d.hazard_on      ? "true" : "false",
        d.ambient_lux);
}

/* --- Gateway data-elements ---------------------------------------------- */
static void json_gw_elements(char *buf, int len)
{
    Sovd_GwData_t d;
    Sovd_DataStore_GetGw(&d);

    snprintf(buf, (size_t)len,
        "{\"data-elements\":["
        "{\"id\":\"routing_active\",\"name\":\"Cross-Domain Routing\","
          "\"unit\":null,\"dataType\":\"boolean\",\"access\":\"read\",\"value\":%s},"
        "{\"id\":\"bus_load_pct\",\"name\":\"Bus Utilisation\","
          "\"unit\":\"%%\",\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"drop_count\",\"name\":\"PDU Drop Count\","
          "\"unit\":null,\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u},"
        "{\"id\":\"domain_count\",\"name\":\"Active Domains\","
          "\"unit\":null,\"dataType\":\"uint8\",\"access\":\"read\",\"value\":%u}"
        "]}",
        d.routing_active ? "true" : "false",
        d.bus_load_pct,
        d.drop_count,
        d.domain_count);
}

/* --- Fault JSON --------------------------------------------------------- */
static void json_faults(char *buf, int len, const char *comp)
{
    Sovd_FaultStore_t fs;
    static const char *sev_str[] = {
        "none", "maintenance", "check-next-halt", "check-immediately"
    };
    int pos;
    uint8 i;
    int first = 1;

    Sovd_DataStore_GetFaults(comp, &fs);

    pos  = snprintf(buf, (size_t)len, "{\"faults\":[");
    for (i = 0u; i < fs.count; i++) {
        const Sovd_FaultRecordType *f = &fs.faults[i];
        if (!f->active) continue;
        {
            int sev_idx = (f->severity == SOVD_DTC_SEV_CHECK_IMMEDIATELY) ? 3 :
                          (f->severity == SOVD_DTC_SEV_CHECK_AT_NEXT_HALT) ? 2 :
                          (f->severity == SOVD_DTC_SEV_MAINTENANCE_ONLY)   ? 1 : 0;
            const char *status_s = (f->statusByte & SOVD_DTC_CONFIRMED) ? "confirmed" : "pending";

            pos += snprintf(buf + pos, (size_t)(len - pos),
                "%s{\"dtcCode\":\"0x%06X\",\"description\":\"%s\","
                "\"status\":\"%s\",\"severity\":\"%s\",\"statusByte\":\"0x%02X\"}",
                first ? "" : ",",
                f->dtcCode, f->description,
                status_s, sev_str[sev_idx],
                f->statusByte);
            first = 0;
        }
    }
    pos += snprintf(buf + pos, (size_t)(len - pos), "]}");
    (void)pos;
}

/* =========================================================================
 * Route dispatcher
 * ====================================================================== */
static void dispatch(Platform_Socket_t fd,
                     const char *method,
                     const PathSegs *ps,
                     const char *raw_request)
{
    char resp[RESP_SZ];

    /* CORS preflight */
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(fd, 204, "No Content", "text/plain", NULL, 0);
        return;
    }

    /* /dashboard – serve HTML page */
    if ((ps->n == 1) && (strcmp(ps->seg[0], "dashboard") == 0)) {
        /* Try loading from filesystem (development convenience) */
        FILE *f = fopen("dashboard/index.html", "rb");
        if (!f) f = fopen("/usr/local/share/sovd/index.html", "rb");
        if (f) {
            long fsz;
            char *html;
            fseek(f, 0, SEEK_END);
            fsz  = ftell(f);
            fseek(f, 0, SEEK_SET);
            html = (char *)malloc((size_t)fsz + 1u);
            if (html) {
                size_t nr = fread(html, 1u, (size_t)fsz, f);
                if (nr < (size_t)fsz) html[nr] = '\0';
                html[fsz] = '\0';
                send_response(fd, 200, "OK", "text/html; charset=utf-8",
                              html, (int)fsz);
                free(html);
            }
            fclose(f);
        } else {
            send_response(fd, 200, "OK", "text/html; charset=utf-8",
                "<!DOCTYPE html><html><body>"
                "<h1>SOVD Dashboard</h1>"
                "<p>dashboard/index.html not found. "
                "Copy the file to the working directory.</p>"
                "<p>API base: <a href='/sovd/v1/'>/sovd/v1/</a></p>"
                "</body></html>",
                0);  /* strlen used internally via Content-Length */
        }
        return;
    }

    /* All SOVD routes: /sovd/v1/... */
    if ((ps->n < 2) ||
        (strcmp(ps->seg[0], "sovd") != 0) ||
        (strcmp(ps->seg[1], "v1")   != 0)) {
        snprintf(resp, sizeof(resp),
            "{\"error\":\"Not Found\",\"path\":\"%s\"}", ps->n > 0 ? ps->seg[0] : "/");
        send_json(fd, 404, resp);
        return;
    }

    /* GET /sovd/v1 – capabilities */
    if (ps->n == 2) {
        send_json(fd, 200,
            "{\"version\":\"1.0\",\"standard\":\"ASAM SOVD 1.0\","
            "\"autosar\":\"R22-11\","
            "\"components\":[\"adas\",\"ecm\",\"tcu\",\"bcm\",\"gateway\"],"
            "\"node\":\"HPC\",\"port\":8080}");
        return;
    }

    /* /sovd/v1/components/... */
    if ((ps->n >= 3) && (strcmp(ps->seg[2], "components") == 0)) {

        /* GET /sovd/v1/components */
        if (ps->n == 3) {
            send_json(fd, 200,
                "{\"components\":["
                "{\"id\":\"adas\",\"name\":\"ADAS HPC Domain\",\"domain\":\"hpc\"},"
                "{\"id\":\"ecm\",\"name\":\"Engine Control Module\",\"domain\":\"powertrain\"},"
                "{\"id\":\"tcu\",\"name\":\"Transmission Control Unit\",\"domain\":\"powertrain\"},"
                "{\"id\":\"bcm\",\"name\":\"Body Control Module\",\"domain\":\"body\"},"
                "{\"id\":\"gateway\",\"name\":\"Domain Gateway\",\"domain\":\"network\"}"
                "]}");
            return;
        }

        {
            const char *comp = ps->seg[3];
            int comp_known =
                (strcmp(comp, SOVD_COMP_ADAS)    == 0) ||
                (strcmp(comp, SOVD_COMP_ECM)     == 0) ||
                (strcmp(comp, SOVD_COMP_TCU)     == 0) ||
                (strcmp(comp, SOVD_COMP_BCM)     == 0) ||
                (strcmp(comp, SOVD_COMP_GATEWAY) == 0);

            if (!comp_known) {
                snprintf(resp, sizeof(resp),
                    "{\"error\":\"Component not found\",\"id\":\"%s\"}", comp);
                send_json(fd, 404, resp);
                return;
            }

            /* GET /sovd/v1/components/{comp} */
            if (ps->n == 4) {
                snprintf(resp, sizeof(resp),
                    "{\"id\":\"%s\",\"resources\":"
                    "[\"data-elements\",\"faults\",\"routines\"]}", comp);
                send_json(fd, 200, resp);
                return;
            }

            /* /sovd/v1/components/{comp}/data-elements[/{id}] */
            if (strcmp(ps->seg[4], "data-elements") == 0) {

                /* All elements (GET) */
                if (ps->n == 5) {
                    if (strcmp(method, "GET") != 0) {
                        send_json(fd, 405, "{\"error\":\"Method Not Allowed\"}");
                        return;
                    }
                    if      (strcmp(comp, SOVD_COMP_ADAS)    == 0) json_adas_elements(resp, RESP_SZ);
                    else if (strcmp(comp, SOVD_COMP_ECM)     == 0) json_ecm_elements (resp, RESP_SZ);
                    else if (strcmp(comp, SOVD_COMP_TCU)     == 0) json_tcu_elements (resp, RESP_SZ);
                    else if (strcmp(comp, SOVD_COMP_BCM)     == 0) json_bcm_elements (resp, RESP_SZ);
                    else                                            json_gw_elements  (resp, RESP_SZ);
                    send_json(fd, 200, resp);
                    return;
                }

                /* Single element GET or PUT */
                if (ps->n == 6) {
                    const char *elem = ps->seg[5];

                    if (strcmp(method, "GET") == 0) {
                        /* Re-use all-elements and scan for the requested id */
                        char all[RESP_SZ];
                        char *p;
                        char needle[128];

                        if      (strcmp(comp, SOVD_COMP_ADAS)    == 0) json_adas_elements(all, RESP_SZ);
                        else if (strcmp(comp, SOVD_COMP_ECM)     == 0) json_ecm_elements (all, RESP_SZ);
                        else if (strcmp(comp, SOVD_COMP_TCU)     == 0) json_tcu_elements (all, RESP_SZ);
                        else if (strcmp(comp, SOVD_COMP_BCM)     == 0) json_bcm_elements (all, RESP_SZ);
                        else                                            json_gw_elements  (all, RESP_SZ);

                        snprintf(needle, sizeof(needle), "\"id\":\"%s\"", elem);
                        p = strstr(all, needle);
                        if (p) {
                            /* Walk back to '{' and forward to '}' to extract element object */
                            char *start = p;
                            char *end;
                            while (start > all && *(start-1) != '{') start--;
                            start--;
                            end = strchr(p, '}');
                            if (end) {
                                int obj_len = (int)(end - start + 1);
                                snprintf(resp, sizeof(resp), "%.*s", obj_len, start);
                                send_json(fd, 200, resp);
                                return;
                            }
                        }
                        snprintf(resp, sizeof(resp),
                            "{\"error\":\"Data element not found\",\"id\":\"%s\"}", elem);
                        send_json(fd, 404, resp);
                        return;
                    }

                    if (strcmp(method, "PUT") == 0) {
                        double val = 0.0;
                        Sovd_StatusType rc;

                        if (parse_json_value(raw_request, &val) != 0) {
                            send_json(fd, 400,
                                "{\"error\":\"Bad Request\","
                                "\"detail\":\"body must contain {\\\"value\\\": N}\"}");
                            return;
                        }
                        rc = Sovd_DataStore_WriteElement(comp, elem, val);
                        if (rc == SOVD_STATUS_OK) {
                            snprintf(resp, sizeof(resp),
                                "{\"id\":\"%s\",\"value\":%.6g,\"status\":\"written\"}", elem, val);
                            send_json(fd, 200, resp);
                        } else if (rc == SOVD_STATUS_NOT_FOUND) {
                            snprintf(resp, sizeof(resp),
                                "{\"error\":\"Element not writable or not found\","
                                "\"id\":\"%s\"}", elem);
                            send_json(fd, 404, resp);
                        } else {
                            send_json(fd, 400, "{\"error\":\"Write rejected\"}");
                        }
                        return;
                    }

                    send_json(fd, 405, "{\"error\":\"Method Not Allowed\"}");
                    return;
                }
            }

            /* /sovd/v1/components/{comp}/faults */
            if ((ps->n == 5) && (strcmp(ps->seg[4], "faults") == 0)) {
                if (strcmp(method, "GET") == 0) {
                    json_faults(resp, RESP_SZ, comp);
                    send_json(fd, 200, resp);
                    return;
                }
                if (strcmp(method, "DELETE") == 0) {
                    Sovd_DataStore_ClearFaults(comp);
                    send_json(fd, 200,
                        "{\"status\":\"faults cleared\"}");
                    return;
                }
                send_json(fd, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }

            /* /sovd/v1/components/{comp}/routines */
            if (strcmp(ps->seg[4], "routines") == 0) {
                if ((ps->n == 5) && (strcmp(method, "GET") == 0)) {
                    snprintf(resp, sizeof(resp),
                        "{\"routines\":["
                        "{\"id\":\"ecu_reset\",\"name\":\"ECU Software Reset\","
                         "\"description\":\"Performs a controlled ECU restart\"},"
                        "{\"id\":\"sensor_calibration\",\"name\":\"Sensor Self-Calibration\","
                         "\"description\":\"Runs internal sensor offset calibration\"}"
                        "]}");
                    send_json(fd, 200, resp);
                    return;
                }
                /* POST /routines/{id}/start */
                if ((ps->n == 7) &&
                    (strcmp(ps->seg[6], "start") == 0) &&
                    (strcmp(method, "POST") == 0)) {
                    snprintf(resp, sizeof(resp),
                        "{\"routineId\":\"%s\",\"status\":\"started\","
                        "\"component\":\"%s\"}", ps->seg[5], comp);
                    send_json(fd, 200, resp);
                    return;
                }
            }
        }
    }

    /* Fallthrough 404 */
    snprintf(resp, sizeof(resp),
        "{\"error\":\"Not Found\",\"hint\":\"Try GET /sovd/v1/\"}");
    send_json(fd, 404, resp);
}

/* =========================================================================
 * Connection handler (runs in its own thread)
 * ====================================================================== */
typedef struct { Platform_Socket_t fd; } ConnArgs;

#ifdef _WIN32
static DWORD WINAPI ConnThread(LPVOID arg)
#else
static void *ConnThread(void *arg)
#endif
{
    ConnArgs  *ca  = (ConnArgs *)arg;
    Platform_Socket_t fd = ca->fd;
    char  req_buf[SOVD_HTTP_RECV_BUF_SIZE];
    char  method[16], path[256];
    PathSegs ps;

    free(ca);

    if (recv_request(fd, req_buf, (int)sizeof(req_buf)) > 0) {
        parse_request_line(req_buf, method, (int)sizeof(method),
                                   path,   (int)sizeof(path));
        split_path(path, &ps);
        dispatch(fd, method, &ps, req_buf);
    }

    PLATFORM_CLOSE_SOCKET(fd);

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

/* =========================================================================
 * Accept loop (server background thread)
 * ====================================================================== */
#ifdef _WIN32
static DWORD WINAPI AcceptThread(LPVOID arg)
#else
static void *AcceptThread(void *arg)
#endif
{
    (void)arg;

    while (g_running) {
        struct sockaddr_in  cli_addr;
        Platform_SockLen_t  cli_len = (Platform_SockLen_t)sizeof(cli_addr);
        Platform_Socket_t   cli_fd;

        cli_fd = accept(g_srv_fd, (struct sockaddr *)&cli_addr, &cli_len);

        if (!PLATFORM_SOCKET_IS_VALID(cli_fd)) {
            if (g_running) {
                Platform_SleepMs(10u);
            }
            continue;
        }

        {
            ConnArgs *ca = (ConnArgs *)malloc(sizeof(ConnArgs));
            if (ca) {
                ca->fd = cli_fd;
#ifdef _WIN32
                {
                    HANDLE t = CreateThread(NULL, 0, ConnThread, ca, 0, NULL);
                    if (t) CloseHandle(t);
                    else { free(ca); PLATFORM_CLOSE_SOCKET(cli_fd); }
                }
#else
                {
                    pthread_t t;
                    if (pthread_create(&t, NULL, ConnThread, ca) == 0) {
                        pthread_detach(t);
                    } else {
                        free(ca);
                        PLATFORM_CLOSE_SOCKET(cli_fd);
                    }
                }
#endif
            } else {
                PLATFORM_CLOSE_SOCKET(cli_fd);
            }
        }
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

/* =========================================================================
 * Public: Init / Deinit
 * ====================================================================== */
Std_ReturnType Sovd_Server_Init(const Sovd_ServerConfigType *config)
{
    struct sockaddr_in  srv_addr;
    int                 opt = 1;
    uint16              port;

    port = (config && config->httpPort) ? config->httpPort : SOVD_DEFAULT_HTTP_PORT;

    g_srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!PLATFORM_SOCKET_IS_VALID(g_srv_fd)) {
        fprintf(stderr, "[SOVD] socket() failed\n");
        return E_NOT_OK;
    }

    (void)setsockopt(g_srv_fd, SOL_SOCKET, SO_REUSEADDR,
                     SOCKOPT_CAST(&opt), (Platform_SockLen_t)sizeof(opt));

    (void)memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family      = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port        = htons(port);

    if (bind(g_srv_fd, (struct sockaddr *)&srv_addr,
             (Platform_SockLen_t)sizeof(srv_addr)) != 0) {
        fprintf(stderr, "[SOVD] bind() failed on port %u\n", port);
        PLATFORM_CLOSE_SOCKET(g_srv_fd);
        g_srv_fd = PLATFORM_INVALID_SOCKET;
        return E_NOT_OK;
    }

    if (listen(g_srv_fd, SOVD_DEFAULT_BACKLOG) != 0) {
        fprintf(stderr, "[SOVD] listen() failed\n");
        PLATFORM_CLOSE_SOCKET(g_srv_fd);
        g_srv_fd = PLATFORM_INVALID_SOCKET;
        return E_NOT_OK;
    }

    g_running = 1;

#ifdef _WIN32
    g_accept_thread = CreateThread(NULL, 0, AcceptThread, NULL, 0, NULL);
    if (!g_accept_thread) {
        fprintf(stderr, "[SOVD] CreateThread failed\n");
        g_running = 0;
        PLATFORM_CLOSE_SOCKET(g_srv_fd);
        return E_NOT_OK;
    }
#else
    if (pthread_create(&g_accept_thread, NULL, AcceptThread, NULL) != 0) {
        fprintf(stderr, "[SOVD] pthread_create failed\n");
        g_running = 0;
        PLATFORM_CLOSE_SOCKET(g_srv_fd);
        return E_NOT_OK;
    }
#endif

    printf("[SOVD] HTTP server listening on http://0.0.0.0:%u\n", port);
    printf("[SOVD] Dashboard: http://localhost:%u/dashboard\n", port);
    printf("[SOVD] API root:  http://localhost:%u/sovd/v1/\n", port);
    return E_OK;
}

void Sovd_Server_Deinit(void)
{
    g_running = 0;
    if (PLATFORM_SOCKET_IS_VALID(g_srv_fd)) {
        PLATFORM_CLOSE_SOCKET(g_srv_fd);
        g_srv_fd = PLATFORM_INVALID_SOCKET;
    }
#ifdef _WIN32
    if (g_accept_thread) {
        WaitForSingleObject(g_accept_thread, 2000u);
        CloseHandle(g_accept_thread);
        g_accept_thread = NULL;
    }
#else
    pthread_join(g_accept_thread, NULL);
#endif
}
