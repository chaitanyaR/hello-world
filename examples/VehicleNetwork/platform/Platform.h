/**
 * Platform.h – OS/Platform Portability Layer
 * AUTOSAR R22-11 Vehicle Network Host Simulation
 *
 * Abstracts over socket, thread and timer APIs so that NodeTransport.c
 * and every node main compile without modification on:
 *
 *   Linux / QNX / macOS  – POSIX sockets + pthreads
 *   Windows 10/11        – Winsock2 + Win32 threads
 *
 * Usage in NodeTransport.c:
 *   #include "Platform.h"     (replaces <sys/socket.h>, <pthread.h>, <unistd.h>)
 *
 * Usage in node main files:
 *   #include "Platform.h"     (provides Platform_SleepMs / Platform_SleepUs)
 *
 * Design rule: zero runtime overhead – all wrappers are static inline.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/* =========================================================================
 * Windows – Winsock2 + Win32 threads
 * ====================================================================== */
#ifdef _WIN32

# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN   /* exclude rarely-used Windows headers */
# endif
# include <winsock2.h>           /* socket(), bind(), sendto(), recvfrom() */
# include <ws2tcpip.h>           /* inet_addr(), socklen_t, sockaddr_in    */
# include <windows.h>            /* CreateThread(), Sleep(), HANDLE        */

/* --- Socket ------------------------------------------------------------ */
typedef SOCKET    Platform_Socket_t;
typedef int       Platform_SockLen_t;

# define PLATFORM_INVALID_SOCKET          INVALID_SOCKET
# define PLATFORM_SOCKET_IS_VALID(s)      ((s) != INVALID_SOCKET)
/* Windows select() ignores nfds – pass 0 to avoid signed/unsigned cast */
# define PLATFORM_SELECT_NFDS(s)          (0)
# define PLATFORM_CLOSE_SOCKET(s)         closesocket(s)
/* Windows send/recv buffers are 'char*'; Linux uses 'void*' (compatible) */
# define PLATFORM_SBUF(p)                 ((const char *)(p))
# define PLATFORM_RBUF(p)                 ((char *)(p))
# define PLATFORM_SLEN(n)                 ((int)(n))

/* --- Thread ------------------------------------------------------------ */
typedef HANDLE    Platform_Thread_t;

/* --- Sleep ------------------------------------------------------------- */
static inline void Platform_SleepMs(unsigned int ms)
{
    Sleep((DWORD)ms);
}
static inline void Platform_SleepUs(unsigned int us)
{
    /* Windows minimum resolution is ~1 ms; round up */
    DWORD ms = (DWORD)((us + 999u) / 1000u);
    Sleep(ms != 0u ? ms : 1u);
}

/* --- Network init / deinit (WSAStartup / WSACleanup) ------------------- */
static inline int  Platform_NetworkInit(void)
{
    WSADATA wsaData;
    return (WSAStartup(MAKEWORD(2u, 2u), &wsaData) == 0) ? 0 : -1;
}
static inline void Platform_NetworkDeinit(void)
{
    WSACleanup();
}

/* =========================================================================
 * POSIX – Linux / QNX / macOS
 * ====================================================================== */
#else

# include <sys/socket.h>         /* socket(), bind(), sendto(), recvfrom() */
# include <sys/select.h>         /* select(), fd_set, struct timeval        */
# include <netinet/in.h>         /* sockaddr_in, INADDR_ANY, htons()        */
# include <arpa/inet.h>          /* inet_addr()                             */
# include <netdb.h>              /* getaddrinfo(), freeaddrinfo()           */
# include <pthread.h>            /* pthread_create(), pthread_join()        */
# include <unistd.h>             /* close(), usleep()                       */
# include <errno.h>              /* errno                                   */

/* --- Socket ------------------------------------------------------------ */
typedef int         Platform_Socket_t;
typedef socklen_t   Platform_SockLen_t;

# define PLATFORM_INVALID_SOCKET          (-1)
# define PLATFORM_SOCKET_IS_VALID(s)      ((s) >= 0)
# define PLATFORM_SELECT_NFDS(s)          ((s) + 1)
# define PLATFORM_CLOSE_SOCKET(s)         close(s)
# define PLATFORM_SBUF(p)                 (p)
# define PLATFORM_RBUF(p)                 (p)
# define PLATFORM_SLEN(n)                 ((size_t)(n))

/* --- Thread ------------------------------------------------------------ */
typedef pthread_t   Platform_Thread_t;

/* --- Sleep ------------------------------------------------------------- */
static inline void Platform_SleepMs(unsigned int ms)
{
    usleep((useconds_t)ms * 1000u);
}
static inline void Platform_SleepUs(unsigned int us)
{
    usleep((useconds_t)us);
}

/* --- Network init / deinit (no-op on POSIX) ---------------------------- */
static inline int  Platform_NetworkInit(void)  { return 0; }
static inline void Platform_NetworkDeinit(void) { }

#endif  /* _WIN32 */

/* =========================================================================
 * Common
 * ====================================================================== */
#include <string.h>
#include <stdio.h>

#endif  /* PLATFORM_H */
