/**
 * Std_Types.h – AUTOSAR Standard Types
 * AUTOSAR Release 22-11  |  Std_Types specification SWS_Std_00015
 *
 * Provides the AUTOSAR platform-independent base types used by all BSW
 * modules. In a full AUTOSAR stack this header is supplied by the
 * Microcontroller Abstraction Layer (MCAL) / Platform package.
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

/* ----- Vendor / Module identification ------------------------------------ */
#define STD_VENDOR_ID       0x0000u
#define STD_MODULE_ID       0x0097u

/* ----- AUTOSAR release version ------------------------------------------- */
#define STD_AR_RELEASE_MAJOR_VERSION    22u
#define STD_AR_RELEASE_MINOR_VERSION    11u
#define STD_AR_RELEASE_PATCH_VERSION     0u

/* ----- Software version -------------------------------------------------- */
#define STD_SW_MAJOR_VERSION    1u
#define STD_SW_MINOR_VERSION    0u
#define STD_SW_PATCH_VERSION    0u

/* ----- Boolean ----------------------------------------------------------- */
#ifndef FALSE
#define FALSE   ((boolean)0u)
#endif
#ifndef TRUE
#define TRUE    ((boolean)1u)
#endif

/* ----- Feature switches -------------------------------------------------- */
#define STD_ON  1u
#define STD_OFF 0u
#define STD_HIGH 1u
#define STD_LOW  0u
#define STD_ACTIVE 1u
#define STD_IDLE   0u

/* ----- Base types --------------------------------------------------------
 * Use fixed-width types from <stdint.h> (C99, available on all supported
 * compilers: GCC, MinGW-w64, MSVC 2015+) to avoid the LP64/LLP64 mismatch
 * where 'unsigned long' is 64-bit on Linux x86_64 but 32-bit on Windows.
 * -------------------------------------------------------------------- */
#include <stdint.h>

typedef uint8_t             uint8;
typedef uint16_t            uint16;
typedef uint32_t            uint32;
typedef uint64_t            uint64;

typedef int8_t              sint8;
typedef int16_t             sint16;
typedef int32_t             sint32;
typedef int64_t             sint64;

typedef float               float32;
typedef double              float64;

typedef uint8_t             boolean;

/* ----- Return type ------------------------------------------------------- */
typedef uint8 Std_ReturnType;

#define E_OK        ((Std_ReturnType)0x00u)
#define E_NOT_OK    ((Std_ReturnType)0x01u)

/* ----- Version info ------------------------------------------------------ */
typedef struct {
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

/* ----- Transformer -------------------------------------------------------- */
typedef uint8 Std_TransformerErrorCode;
typedef uint8 Std_TransformerClass;
typedef struct {
    Std_TransformerErrorCode errorCode;
    Std_TransformerClass     transformerClass;
} Std_TransformerError;

#define STD_TRANSFORMER_UNSPECIFIED 0x00u
#define STD_TRANSFORMER_SERIALIZER  0x01u
#define STD_TRANSFORMER_SAFETY      0x02u
#define STD_TRANSFORMER_SECURITY    0x03u
#define STD_TRANSFORMER_CUSTOM      0xFFu

#endif /* STD_TYPES_H */
