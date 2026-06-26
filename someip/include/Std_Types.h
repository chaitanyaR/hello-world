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

/* ----- Base types -------------------------------------------------------- */
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned long       uint32;
typedef unsigned long long  uint64;

typedef signed char         sint8;
typedef signed short        sint16;
typedef signed long         sint32;
typedef signed long long    sint64;

typedef float               float32;
typedef double              float64;

typedef unsigned char       boolean;

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
