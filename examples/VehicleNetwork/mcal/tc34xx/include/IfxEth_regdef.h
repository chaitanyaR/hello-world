/**
 * IfxEth_regdef.h – Aurix TC34xx EQOS Ethernet MAC Register Definitions
 * AUTOSAR Release R22-11
 *
 * Register map for the Synopsys DesignWare Ethernet QoS (EQOS) controller
 * integrated into Infineon Aurix TC34xx (TC344 / TC347 / TC348).
 *
 * The EQOS MAC provides:
 *   - 10 / 100 / 1000 Mbps Ethernet (MII / RMII / RGMII)
 *   - 100BASE-T1 via RMII + TJA1100 PHY (automotive single-pair)
 *   - 8 TX DMA channels, 8 RX DMA channels
 *   - IEEE 1588 hardware time-stamping
 *   - QoS / AVB support (IEEE 802.1Qav)
 *
 * Reference: Aurix TC38x User Manual v1.0, Chapter 23 "Ethernet"
 *            Synopsys EQOS Databook v5.10a
 *
 * NOTE: This header is documentation / compile-time reference only.
 *       On host simulation (ETH_17_GETHM AC_HOST_SIM), registers are not
 *       accessed; all MCAL_REG32_* macros reduce to no-ops.
 */

#ifndef IFXETH_REGDEF_H
#define IFXETH_REGDEF_H

#include "Mcal.h"

/* =========================================================================
 * Helper: derive a register address from the controller base address.
 * ====================================================================== */
#define IFX_ETH_REG(base, offset)   ((uint32)((base) + (uint32)(offset)))

/* =========================================================================
 * MAC Core Registers  (base + 0x000 … 0x0FC)
 * ====================================================================== */

#define IFXETH_MAC_CONFIGURATION_OFFSET           (0x0000u)
#define IFXETH_MAC_EXT_CONFIGURATION_OFFSET       (0x0004u)
#define IFXETH_MAC_PACKET_FILTER_OFFSET           (0x0008u)
#define IFXETH_MAC_WATCHDOG_TIMEOUT_OFFSET        (0x000Cu)
#define IFXETH_MAC_HASH_TABLE_REG0_OFFSET         (0x0010u)
#define IFXETH_MAC_HASH_TABLE_REG1_OFFSET         (0x0014u)
#define IFXETH_MAC_VLAN_TAG_CTRL_OFFSET           (0x0050u)
#define IFXETH_MAC_VLAN_TAG_DATA_OFFSET           (0x0054u)
#define IFXETH_MAC_FLOW_CTRL_OFFSET               (0x0070u)
#define IFXETH_MAC_INTERRUPT_STATUS_OFFSET        (0x00B0u)
#define IFXETH_MAC_INTERRUPT_ENABLE_OFFSET        (0x00B4u)
#define IFXETH_MAC_RX_TX_STATUS_OFFSET            (0x00B8u)
#define IFXETH_MAC_PMT_CTRL_STATUS_OFFSET         (0x00C0u)
#define IFXETH_MAC_HW_FEATURE0_OFFSET             (0x011Cu)
#define IFXETH_MAC_HW_FEATURE1_OFFSET             (0x0120u)
#define IFXETH_MAC_HW_FEATURE2_OFFSET             (0x0124u)
#define IFXETH_MAC_HW_FEATURE3_OFFSET             (0x0128u)
#define IFXETH_MAC_MDIO_ADDRESS_OFFSET            (0x0200u)
#define IFXETH_MAC_MDIO_DATA_OFFSET               (0x0204u)
#define IFXETH_MAC_ADDRESS0_HIGH_OFFSET           (0x0300u)
#define IFXETH_MAC_ADDRESS0_LOW_OFFSET            (0x0304u)
#define IFXETH_MAC_ADDRESS1_HIGH_OFFSET           (0x0308u)
#define IFXETH_MAC_ADDRESS1_LOW_OFFSET            (0x030Cu)

