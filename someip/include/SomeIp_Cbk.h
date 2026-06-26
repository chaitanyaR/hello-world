/**
 * SomeIp_Cbk.h – SOME/IP Module Callback Declarations
 * AUTOSAR Release 22-11
 * Reference: AUTOSAR_SWS_SOMEIPTransformer R22-11
 *
 * Declares the callback functions that the upper layer (e.g. RTE / SomeIpSd)
 * must implement and that are called by the SomeIp module.
 */

#ifndef SOMEIP_CBK_H
#define SOMEIP_CBK_H

#include "SomeIp_Types.h"

/* ----- AUTOSAR version --------------------------------------------------- */
#define SOMEIP_CBK_AR_RELEASE_MAJOR_VERSION     22u
#define SOMEIP_CBK_AR_RELEASE_MINOR_VERSION     11u
#define SOMEIP_CBK_AR_RELEASE_PATCH_VERSION      0u

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SomeIp_SoConModeChg
 * Notification that the socket connection mode has changed.
 * Called by SoAd; implemented by SomeIp upper-layer glue.
 *
 * @param SoConId   Socket connection identifier.
 * @param Mode      New mode (online / offline / reconnect).
 */
void SomeIp_SoConModeChg(SoAd_SoConIdType SoConId, uint8 Mode);

/**
 * SomeIp_LocalIpAddrAssignmentChg
 * Notification that the local IP address assignment state has changed.
 *
 * @param SoConId  Socket connection identifier.
 * @param State    New IP address assignment state.
 */
void SomeIp_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId, uint8 State);

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_CBK_H */
