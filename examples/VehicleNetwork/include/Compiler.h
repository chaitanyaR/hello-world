/**
 * Compiler.h – AUTOSAR Compiler Abstraction
 * AUTOSAR Release R22-11  |  AUTOSAR_SWS_CompilerAbstraction
 *
 * Provides compiler-independent keywords for memory classes, pointer
 * qualifiers, and function qualifiers as mandated by the AUTOSAR
 * Compiler Abstraction specification.
 */

#ifndef COMPILER_H
#define COMPILER_H

/* -------------------------------------------------------------------------
 * Compiler vendor / version detection
 * ---------------------------------------------------------------------- */
#if defined(_MSC_VER)
#  define COMPILER_VENDOR_ID  0x0062u  /* Microsoft MSVC */
#elif defined(__GNUC__)
#  define COMPILER_VENDOR_ID  0x0011u  /* GCC / MinGW-w64 / Clang */
#elif defined(__ICCARM__)
#  define COMPILER_VENDOR_ID  0x0051u  /* IAR */
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#  define COMPILER_VENDOR_ID  0x002Eu  /* Keil/ARMCC */
#else
#  define COMPILER_VENDOR_ID  0x0000u  /* Unknown */
#endif

/* -------------------------------------------------------------------------
 * FUNC(rettype, memclass)
 * Used for function declarations / definitions.
 * memclass is typically left empty or mapped to a linker section.
 * ---------------------------------------------------------------------- */
#define FUNC(rettype, memclass)   rettype

/* -------------------------------------------------------------------------
 * FUNC_P2CONST(rettype, ptrclass, memclass)
 * Function returning a pointer to const data.
 * ---------------------------------------------------------------------- */
#define FUNC_P2CONST(rettype, ptrclass, memclass)   const rettype *

/* -------------------------------------------------------------------------
 * FUNC_P2VAR(rettype, ptrclass, memclass)
 * Function returning a pointer to variable data.
 * ---------------------------------------------------------------------- */
#define FUNC_P2VAR(rettype, ptrclass, memclass)   rettype *

/* -------------------------------------------------------------------------
 * P2VAR(ptrtype, memclass, ptrclass)
 * Pointer to variable data.
 * ---------------------------------------------------------------------- */
#define P2VAR(ptrtype, memclass, ptrclass)   ptrtype *

/* -------------------------------------------------------------------------
 * P2CONST(ptrtype, memclass, ptrclass)
 * Pointer to constant data.
 * ---------------------------------------------------------------------- */
#define P2CONST(ptrtype, memclass, ptrclass)   const ptrtype *

/* -------------------------------------------------------------------------
 * CONSTP2VAR(ptrtype, memclass, ptrclass)
 * Constant pointer to variable data.
 * ---------------------------------------------------------------------- */
#define CONSTP2VAR(ptrtype, memclass, ptrclass)   ptrtype * const

/* -------------------------------------------------------------------------
 * CONSTP2CONST(ptrtype, memclass, ptrclass)
 * Constant pointer to constant data.
 * ---------------------------------------------------------------------- */
#define CONSTP2CONST(ptrtype, memclass, ptrclass)   const ptrtype * const

/* -------------------------------------------------------------------------
 * P2FUNC(rettype, ptrclass, fctname)
 * Pointer to function.
 * ---------------------------------------------------------------------- */
#define P2FUNC(rettype, ptrclass, fctname)   rettype (*fctname)

/* -------------------------------------------------------------------------
 * CONST(consttype, memclass)
 * Constant data declaration.
 * ---------------------------------------------------------------------- */
#define CONST(consttype, memclass)   const consttype

/* -------------------------------------------------------------------------
 * VAR(vartype, memclass)
 * Variable data declaration.
 * ---------------------------------------------------------------------- */
#define VAR(vartype, memclass)   vartype

/* -------------------------------------------------------------------------
 * Memory class specifiers (host build: all map to nothing)
 * On embedded targets, replace with __attribute__((section("..."))) as
 * needed by the linker script.
 * ---------------------------------------------------------------------- */
#define AUTOMATIC          /* automatic (stack) storage */
#define TYPEDEF            /* used inside typedef declarations */

/* -------------------------------------------------------------------------
 * NULL_PTR
 * AUTOSAR-defined null pointer constant.
 * ---------------------------------------------------------------------- */
#ifndef NULL_PTR
#  ifdef __cplusplus
#    define NULL_PTR  nullptr
#  else
#    define NULL_PTR  ((void *)0)
#  endif
#endif

/* -------------------------------------------------------------------------
 * INLINE keyword
 * ---------------------------------------------------------------------- */
#if defined(__GNUC__) || defined(__clang__)
#  define INLINE   __inline__
#else
#  define INLINE   inline
#endif

/* -------------------------------------------------------------------------
 * LOCAL_INLINE – module-internal inline function
 * ---------------------------------------------------------------------- */
#define LOCAL_INLINE   static INLINE

#endif /* COMPILER_H */
