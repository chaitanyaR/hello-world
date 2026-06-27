/**
 * NodeTransport.c – UDP Transport Binding for Vehicle Network Nodes
 * AUTOSAR Release R22-11 (Host Simulation Layer)
 *
 * Implements the SoAd / EthIf abstraction on UDP sockets.
 * Portable across Linux/QNX (POSIX) and Windows (Winsock2) via Platform.h.
 *
 *  Send path:
 *    SomeIpSd_Transmit(frame, len)
 *      └─► sendto() to each peer port on 127.0.0.1
 *          (simulates Ethernet switch multicast flooding)
 *
 *  Receive path (background thread):
 *    recvfrom() ──► SomeIp_Deserialize()
 *                    ├─► if SD frame: parse entries → SD callbacks
 *                    └─► SomeIpSd_RxIndication()   (SD state machine)
 *
 * SD entry parsing reads the raw 16-byte entry layout exactly as written
 * by SomeIpSd_SendServiceEntry() and SomeIpSd_SendEventGroupEntry().
 *
 * Thread safety: all module-static state is written once during Init and
 * then read-only from both the main thread and the receive thread.
 * The `volatile` running flag is the only shared mutable state.
 *
 * Platform support:
 *   Linux / QNX  – POSIX sockets + pthreads
 *   Windows 10+  – Winsock2 + CreateThread (Win32)
 */

#include "NodeTransport.h"
#include "Platform.h"       /* must precede all AUTOSAR headers on Windows  */
#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "SomeIp_Types.h"
#include "ComStack_Types.h"

/* Forward declaration: SomeIp_SD.c declares this via extern – no AUTOSAR header
 * exists for BSW integration points; prototype provided here for -Wmissing-prototypes. */
Std_ReturnType SomeIpSd_Transmit(const uint8 *BufPtr, uint32 Length);

/* =========================================================================
 * Module-static state (one instance per process)
 * ====================================================================== */
static Platform_Socket_t  Transport_Sock      = PLATFORM_INVALID_SOCKET;
static struct sockaddr_in Transport_Peers[NODE_TRANSPORT_MAX_PEERS];
static uint8              Transport_PeerCount = 0u;
static volatile int       Transport_Running   = 0;
static Platform_Thread_t  Transport_Thread;
static NodeSdCallbacks    Transport_Cbs;

/* =========================================================================
 * Big-endian read helpers (mirrors SomeIp_SD.c internal helpers)
 * ====================================================================== */

static uint16 ReadU16Be(const uint8 *p)
{
    return (uint16)(((uint16)p[0] << 8u) | (uint16)p[1]);
}

static uint32 ReadU32Be(const uint8 *p)
{
    return ((uint32)p[0] << 24u)
         | ((uint32)p[1] << 16u)
         | ((uint32)p[2] <<  8u)
         |  (uint32)p[3];
}

/* =========================================================================
 * SD entry parser
 * Parses one 16-byte SD entry and dispatches to the registered callback.
 * Layout mirrors SomeIpSd_SendServiceEntry / SomeIpSd_SendEventGroupEntry.
 * ====================================================================== */

static void ParseOneSdEntry(const uint8 *entry)
{
    uint8                   entryType  = entry[0];
    SomeIp_ServiceIdType    svcId      = ReadU16Be(&entry[4]);
    SomeIp_InstanceIdType   instId     = ReadU16Be(&entry[6]);
    uint8                   majorVer   = entry[8];
    uint32                  ttl        = ((uint32)entry[9]  << 16u)
                                       | ((uint32)entry[10] <<  8u)
                                       |  (uint32)entry[11];

    if (entryType == (uint8)SOMEIPSD_ENTRY_OFFER_SERVICE)
    {
        uint32 minorVer = ReadU32Be(&entry[12]);
        if (Transport_Cbs.OnOfferService != NULL)
        {
            Transport_Cbs.OnOfferService(svcId, instId, majorVer, minorVer, ttl);
        }
    }
    else if (entryType == (uint8)SOMEIPSD_ENTRY_FIND_SERVICE)
    {
        uint32 minorVer = ReadU32Be(&entry[12]);
        if (Transport_Cbs.OnFindService != NULL)
        {
            Transport_Cbs.OnFindService(svcId, instId, majorVer, minorVer);
        }
    }
    else if (entryType == (uint8)SOMEIPSD_ENTRY_SUBSCRIBE_EVENTGROUP)
    {
        SomeIp_EventGroupIdType egId = ReadU16Be(&entry[14]);
        if (Transport_Cbs.OnSubscribeEventgroup != NULL)
        {
            Transport_Cbs.OnSubscribeEventgroup(svcId, instId, egId, majorVer, ttl);
        }
    }
    else if (entryType == (uint8)SOMEIPSD_ENTRY_SUBSCRIBE_EVENTGROUP_ACK)
    {
        SomeIp_EventGroupIdType egId = ReadU16Be(&entry[14]);
        if (Transport_Cbs.OnSubscribeEventgroupAck != NULL)
        {
            Transport_Cbs.OnSubscribeEventgroupAck(svcId, instId, egId, majorVer, ttl);
        }
    }
    /* else: unknown entry type – silently ignored per AUTOSAR spec */
}

