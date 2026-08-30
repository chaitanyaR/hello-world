/**
 * Mcal.h – Aurix TC34xx MCAL Compiler Abstraction
 * AUTOSAR Release R22-11
 *
 * TriCore-specific compiler intrinsics, memory-section qualifiers,
 * interrupt-controller macros, and MMIO register-access helpers for the
 * Infineon Aurix TC34xx (TC344 / TC347 / TC348) micro-controller family.
 *
 * Host simulation: when ETH_17_GETH_MAC_HOST_SIM is defined, all
 * hardware-specific constructs reduce to no-ops so the module compiles
 * on Linux / Windows without a TriCore toolchain.
 *
 * Reference: Aurix TC3x Architecture Reference Manual v2.1
 *            AUTOSAR SWS_MCAL R22-11
 */

#ifndef MCAL_H
#define MCAL_H

#include "Std_Types.h"

/* =========================================================================
 * Compiler abstraction – TriCore production toolchains vs. host GCC/MSVC
 * ====================================================================== */

#if defined(__TASKING__) || defined(__DCC__)

    /* TriCore memory section qualifiers */
#   define MCAL_DSPR_DATA   __at(0x70000000u)  /* Data Scratch-Pad RAM        */
#   define MCAL_PSPR_CODE   __at(0x70100000u)  /* Program Scratch-Pad RAM     */
#   define MCAL_LMU_DATA    __at(0x90000000u)  /* Local Memory Unit           */
#   define MCAL_CACHED      /* default: PFLASH access through cache */

    /* Context sync fences (for CSA-based context save on RTOS switch) */
#   define MCAL_SAVE_CONTEXT()    __dsync()
#   define MCAL_RESTORE_CONTEXT() __isync()

    /* Core identifier (multi-core TC34xx) – reads CPU_ID CSFR */
#   define MCAL_CORE_ID()   ((uint8)__mfcr(0xFE1Cu))

#else  /* Host: GCC / MinGW / MSVC */

#   define MCAL_DSPR_DATA
#   define MCAL_PSPR_CODE
#   define MCAL_LMU_DATA
#   define MCAL_CACHED
#   define MCAL_SAVE_CONTEXT()    do {} while (0)
#   define MCAL_RESTORE_CONTEXT() do {} while (0)
#   define MCAL_CORE_ID()         ((uint8)0u)

#endif

/* =========================================================================
 * SRC – Service Request Controller (interrupt enable/disable)
 * On real TC34xx each peripheral has a dedicated SRC register.
 * ====================================================================== */

#ifdef ETH_17_GETH_MAC_HOST_SIM
#   define MCAL_SRC_ENABLE(srcReg)    do {} while (0)
#   define MCAL_SRC_DISABLE(srcReg)   do {} while (0)
#   define MCAL_SRC_CLEAR(srcReg)     do {} while (0)
#   define MCAL_ISR_PRIORITY(p)
#else
    /* SRC register: bit 10 = SRE (enable), bit 12 = CLRR (clear request) */
#   define MCAL_SRC_ENABLE(srcReg)    ((srcReg) |=  (uint32)0x00000400u)
#   define MCAL_SRC_DISABLE(srcReg)   ((srcReg) &= ~(uint32)0x00000400u)
#   define MCAL_SRC_CLEAR(srcReg)     ((srcReg)  =  (uint32)0x00001000u)
#   define MCAL_ISR_PRIORITY(p)       __attribute__((interrupt_priority(p)))
#endif

/* =========================================================================
 * 32-bit MMIO register access helpers
 * Volatile access prevents compiler reordering across register writes.
 * ====================================================================== */

#ifndef ETH_17_GETH_MAC_HOST_SIM
    typedef volatile uint32 Mcal_Reg32_t;
#   define MCAL_REG32_READ(addr)          (*((Mcal_Reg32_t *)(addr)))
#   define MCAL_REG32_WRITE(addr, val)    (*((Mcal_Reg32_t *)(addr)) = (uint32)(val))
#   define MCAL_REG32_SETBITS(addr, mask) (*((Mcal_Reg32_t *)(addr)) |= (uint32)(mask))
#   define MCAL_REG32_CLRBITS(addr, mask) (*((Mcal_Reg32_t *)(addr)) &= ~(uint32)(mask))
#else
#   define MCAL_REG32_READ(addr)          (0u)
#   define MCAL_REG32_WRITE(addr, val)    do {} while (0)
#   define MCAL_REG32_SETBITS(addr, mask) do {} while (0)
#   define MCAL_REG32_CLRBITS(addr, mask) do {} while (0)
#endif

/* =========================================================================
 * TC34xx Peripheral base addresses
 * Source: Aurix TC38x User Manual v1.0, Section 23 – Ethernet
 * ====================================================================== */

#define MCAL_TC34XX_ETH0_BASE  ((uint32)0xF001D000u)  /* ETH0 EQOS MAC base */
#define MCAL_TC34XX_ETH1_BASE  ((uint32)0xF001E000u)  /* ETH1 (TC389 only)  */
#define MCAL_TC34XX_ETH0_CLC   ((uint32)0xF001D800u)  /* Clock Control Reg  */

/* =========================================================================
 * Vendor / module version constants (AUTOSAR GetVersionInfo)
 * ====================================================================== */

#define MCAL_VENDOR_ID          ((uint16)17u)   /* Infineon Technologies AG */
#define MCAL_MODULE_ID_ETH      ((uint16)88u)
#define MCAL_MODULE_ID_ETHIF    ((uint16)65u)
#define MCAL_MODULE_ID_PORT     ((uint16)124u)
#define MCAL_MODULE_ID_MCU      ((uint16)101u)

#define MCAL_SW_MAJOR_VERSION   (1u)
#define MCAL_SW_MINOR_VERSION   (0u)
#define MCAL_SW_PATCH_VERSION   (0u)

#endif  /* MCAL_H */