/* MAC_CONFIGURATION register bit fields */
#define IFXETH_MAC_CFG_RE      (1u <<  2u)   /* Receiver Enable */
#define IFXETH_MAC_CFG_TE      (1u <<  1u)   /* Transmitter Enable */
#define IFXETH_MAC_CFG_DM      (1u << 13u)   /* Duplex Mode (1=full) */
#define IFXETH_MAC_CFG_FES     (1u << 14u)   /* Fast Ethernet Speed (1=100M) */
#define IFXETH_MAC_CFG_PS      (1u << 15u)   /* Port Select (1=MII/RMII) */
#define IFXETH_MAC_CFG_JD      (1u << 17u)   /* Jabber Disable */
#define IFXETH_MAC_CFG_WD      (1u << 19u)   /* Watchdog Disable */
#define IFXETH_MAC_CFG_ACS     (1u << 20u)   /* Automatic Pad/CRC Stripping */
#define IFXETH_MAC_CFG_CST     (1u << 21u)   /* CRC Stripping for Type frames */
#define IFXETH_MAC_CFG_S2KP    (1u << 22u)   /* IEEE 802.3as 2K Packets */

/* =========================================================================
 * MTL (MAC Transaction Layer) Registers  (base + 0xC00 … 0xDFC)
 * ====================================================================== */

#define IFXETH_MTL_OPERATION_MODE_OFFSET          (0x0C00u)
#define IFXETH_MTL_INT_STATUS_OFFSET              (0x0C20u)
#define IFXETH_MTL_RXQ_DMA_MAP0_OFFSET            (0x0C30u)
#define IFXETH_MTL_TXQ0_OPERATION_MODE_OFFSET     (0x0D00u)
#define IFXETH_MTL_TXQ0_DEBUG_OFFSET              (0x0D08u)
#define IFXETH_MTL_TXQ0_FLOW_CTRL_OFFSET          (0x0D18u)
#define IFXETH_MTL_RXQ0_OPERATION_MODE_OFFSET     (0x0D30u)
#define IFXETH_MTL_RXQ0_MISSED_PACKET_OFFSET      (0x0D34u)
#define IFXETH_MTL_RXQ0_DEBUG_OFFSET              (0x0D38u)
#define IFXETH_MTL_RXQ0_CTRL_OFFSET               (0x0D3Cu)

/* MTL_OPERATION_MODE bits */
#define IFXETH_MTL_OP_DTXSTS   (1u <<  1u)   /* Drop Tx Status */
#define IFXETH_MTL_OP_RAA      (1u <<  2u)   /* Receive Arbitration Algorithm */
#define IFXETH_MTL_OP_SCHALG   (3u <<  5u)   /* Tx Scheduling (RR=0x0) */
#define IFXETH_MTL_OP_CNTPRST  (1u <<  8u)   /* Counters Preset */
#define IFXETH_MTL_OP_CNTCLR   (1u <<  9u)   /* Counters Reset */

/* =========================================================================
 * DMA Registers  (base + 0x1000 … 0x117C per channel)
 * Eight DMA channels; channel 0 used by default for vehicle network.
 * ====================================================================== */

#define IFXETH_DMA_MODE_OFFSET                    (0x1000u)
#define IFXETH_DMA_SYSBUS_MODE_OFFSET             (0x1004u)
#define IFXETH_DMA_INT_STATUS_OFFSET              (0x1008u)
#define IFXETH_DMA_DBG_STATUS0_OFFSET             (0x100Cu)

/* Per-channel base stride = 0x80 */
#define IFXETH_DMA_CH_STRIDE                      (0x0080u)
#define IFXETH_DMA_CH_BASE(ch)                    ((uint32)(0x1100u + ((uint32)(ch) * IFXETH_DMA_CH_STRIDE)))

#define IFXETH_DMA_CH_CTRL_OFFSET                 (0x00u)
#define IFXETH_DMA_CH_TX_CTRL_OFFSET              (0x04u)
#define IFXETH_DMA_CH_RX_CTRL_OFFSET              (0x08u)
#define IFXETH_DMA_CH_TX_DESC_LIST_ADDR_OFFSET    (0x14u)
#define IFXETH_DMA_CH_RX_DESC_LIST_ADDR_OFFSET    (0x1Cu)
#define IFXETH_DMA_CH_TX_DESC_RING_LEN_OFFSET     (0x2Cu)
#define IFXETH_DMA_CH_RX_DESC_RING_LEN_OFFSET     (0x30u)
#define IFXETH_DMA_CH_INT_ENABLE_OFFSET           (0x34u)
#define IFXETH_DMA_CH_RX_INT_WDOG_TMOUT_OFFSET   (0x38u)
#define IFXETH_DMA_CH_SLOT_FUNC_CTRL_OFFSET       (0x3Cu)
#define IFXETH_DMA_CH_CURR_TX_DESC_OFFSET         (0x44u)
#define IFXETH_DMA_CH_CURR_RX_DESC_OFFSET         (0x4Cu)
#define IFXETH_DMA_CH_CURR_TX_BUFPTR_OFFSET       (0x54u)
#define IFXETH_DMA_CH_CURR_RX_BUFPTR_OFFSET       (0x5Cu)
#define IFXETH_DMA_CH_STATUS_OFFSET               (0x60u)