/* =========================================================================
 * SD frame parser
 * Parses the SD payload (after the SOME/IP header has been stripped by
 * SomeIp_Deserialize) and dispatches each entry.
 *
 * SD payload layout (PRS_SOMEIPSD):
 *   [0]     Flags (1 byte)
 *   [1-3]   Reserved (3 bytes)
 *   [4-7]   Entries Array Length (BE uint32)
 *   [8 ...]  Entries (16 bytes each)
 *   [8+N..] Options Array Length + Options (not parsed here)
 * ====================================================================== */

static void ParseSdPayload(const PduInfoType *payload)
{
    uint32 offset;
    uint32 entriesLen;

    if ((payload->SduDataPtr == NULL) || (payload->SduLength < 8u))
    {
        return;
    }

    /* Skip Flags(1) + Reserved(3) */
    offset = 4u;

    entriesLen = ReadU32Be(&payload->SduDataPtr[offset]);
    offset    += 4u;

    while ((entriesLen >= 16u) && ((offset + 16u) <= payload->SduLength))
    {
        ParseOneSdEntry(&payload->SduDataPtr[offset]);
        offset     += 16u;
        entriesLen -= 16u;
    }
}

/* =========================================================================
 * Raw frame receive handler
 * Called by the receive thread for every incoming UDP packet.
 * ====================================================================== */

static void OnRawFrameReceived(const uint8 *data, uint32 length)
{
    SomeIp_HeaderType header;
    PduInfoType       payload;
    PduInfoType       pduInfo;

    (void)memset(&header,  0, sizeof(header));
    (void)memset(&payload, 0, sizeof(payload));

    /* Deserialise the SOME/IP header; payload points zero-copy into data[] */
    if (SomeIp_Deserialize(data, length, &header, &payload) == E_OK)
    {
        /* If SD frame, parse entries and dispatch to app callbacks */
        if ((header.ServiceId == SOMEIP_SD_SERVICE_ID) &&
            (header.MethodId  == SOMEIP_SD_METHOD_ID))
        {
            ParseSdPayload(&payload);
        }
    }

    /* Also pass to the SD state machine (SWS_SD_00012) */
    pduInfo.SduDataPtr  = (uint8 *)data;  /* PRQA S 0311 – const cast for AUTOSAR API */
    pduInfo.MetaDataPtr = NULL;
    pduInfo.SduLength   = (PduLengthType)length;
    SomeIpSd_RxIndication(SOMEIPSD_RX_PDU_ID, &pduInfo);
}

/* =========================================================================
 * Receive loop (common logic – called from platform thread entry below)
 * Uses select() with a 100 ms timeout so it can check Transport_Running.
 * ====================================================================== */

static void DoReceiveLoop(void)
{
    uint8                rxBuf[NODE_TRANSPORT_MTU];
    struct sockaddr_in   sender;
    Platform_SockLen_t   senderLen;
    int                  rxBytes;
    fd_set               readFds;
    struct timeval       timeout;
    int                  selResult;

    while (Transport_Running != 0)
    {
        FD_ZERO(&readFds);
        FD_SET(Transport_Sock, &readFds);
        timeout.tv_sec  = 0;
        timeout.tv_usec = 100000;  /* 100 ms */

        selResult = select(PLATFORM_SELECT_NFDS(Transport_Sock),
                           &readFds, NULL, NULL, &timeout);
        if (selResult <= 0)
        {
            continue;  /* timeout or signal – re-check running flag */
        }

        senderLen = (Platform_SockLen_t)sizeof(sender);
        rxBytes   = (int)recvfrom(Transport_Sock,
                                   PLATFORM_RBUF(rxBuf), (int)sizeof(rxBuf),
                                   0,
                                   (struct sockaddr *)&sender, &senderLen);
        if (rxBytes > 0)
        {
            OnRawFrameReceived(rxBuf, (uint32)rxBytes);
        }
    }
}

/* =========================================================================
 * Platform thread entry points
 * Windows requires DWORD WINAPI; POSIX requires void*.
 * Both delegate to DoReceiveLoop().
 * ====================================================================== */

#ifdef _WIN32
static DWORD WINAPI ReceiveThread(LPVOID arg)
{
    (void)arg;
    DoReceiveLoop();
    return 0u;
}

static int Plat_ThreadCreate(Platform_Thread_t *t)
{
    *t = CreateThread(NULL, 0u, ReceiveThread, NULL, 0u, NULL);
    return (*t != NULL) ? 0 : -1;
}