/* DMA_MODE bits */
#define IFXETH_DMA_MODE_SWR    (1u <<  0u)   /* Software Reset */
#define IFXETH_DMA_MODE_DA     (1u <<  1u)   /* DMA Tx/Rx Arbitration */
#define IFXETH_DMA_MODE_TXPR   (1u << 11u)   /* Transmit has priority */

/* DMA_SYSBUS_MODE bits */
#define IFXETH_DMA_SBM_EAME   (1u << 11u)   /* Enhanced Address Mode Enable */
#define IFXETH_DMA_SBM_BLEN4  (1u <<  1u)   /* AXI burst length 4 */
#define IFXETH_DMA_SBM_BLEN8  (1u <<  2u)   /* AXI burst length 8 */
#define IFXETH_DMA_SBM_BLEN16 (1u <<  3u)   /* AXI burst length 16 */

/* =========================================================================
 * DMA Descriptor layout (enhanced descriptor format, 4x 32-bit words)
 * Used for both TX and RX ring buffers allocated in DSPR/LMU.
 * ====================================================================== */

typedef struct
{
    volatile uint32 Des0;   /* TX: Buffer1 address / RX: Buffer1 address */
    volatile uint32 Des1;   /* TX: Buffer2 address / RX: Buffer2 address */
    volatile uint32 Des2;   /* TX: Control (BUF1V, IOC, LD, FD, len) / RX: buf length */
    volatile uint32 Des3;   /* TX: OWN + FD/LD + len / RX: OWN + status  */
} IfxEth_DmaDesc_t;

/* TX Descriptor Des3 bits */
#define IFXETH_TX_DES3_OWN    ((uint32)1u << 31u)  /* DMA owns descriptor */
#define IFXETH_TX_DES3_FD     ((uint32)1u << 29u)  /* First Descriptor */
#define IFXETH_TX_DES3_LD     ((uint32)1u << 28u)  /* Last Descriptor */
#define IFXETH_TX_DES3_CIC    ((uint32)3u << 16u)  /* Checksum Insertion (IP+TCP/UDP) */

/* RX Descriptor Des3 bits */
#define IFXETH_RX_DES3_OWN    ((uint32)1u << 31u)  /* DMA owns descriptor */
#define IFXETH_RX_DES3_IOC    ((uint32)1u << 30u)  /* Interrupt on Completion */
#define IFXETH_RX_DES3_PL     ((uint32)0x7FFFu)    /* Packet Length [14:0] */

/* =========================================================================
 * PHY Management (MDIO) – TJA1100 100BASE-T1 PHY
 * Aurix TC34xx drives MDIO via the MAC_MDIO_ADDRESS register.
 * ====================================================================== */

#define IFXETH_MDIO_PHYADDR_SHIFT  (21u)
#define IFXETH_MDIO_REGADDR_SHIFT  (16u)
#define IFXETH_MDIO_CR_SHIFT       (8u)    /* CSR Clock Range */
#define IFXETH_MDIO_CR_DIV42       (1u)    /* HCLK/42 → ~2.38 MHz MDC */
#define IFXETH_MDIO_WRITE          (1u <<  2u)
#define IFXETH_MDIO_READ           (3u <<  2u)
#define IFXETH_MDIO_GB             (1u <<  0u)   /* Busy bit */

/* TJA1100 PHY register addresses */
#define IFXETH_TJA1100_BASIC_CTRL_REG   (0x00u)
#define IFXETH_TJA1100_BASIC_STATUS_REG (0x01u)
#define IFXETH_TJA1100_PHY_ID1_REG      (0x02u)
#define IFXETH_TJA1100_PHY_ID2_REG      (0x03u)
#define IFXETH_TJA1100_EXTENDED_CTRL_REG (0x11u)
#define IFXETH_TJA1100_CONFIG_REG1      (0x12u)
#define IFXETH_TJA1100_CONFIG_REG2      (0x13u)

/* TJA1100 Basic Control: master mode + auto-neg disabled (100BASE-T1) */
#define IFXETH_TJA1100_MASTER_MODE    (1u << 11u)
#define IFXETH_TJA1100_LOOPBACK       (1u << 14u)

#endif  /* IFXETH_REGDEF_H */