static void Plat_ThreadJoin(Platform_Thread_t t)
{
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}

#else  /* POSIX */

static void *ReceiveThread(void *arg)
{
    (void)arg;
    DoReceiveLoop();
    return NULL;
}

static int Plat_ThreadCreate(Platform_Thread_t *t)
{
    return pthread_create(t, NULL, ReceiveThread, NULL);
}

static void Plat_ThreadJoin(Platform_Thread_t t)
{
    pthread_join(t, NULL);
}

#endif  /* _WIN32 */

/* =========================================================================
 * Public API
 * ====================================================================== */

Std_ReturnType NodeTransport_Init(uint16        localPort,
                                   const uint16 *peerPorts,
                                   uint8         peerCount)
{
    struct sockaddr_in localAddr;
    int                reuseOpt = 1;
    uint8              i;
    uint8              count;

    /* Winsock2: must call WSAStartup before any socket operation (no-op on POSIX) */
    if (Platform_NetworkInit() != 0)
    {
        (void)fprintf(stderr, "[Transport] WSAStartup failed\n");
        return E_NOT_OK;
    }

    Transport_Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (!PLATFORM_SOCKET_IS_VALID(Transport_Sock))
    {
        perror("[Transport] socket()");
        return E_NOT_OK;
    }

    (void)setsockopt(Transport_Sock, SOL_SOCKET, SO_REUSEADDR,
                     (const char *)&reuseOpt, (int)sizeof(reuseOpt));

    (void)memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family      = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port        = htons(localPort);

    if (bind(Transport_Sock,
             (const struct sockaddr *)&localAddr,
             sizeof(localAddr)) != 0)
    {
        perror("[Transport] bind()");
        (void)PLATFORM_CLOSE_SOCKET(Transport_Sock);
        Transport_Sock = PLATFORM_INVALID_SOCKET;
        return E_NOT_OK;
    }

    /* Build peer address table */
    count = (peerCount > NODE_TRANSPORT_MAX_PEERS)
                ? NODE_TRANSPORT_MAX_PEERS
                : peerCount;
    Transport_PeerCount = count;

    for (i = 0u; i < count; i++)
    {
        (void)memset(&Transport_Peers[i], 0, sizeof(Transport_Peers[i]));
        Transport_Peers[i].sin_family      = AF_INET;
        Transport_Peers[i].sin_addr.s_addr = inet_addr("127.0.0.1");
        Transport_Peers[i].sin_port        = htons(peerPorts[i]);
    }

    (void)memset(&Transport_Cbs, 0, sizeof(Transport_Cbs));

    printf("[Transport] bound to UDP port %u  (%u peer(s))\n",
           (unsigned)localPort, (unsigned)count);
    return E_OK;
}

void NodeTransport_RegisterSdCallbacks(const NodeSdCallbacks *callbacks)
{
    if (callbacks != NULL)
    {
        Transport_Cbs = *callbacks;
    }
}

Std_ReturnType NodeTransport_StartReceive(void)
{
    Transport_Running = 1;
    if (Plat_ThreadCreate(&Transport_Thread) != 0)
    {
        perror("[Transport] thread create failed");
        Transport_Running = 0;
        return E_NOT_OK;
    }
    return E_OK;
}

void NodeTransport_Deinit(void)
{
    Transport_Running = 0;
    Plat_ThreadJoin(Transport_Thread);

    if (PLATFORM_SOCKET_IS_VALID(Transport_Sock))
    {
        (void)PLATFORM_CLOSE_SOCKET(Transport_Sock);
        Transport_Sock = PLATFORM_INVALID_SOCKET;
    }

    Platform_NetworkDeinit();  /* WSACleanup on Windows, no-op on POSIX */
}

/* =========================================================================
 * SomeIpSd_Transmit – called by SomeIp_SD.c (SoAd integration point)
 *
 * Sends the serialised SD frame to every peer node's SD port.
 * This simulates the Ethernet switch flooding an SD multicast frame to
 * all connected ports (PRS_SOMEIPSD_00001: multicast group 239.192.255.251).
 * ====================================================================== */

Std_ReturnType SomeIpSd_Transmit(const uint8 *BufPtr, uint32 Length)
{
    uint8 i;
    int   sent;

    if (!PLATFORM_SOCKET_IS_VALID(Transport_Sock) || (BufPtr == NULL))
    {
        return E_NOT_OK;
    }

    for (i = 0u; i < Transport_PeerCount; i++)
    {
        sent = (int)sendto(Transport_Sock,
                           PLATFORM_SBUF(BufPtr), (int)Length,
                           0,
                           (const struct sockaddr *)&Transport_Peers[i],
                           (int)sizeof(Transport_Peers[i]));
        if (sent < 0)
        {
            /* Log but continue – other peers may still be reachable */
            perror("[Transport] sendto()");
        }
    }
    return E_OK;
}
