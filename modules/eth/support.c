#include <linux/bitmap.h>
#include <linux/bpf.h>
#include <linux/bpf_trace.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/etherdevice.h>
#include <linux/genalloc.h>
#include <linux/gpio/consumer.h>
#include <linux/if_bridge.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irqreturn.h>
#include <linux/kernel.h>
#include <linux/kmemleak.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/net_tstamp.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/phy/phy.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/stat.h>
#include <linux/sys_soc.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <net/devlink.h>
#include <net/page_pool.h>
#include <net/pkt_cls.h>
#include <net/pkt_sched.h>
#include <net/switchdev.h>
#include "cpsw.h"
#include "cpsw_ale.h"
#include "cpsw_priv.h"
#include "cpsw_sl.h"
#include "cpsw_switchdev.h"
#include "cpts.h"
#include "davinci_cpdma.h"

#define CTRL_MAC_LO_REG(offset, id) ((offset) + 0x8 * (id))
#define CTRL_MAC_HI_REG(offset, id) ((offset) + 0x8 * (id) + 0x4)

#define BITMASK(bits) (BIT(bits) - 1)

#define ALE_VERSION_MAJOR(rev, mask) (((rev) >> 8) & (mask))
#define ALE_VERSION_MINOR(rev) (rev & 0xff)
#define ALE_VERSION_1R3 0x0103
#define ALE_VERSION_1R4 0x0104

/* ALE Registers */
#define ALE_IDVER 0x00
#define ALE_STATUS 0x04
#define ALE_CONTROL 0x08
#define ALE_PRESCALE 0x10
#define ALE_AGING_TIMER 0x14
#define ALE_UNKNOWNVLAN 0x18
#define ALE_TABLE_CONTROL 0x20
#define ALE_TABLE 0x34
#define ALE_PORTCTL 0x40

/* ALE NetCP NU switch specific Registers */
#define ALE_UNKNOWNVLAN_MEMBER 0x90
#define ALE_UNKNOWNVLAN_UNREG_MCAST_FLOOD 0x94
#define ALE_UNKNOWNVLAN_REG_MCAST_FLOOD 0x98
#define ALE_UNKNOWNVLAN_FORCE_UNTAG_EGRESS 0x9C
#define ALE_VLAN_MASK_MUX(reg) (0xc0 + (0x4 * (reg)))

#define AM65_CPSW_ALE_THREAD_DEF_REG 0x134

/* ALE_AGING_TIMER */
#define ALE_AGING_TIMER_MASK GENMASK(23, 0)

#define ALE_RATE_LIMIT_MIN_PPS 1000
#define ALE_TABLE_WRITE BIT(31)

#define ALE_TYPE_FREE 0
#define ALE_TYPE_ADDR 1
#define ALE_TYPE_VLAN 2
#define ALE_TYPE_VLAN_ADDR 3

#define ALE_UCAST_PERSISTANT 0
#define ALE_UCAST_UNTOUCHED 1
#define ALE_UCAST_OUI 2
#define ALE_UCAST_TOUCHED 3

#define ALE_TABLE_SIZE_MULTIPLIER 1024
#define ALE_STATUS_SIZE_MASK 0x1f

#define ALE_FLD_ALLOWED BIT(0)
#define ALE_FLD_SIZE_PORT_MASK_BITS BIT(1)
#define ALE_FLD_SIZE_PORT_NUM_BITS BIT(2)

#define ALE_ENTRY_FLD(id, start, bits) \
    [id] = {                           \
        .start_bit = start,            \
        .num_bits = bits,              \
        .flags = ALE_FLD_ALLOWED,      \
    }

#define ALE_ENTRY_FLD_DYN_MSK_SIZE(id, start)                   \
    [id] = {                                                    \
        .start_bit = start,                                     \
        .num_bits = 0,                                          \
        .flags = ALE_FLD_ALLOWED | ALE_FLD_SIZE_PORT_MASK_BITS, \
    }

#define DEFINE_ALE_FIELD(name, start, bits)                             \
    static inline int cpsw_ale_get_##name(u32* ale_entry) {             \
        return cpsw_ale_get_field(ale_entry, start, bits);              \
    }                                                                   \
    static inline void cpsw_ale_set_##name(u32* ale_entry, u32 value) { \
        cpsw_ale_set_field(ale_entry, start, bits, value);              \
    }

#define DEFINE_ALE_FIELD1(name, start)                                \
    static inline int cpsw_ale_get_##name(u32* ale_entry, u32 bits) { \
        return cpsw_ale_get_field(ale_entry, start, bits);            \
    }                                                                 \
    static inline void cpsw_ale_set_##name(u32* ale_entry, u32 value, \
                                           u32 bits) {                \
        cpsw_ale_set_field(ale_entry, start, bits, value);            \
    }
#define CPSW_STAT(m)                                   \
    CPSW_STATS, sizeof_field(struct cpsw_hw_stats, m), \
        offsetof(struct cpsw_hw_stats, m)
#define CPDMA_RX_STAT(m)                                      \
    CPDMA_RX_STATS, sizeof_field(struct cpdma_chan_stats, m), \
        offsetof(struct cpdma_chan_stats, m)
#define CPDMA_TX_STAT(m)                                      \
    CPDMA_TX_STATS, sizeof_field(struct cpdma_chan_stats, m), \
        offsetof(struct cpdma_chan_stats, m)
#define CPSW_STATS_COMMON_LEN ARRAY_SIZE(cpsw_gstrings_stats)
#define CPSW_STATS_CH_LEN ARRAY_SIZE(cpsw_gstrings_ch_stats)
#define CPTS_N_ETX_TS 4
#define CPSW_SL_REG_NOTUSED U16_MAX
#define CPSW_SL_SOFT_RESET_BIT BIT(0)

#define CPSW_SL_STATUS_PN_IDLE BIT(31)
#define CPSW_SL_AM65_STATUS_PN_E_IDLE BIT(30)
#define CPSW_SL_AM65_STATUS_PN_P_IDLE BIT(29)
#define CPSW_SL_AM65_STATUS_PN_TX_IDLE BIT(28)

#define CPSW_SL_STATUS_IDLE_MASK_BASE (CPSW_SL_STATUS_PN_IDLE)

#define CPSW_SL_STATUS_IDLE_MASK_K3                                  \
    (CPSW_SL_STATUS_IDLE_MASK_BASE | CPSW_SL_AM65_STATUS_PN_E_IDLE | \
     CPSW_SL_AM65_STATUS_PN_P_IDLE | CPSW_SL_AM65_STATUS_PN_TX_IDLE)

#define CPSW_SL_CTL_FUNC_BASE                                                 \
    (CPSW_SL_CTL_FULLDUPLEX | CPSW_SL_CTL_LOOPBACK | CPSW_SL_CTL_RX_FLOW_EN | \
     CPSW_SL_CTL_TX_FLOW_EN | CPSW_SL_CTL_GMII_EN | CPSW_SL_CTL_TX_PACE |     \
     CPSW_SL_CTL_GIG | CPSW_SL_CTL_CMD_IDLE | CPSW_SL_CTL_IFCTL_A |           \
     CPSW_SL_CTL_IFCTL_B | CPSW_SL_CTL_GIG_FORCE | CPSW_SL_CTL_EXT_EN |       \
     CPSW_SL_CTL_RX_CEF_EN | CPSW_SL_CTL_RX_CSF_EN | CPSW_SL_CTL_RX_CMF_EN)
/* DMA Registers */
#define CPDMA_TXIDVER 0x00
#define CPDMA_TXCONTROL 0x04
#define CPDMA_TXTEARDOWN 0x08
#define CPDMA_RXIDVER 0x10
#define CPDMA_RXCONTROL 0x14
#define CPDMA_SOFTRESET 0x1c
#define CPDMA_RXTEARDOWN 0x18
#define CPDMA_TX_PRI0_RATE 0x30
#define CPDMA_TXINTSTATRAW 0x80
#define CPDMA_TXINTSTATMASKED 0x84
#define CPDMA_TXINTMASKSET 0x88
#define CPDMA_TXINTMASKCLEAR 0x8c
#define CPDMA_MACINVECTOR 0x90
#define CPDMA_MACEOIVECTOR 0x94
#define CPDMA_RXINTSTATRAW 0xa0
#define CPDMA_RXINTSTATMASKED 0xa4
#define CPDMA_RXINTMASKSET 0xa8
#define CPDMA_RXINTMASKCLEAR 0xac
#define CPDMA_DMAINTSTATRAW 0xb0
#define CPDMA_DMAINTSTATMASKED 0xb4
#define CPDMA_DMAINTMASKSET 0xb8
#define CPDMA_DMAINTMASKCLEAR 0xbc
#define CPDMA_DMAINT_HOSTERR BIT(1)

/* the following exist only if has_ext_regs is set */
#define CPDMA_DMACONTROL 0x20
#define CPDMA_DMASTATUS 0x24
#define CPDMA_RXBUFFOFS 0x28
#define CPDMA_EM_CONTROL 0x2c

/* Descriptor mode bits */
#define CPDMA_DESC_SOP BIT(31)
#define CPDMA_DESC_EOP BIT(30)
#define CPDMA_DESC_OWNER BIT(29)
#define CPDMA_DESC_EOQ BIT(28)
#define CPDMA_DESC_TD_COMPLETE BIT(27)
#define CPDMA_DESC_PASS_CRC BIT(26)
#define CPDMA_DESC_TO_PORT_EN BIT(20)
#define CPDMA_TO_PORT_SHIFT 16
#define CPDMA_DESC_PORT_MASK (BIT(18) | BIT(17) | BIT(16))
#define CPDMA_DESC_CRC_LEN 4

#define CPDMA_TEARDOWN_VALUE 0xfffffffc

#define CPDMA_MAX_RLIM_CNT 16384

static int davinci_emac_3517_get_macid(struct device* dev,
                                       u16 offset,
                                       int slave,
                                       u8* mac_addr) {
    u32 macid_lsb;
    u32 macid_msb;
    struct regmap* syscon;

    syscon = syscon_regmap_lookup_by_phandle(dev->of_node, "syscon");
    if (IS_ERR(syscon)) {
        if (PTR_ERR(syscon) == -ENODEV)
            return 0;
        return PTR_ERR(syscon);
    }

    regmap_read(syscon, CTRL_MAC_LO_REG(offset, slave), &macid_lsb);
    regmap_read(syscon, CTRL_MAC_HI_REG(offset, slave), &macid_msb);

    mac_addr[0] = (macid_msb >> 16) & 0xff;
    mac_addr[1] = (macid_msb >> 8) & 0xff;
    mac_addr[2] = macid_msb & 0xff;
    mac_addr[3] = (macid_lsb >> 16) & 0xff;
    mac_addr[4] = (macid_lsb >> 8) & 0xff;
    mac_addr[5] = macid_lsb & 0xff;

    return 0;
}

static int cpsw_am33xx_cm_get_macid(struct device* dev,
                                    u16 offset,
                                    int slave,
                                    u8* mac_addr) {
    u32 macid_lo;
    u32 macid_hi;
    struct regmap* syscon;

    syscon = syscon_regmap_lookup_by_phandle(dev->of_node, "syscon");
    if (IS_ERR(syscon)) {
        if (PTR_ERR(syscon) == -ENODEV)
            return 0;
        return PTR_ERR(syscon);
    }

    regmap_read(syscon, CTRL_MAC_LO_REG(offset, slave), &macid_lo);
    regmap_read(syscon, CTRL_MAC_HI_REG(offset, slave), &macid_hi);

    mac_addr[5] = (macid_lo >> 8) & 0xff;
    mac_addr[4] = macid_lo & 0xff;
    mac_addr[3] = (macid_hi >> 24) & 0xff;
    mac_addr[2] = (macid_hi >> 16) & 0xff;
    mac_addr[1] = (macid_hi >> 8) & 0xff;
    mac_addr[0] = macid_hi & 0xff;

    return 0;
}

int ti_cm_get_macid(struct device* dev, int slave, u8* mac_addr) {
    if (of_machine_is_compatible("ti,dm8148"))
        return cpsw_am33xx_cm_get_macid(dev, 0x630, slave, mac_addr);

    if (of_machine_is_compatible("ti,am33xx"))
        return cpsw_am33xx_cm_get_macid(dev, 0x630, slave, mac_addr);

    if (of_device_is_compatible(dev->of_node, "ti,am3517-emac"))
        return davinci_emac_3517_get_macid(dev, 0x110, slave, mac_addr);

    if (of_device_is_compatible(dev->of_node, "ti,dm816-emac"))
        return cpsw_am33xx_cm_get_macid(dev, 0x30, slave, mac_addr);

    if (of_machine_is_compatible("ti,am43"))
        return cpsw_am33xx_cm_get_macid(dev, 0x630, slave, mac_addr);

    if (of_machine_is_compatible("ti,dra7"))
        return davinci_emac_3517_get_macid(dev, 0x514, slave, mac_addr);

    dev_info(dev, "incompatible machine/device type for reading mac address\n");
    return -ENOENT;
}

/**
 * struct ale_entry_fld - The ALE tbl entry field description
 * @start_bit: field start bit
 * @num_bits: field bit length
 * @flags: field flags
 */
struct ale_entry_fld {
    u8 start_bit;
    u8 num_bits;
    u8 flags;
};

enum {
    CPSW_ALE_F_STATUS_REG = BIT(0),   /* Status register present */
    CPSW_ALE_F_HW_AUTOAGING = BIT(1), /* HW auto aging */

    CPSW_ALE_F_COUNT
};

/**
 * struct cpsw_ale_dev_id - The ALE version/SoC specific configuration
 * @dev_id: ALE version/SoC id
 * @features: features supported by ALE
 * @tbl_entries: number of ALE entries
 * @major_ver_mask: mask of ALE Major Version Value in ALE_IDVER reg.
 * @nu_switch_ale: NU Switch ALE
 * @vlan_entry_tbl: ALE vlan entry fields description tbl
 */
struct cpsw_ale_dev_id {
    const char* dev_id;
    u32 features;
    u32 tbl_entries;
    u32 major_ver_mask;
    bool nu_switch_ale;
    const struct ale_entry_fld* vlan_entry_tbl;
};

static inline int cpsw_ale_get_field(u32* ale_entry, u32 start, u32 bits) {
    int idx;

    idx = start / 32;
    start -= idx * 32;
    idx = 2 - idx; /* flip */
    return (ale_entry[idx] >> start) & BITMASK(bits);
}

static inline void cpsw_ale_set_field(u32* ale_entry,
                                      u32 start,
                                      u32 bits,
                                      u32 value) {
    int idx;

    value &= BITMASK(bits);
    idx = start / 32;
    start -= idx * 32;
    idx = 2 - idx; /* flip */
    ale_entry[idx] &= ~(BITMASK(bits) << start);
    ale_entry[idx] |= (value << start);
}

enum {
    ALE_ENT_VID_MEMBER_LIST = 0,
    ALE_ENT_VID_UNREG_MCAST_MSK,
    ALE_ENT_VID_REG_MCAST_MSK,
    ALE_ENT_VID_FORCE_UNTAGGED_MSK,
    ALE_ENT_VID_UNREG_MCAST_IDX,
    ALE_ENT_VID_REG_MCAST_IDX,
    ALE_ENT_VID_LAST,
};

/* dm814x, am3/am4/am5, k2hk */
static const struct ale_entry_fld vlan_entry_cpsw[ALE_ENT_VID_LAST] = {
    ALE_ENTRY_FLD(ALE_ENT_VID_MEMBER_LIST, 0, 3),
    ALE_ENTRY_FLD(ALE_ENT_VID_UNREG_MCAST_MSK, 8, 3),
    ALE_ENTRY_FLD(ALE_ENT_VID_REG_MCAST_MSK, 16, 3),
    ALE_ENTRY_FLD(ALE_ENT_VID_FORCE_UNTAGGED_MSK, 24, 3),
};

/* k2e/k2l, k3 am65/j721e cpsw2g  */
static const struct ale_entry_fld vlan_entry_nu[ALE_ENT_VID_LAST] = {
    ALE_ENTRY_FLD_DYN_MSK_SIZE(ALE_ENT_VID_MEMBER_LIST, 0),
    ALE_ENTRY_FLD(ALE_ENT_VID_UNREG_MCAST_IDX, 20, 3),
    ALE_ENTRY_FLD_DYN_MSK_SIZE(ALE_ENT_VID_FORCE_UNTAGGED_MSK, 24),
    ALE_ENTRY_FLD(ALE_ENT_VID_REG_MCAST_IDX, 44, 3),
};

/* K3 j721e/j7200 cpsw9g/5g, am64x cpsw3g  */
static const struct ale_entry_fld vlan_entry_k3_cpswxg[] = {
    ALE_ENTRY_FLD_DYN_MSK_SIZE(ALE_ENT_VID_MEMBER_LIST, 0),
    ALE_ENTRY_FLD_DYN_MSK_SIZE(ALE_ENT_VID_UNREG_MCAST_MSK, 12),
    ALE_ENTRY_FLD_DYN_MSK_SIZE(ALE_ENT_VID_FORCE_UNTAGGED_MSK, 24),
    ALE_ENTRY_FLD_DYN_MSK_SIZE(ALE_ENT_VID_REG_MCAST_MSK, 36),
};

DEFINE_ALE_FIELD(entry_type, 60, 2)
DEFINE_ALE_FIELD(vlan_id, 48, 12)
DEFINE_ALE_FIELD(mcast_state, 62, 2)
DEFINE_ALE_FIELD1(port_mask, 66)
DEFINE_ALE_FIELD(super, 65, 1)
DEFINE_ALE_FIELD(ucast_type, 62, 2)
DEFINE_ALE_FIELD1(port_num, 66)
DEFINE_ALE_FIELD(blocked, 65, 1)
DEFINE_ALE_FIELD(secure, 64, 1)
DEFINE_ALE_FIELD(mcast, 40, 1)

#define NU_VLAN_UNREG_MCAST_IDX 1

static int cpsw_ale_entry_get_fld(struct cpsw_ale* ale,
                                  u32* ale_entry,
                                  const struct ale_entry_fld* entry_tbl,
                                  int fld_id) {
    const struct ale_entry_fld* entry_fld;
    u32 bits;

    if (!ale || !ale_entry)
        return -EINVAL;

    entry_fld = &entry_tbl[fld_id];
    if (!(entry_fld->flags & ALE_FLD_ALLOWED)) {
        dev_err(ale->params.dev, "get: wrong ale fld id %d\n", fld_id);
        return -ENOENT;
    }

    bits = entry_fld->num_bits;
    if (entry_fld->flags & ALE_FLD_SIZE_PORT_MASK_BITS)
        bits = ale->port_mask_bits;

    return cpsw_ale_get_field(ale_entry, entry_fld->start_bit, bits);
}

static void cpsw_ale_entry_set_fld(struct cpsw_ale* ale,
                                   u32* ale_entry,
                                   const struct ale_entry_fld* entry_tbl,
                                   int fld_id,
                                   u32 value) {
    const struct ale_entry_fld* entry_fld;
    u32 bits;

    if (!ale || !ale_entry)
        return;

    entry_fld = &entry_tbl[fld_id];
    if (!(entry_fld->flags & ALE_FLD_ALLOWED)) {
        dev_err(ale->params.dev, "set: wrong ale fld id %d\n", fld_id);
        return;
    }

    bits = entry_fld->num_bits;
    if (entry_fld->flags & ALE_FLD_SIZE_PORT_MASK_BITS)
        bits = ale->port_mask_bits;

    cpsw_ale_set_field(ale_entry, entry_fld->start_bit, bits, value);
}

static int cpsw_ale_vlan_get_fld(struct cpsw_ale* ale,
                                 u32* ale_entry,
                                 int fld_id) {
    return cpsw_ale_entry_get_fld(ale, ale_entry, ale->vlan_entry_tbl, fld_id);
}

static void cpsw_ale_vlan_set_fld(struct cpsw_ale* ale,
                                  u32* ale_entry,
                                  int fld_id,
                                  u32 value) {
    cpsw_ale_entry_set_fld(ale, ale_entry, ale->vlan_entry_tbl, fld_id, value);
}

/* The MAC address field in the ALE entry cannot be macroized as above */
static inline void cpsw_ale_get_addr(u32* ale_entry, u8* addr) {
    int i;

    for (i = 0; i < 6; i++)
        addr[i] = cpsw_ale_get_field(ale_entry, 40 - 8 * i, 8);
}

static inline void cpsw_ale_set_addr(u32* ale_entry, const u8* addr) {
    int i;

    for (i = 0; i < 6; i++)
        cpsw_ale_set_field(ale_entry, 40 - 8 * i, 8, addr[i]);
}

static int cpsw_ale_read(struct cpsw_ale* ale, int idx, u32* ale_entry) {
    int i;

    WARN_ON(idx > ale->params.ale_entries);

    writel_relaxed(idx, ale->params.ale_regs + ALE_TABLE_CONTROL);

    for (i = 0; i < ALE_ENTRY_WORDS; i++)
        ale_entry[i] = readl_relaxed(ale->params.ale_regs + ALE_TABLE + 4 * i);

    return idx;
}

static int cpsw_ale_write(struct cpsw_ale* ale, int idx, u32* ale_entry) {
    int i;

    WARN_ON(idx > ale->params.ale_entries);

    for (i = 0; i < ALE_ENTRY_WORDS; i++)
        writel_relaxed(ale_entry[i], ale->params.ale_regs + ALE_TABLE + 4 * i);

    writel_relaxed(idx | ALE_TABLE_WRITE,
                   ale->params.ale_regs + ALE_TABLE_CONTROL);

    return idx;
}

static int cpsw_ale_match_addr(struct cpsw_ale* ale, const u8* addr, u16 vid) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int type, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        u8 entry_addr[6];

        cpsw_ale_read(ale, idx, ale_entry);
        type = cpsw_ale_get_entry_type(ale_entry);
        if (type != ALE_TYPE_ADDR && type != ALE_TYPE_VLAN_ADDR)
            continue;
        if (cpsw_ale_get_vlan_id(ale_entry) != vid)
            continue;
        cpsw_ale_get_addr(ale_entry, entry_addr);
        if (ether_addr_equal(entry_addr, addr))
            return idx;
    }
    return -ENOENT;
}

static int cpsw_ale_match_vlan(struct cpsw_ale* ale, u16 vid) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int type, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        cpsw_ale_read(ale, idx, ale_entry);
        type = cpsw_ale_get_entry_type(ale_entry);
        if (type != ALE_TYPE_VLAN)
            continue;
        if (cpsw_ale_get_vlan_id(ale_entry) == vid)
            return idx;
    }
    return -ENOENT;
}

static int cpsw_ale_match_free(struct cpsw_ale* ale) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int type, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        cpsw_ale_read(ale, idx, ale_entry);
        type = cpsw_ale_get_entry_type(ale_entry);
        if (type == ALE_TYPE_FREE)
            return idx;
    }
    return -ENOENT;
}

static int cpsw_ale_find_ageable(struct cpsw_ale* ale) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int type, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        cpsw_ale_read(ale, idx, ale_entry);
        type = cpsw_ale_get_entry_type(ale_entry);
        if (type != ALE_TYPE_ADDR && type != ALE_TYPE_VLAN_ADDR)
            continue;
        if (cpsw_ale_get_mcast(ale_entry))
            continue;
        type = cpsw_ale_get_ucast_type(ale_entry);
        if (type != ALE_UCAST_PERSISTANT && type != ALE_UCAST_OUI)
            return idx;
    }
    return -ENOENT;
}

static void cpsw_ale_flush_mcast(struct cpsw_ale* ale,
                                 u32* ale_entry,
                                 int port_mask) {
    int mask;

    mask = cpsw_ale_get_port_mask(ale_entry, ale->port_mask_bits);
    if ((mask & port_mask) == 0)
        return; /* ports dont intersect, not interested */
    mask &= ~port_mask;

    /* free if only remaining port is host port */
    if (mask)
        cpsw_ale_set_port_mask(ale_entry, mask, ale->port_mask_bits);
    else
        cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_FREE);
}

int cpsw_ale_flush_multicast(struct cpsw_ale* ale, int port_mask, int vid) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int ret, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        cpsw_ale_read(ale, idx, ale_entry);
        ret = cpsw_ale_get_entry_type(ale_entry);
        if (ret != ALE_TYPE_ADDR && ret != ALE_TYPE_VLAN_ADDR)
            continue;

        /* if vid passed is -1 then remove all multicast entry from
         * the table irrespective of vlan id, if a valid vlan id is
         * passed then remove only multicast added to that vlan id.
         * if vlan id doesn't match then move on to next entry.
         */
        if (vid != -1 && cpsw_ale_get_vlan_id(ale_entry) != vid)
            continue;

        if (cpsw_ale_get_mcast(ale_entry)) {
            u8 addr[6];

            if (cpsw_ale_get_super(ale_entry))
                continue;

            cpsw_ale_get_addr(ale_entry, addr);
            if (!is_broadcast_ether_addr(addr))
                cpsw_ale_flush_mcast(ale, ale_entry, port_mask);
        }

        cpsw_ale_write(ale, idx, ale_entry);
    }
    return 0;
}

static inline void cpsw_ale_set_vlan_entry_type(u32* ale_entry,
                                                int flags,
                                                u16 vid) {
    if (flags & ALE_VLAN) {
        cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_VLAN_ADDR);
        cpsw_ale_set_vlan_id(ale_entry, vid);
    } else {
        cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_ADDR);
    }
}

int cpsw_ale_add_ucast(struct cpsw_ale* ale,
                       const u8* addr,
                       int port,
                       int flags,
                       u16 vid) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int idx;

    cpsw_ale_set_vlan_entry_type(ale_entry, flags, vid);

    cpsw_ale_set_addr(ale_entry, addr);
    cpsw_ale_set_ucast_type(ale_entry, ALE_UCAST_PERSISTANT);
    cpsw_ale_set_secure(ale_entry, (flags & ALE_SECURE) ? 1 : 0);
    cpsw_ale_set_blocked(ale_entry, (flags & ALE_BLOCKED) ? 1 : 0);
    cpsw_ale_set_port_num(ale_entry, port, ale->port_num_bits);

    idx = cpsw_ale_match_addr(ale, addr, (flags & ALE_VLAN) ? vid : 0);
    if (idx < 0)
        idx = cpsw_ale_match_free(ale);
    if (idx < 0)
        idx = cpsw_ale_find_ageable(ale);
    if (idx < 0)
        return -ENOMEM;

    cpsw_ale_write(ale, idx, ale_entry);
    return 0;
}

int cpsw_ale_del_ucast(struct cpsw_ale* ale,
                       const u8* addr,
                       int port,
                       int flags,
                       u16 vid) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int idx;

    idx = cpsw_ale_match_addr(ale, addr, (flags & ALE_VLAN) ? vid : 0);
    if (idx < 0)
        return -ENOENT;

    cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_FREE);
    cpsw_ale_write(ale, idx, ale_entry);
    return 0;
}

int cpsw_ale_add_mcast(struct cpsw_ale* ale,
                       const u8* addr,
                       int port_mask,
                       int flags,
                       u16 vid,
                       int mcast_state) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int idx, mask;

    idx = cpsw_ale_match_addr(ale, addr, (flags & ALE_VLAN) ? vid : 0);
    if (idx >= 0)
        cpsw_ale_read(ale, idx, ale_entry);

    cpsw_ale_set_vlan_entry_type(ale_entry, flags, vid);

    cpsw_ale_set_addr(ale_entry, addr);
    cpsw_ale_set_super(ale_entry, (flags & ALE_SUPER) ? 1 : 0);
    cpsw_ale_set_mcast_state(ale_entry, mcast_state);

    mask = cpsw_ale_get_port_mask(ale_entry, ale->port_mask_bits);
    port_mask |= mask;
    cpsw_ale_set_port_mask(ale_entry, port_mask, ale->port_mask_bits);

    if (idx < 0)
        idx = cpsw_ale_match_free(ale);
    if (idx < 0)
        idx = cpsw_ale_find_ageable(ale);
    if (idx < 0)
        return -ENOMEM;

    cpsw_ale_write(ale, idx, ale_entry);
    return 0;
}

int cpsw_ale_del_mcast(struct cpsw_ale* ale,
                       const u8* addr,
                       int port_mask,
                       int flags,
                       u16 vid) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int mcast_members = 0;
    int idx;

    idx = cpsw_ale_match_addr(ale, addr, (flags & ALE_VLAN) ? vid : 0);
    if (idx < 0)
        return -ENOENT;

    cpsw_ale_read(ale, idx, ale_entry);

    if (port_mask) {
        mcast_members = cpsw_ale_get_port_mask(ale_entry, ale->port_mask_bits);
        mcast_members &= ~port_mask;
    }

    if (mcast_members)
        cpsw_ale_set_port_mask(ale_entry, mcast_members, ale->port_mask_bits);
    else
        cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_FREE);

    cpsw_ale_write(ale, idx, ale_entry);
    return 0;
}

/* ALE NetCP NU switch specific vlan functions */
static void cpsw_ale_set_vlan_mcast(struct cpsw_ale* ale,
                                    u32* ale_entry,
                                    int reg_mcast,
                                    int unreg_mcast) {
    int idx;

    /* Set VLAN registered multicast flood mask */
    idx = cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_REG_MCAST_IDX);
    writel(reg_mcast, ale->params.ale_regs + ALE_VLAN_MASK_MUX(idx));

    /* Set VLAN unregistered multicast flood mask */
    idx = cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_IDX);
    writel(unreg_mcast, ale->params.ale_regs + ALE_VLAN_MASK_MUX(idx));
}

static void cpsw_ale_set_vlan_untag(struct cpsw_ale* ale,
                                    u32* ale_entry,
                                    u16 vid,
                                    int untag_mask) {
    cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_FORCE_UNTAGGED_MSK,
                          untag_mask);
    if (untag_mask & ALE_PORT_HOST)
        bitmap_set(ale->p0_untag_vid_mask, vid, 1);
    else
        bitmap_clear(ale->p0_untag_vid_mask, vid, 1);
}

int cpsw_ale_add_vlan(struct cpsw_ale* ale,
                      u16 vid,
                      int port_mask,
                      int untag,
                      int reg_mcast,
                      int unreg_mcast) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int idx;

    idx = cpsw_ale_match_vlan(ale, vid);
    if (idx >= 0)
        cpsw_ale_read(ale, idx, ale_entry);

    cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_VLAN);
    cpsw_ale_set_vlan_id(ale_entry, vid);
    cpsw_ale_set_vlan_untag(ale, ale_entry, vid, untag);

    if (!ale->params.nu_switch_ale) {
        cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_REG_MCAST_MSK,
                              reg_mcast);
        cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK,
                              unreg_mcast);
    } else {
        cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_IDX,
                              NU_VLAN_UNREG_MCAST_IDX);
        cpsw_ale_set_vlan_mcast(ale, ale_entry, reg_mcast, unreg_mcast);
    }

    cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_MEMBER_LIST, port_mask);

    if (idx < 0)
        idx = cpsw_ale_match_free(ale);
    if (idx < 0)
        idx = cpsw_ale_find_ageable(ale);
    if (idx < 0)
        return -ENOMEM;

    cpsw_ale_write(ale, idx, ale_entry);
    return 0;
}

static void cpsw_ale_vlan_del_modify_int(struct cpsw_ale* ale,
                                         u32* ale_entry,
                                         u16 vid,
                                         int port_mask) {
    int reg_mcast, unreg_mcast;
    int members, untag;

    members = cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_MEMBER_LIST);
    members &= ~port_mask;
    if (!members) {
        cpsw_ale_set_vlan_untag(ale, ale_entry, vid, 0);
        cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_FREE);
        return;
    }

    untag =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_FORCE_UNTAGGED_MSK);
    reg_mcast =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_REG_MCAST_MSK);
    unreg_mcast =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK);
    untag &= members;
    reg_mcast &= members;
    unreg_mcast &= members;

    cpsw_ale_set_vlan_untag(ale, ale_entry, vid, untag);

    if (!ale->params.nu_switch_ale) {
        cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_REG_MCAST_MSK,
                              reg_mcast);
        cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK,
                              unreg_mcast);
    } else {
        cpsw_ale_set_vlan_mcast(ale, ale_entry, reg_mcast, unreg_mcast);
    }
    cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_MEMBER_LIST, members);
}

int cpsw_ale_vlan_del_modify(struct cpsw_ale* ale, u16 vid, int port_mask) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int idx;

    idx = cpsw_ale_match_vlan(ale, vid);
    if (idx < 0)
        return -ENOENT;

    cpsw_ale_read(ale, idx, ale_entry);

    cpsw_ale_vlan_del_modify_int(ale, ale_entry, vid, port_mask);
    cpsw_ale_write(ale, idx, ale_entry);

    return 0;
}

int cpsw_ale_del_vlan(struct cpsw_ale* ale, u16 vid, int port_mask) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int members, idx;

    idx = cpsw_ale_match_vlan(ale, vid);
    if (idx < 0)
        return -ENOENT;

    cpsw_ale_read(ale, idx, ale_entry);

    /* if !port_mask - force remove VLAN (legacy).
     * Check if there are other VLAN members ports
     * if no - remove VLAN.
     * if yes it means same VLAN was added to >1 port in multi port mode, so
     * remove port_mask ports from VLAN ALE entry excluding Host port.
     */
    members = cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_MEMBER_LIST);
    members &= ~port_mask;

    if (!port_mask || !members) {
        /* last port or force remove - remove VLAN */
        cpsw_ale_set_vlan_untag(ale, ale_entry, vid, 0);
        cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_FREE);
    } else {
        port_mask &= ~ALE_PORT_HOST;
        cpsw_ale_vlan_del_modify_int(ale, ale_entry, vid, port_mask);
    }

    cpsw_ale_write(ale, idx, ale_entry);

    return 0;
}

int cpsw_ale_vlan_add_modify(struct cpsw_ale* ale,
                             u16 vid,
                             int port_mask,
                             int untag_mask,
                             int reg_mask,
                             int unreg_mask) {
    u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
    int reg_mcast_members, unreg_mcast_members;
    int vlan_members, untag_members;
    int idx, ret = 0;

    idx = cpsw_ale_match_vlan(ale, vid);
    if (idx >= 0)
        cpsw_ale_read(ale, idx, ale_entry);

    vlan_members =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_MEMBER_LIST);
    reg_mcast_members =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_REG_MCAST_MSK);
    unreg_mcast_members =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK);
    untag_members =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_FORCE_UNTAGGED_MSK);

    vlan_members |= port_mask;
    untag_members = (untag_members & ~port_mask) | untag_mask;
    reg_mcast_members = (reg_mcast_members & ~port_mask) | reg_mask;
    unreg_mcast_members = (unreg_mcast_members & ~port_mask) | unreg_mask;

    ret = cpsw_ale_add_vlan(ale, vid, vlan_members, untag_members,
                            reg_mcast_members, unreg_mcast_members);
    if (ret) {
        dev_err(ale->params.dev, "Unable to add vlan\n");
        return ret;
    }
    dev_dbg(ale->params.dev, "port mask 0x%x untag 0x%x\n", vlan_members,
            untag_mask);

    return ret;
}

void cpsw_ale_set_unreg_mcast(struct cpsw_ale* ale,
                              int unreg_mcast_mask,
                              bool add) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int unreg_members = 0;
    int type, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        cpsw_ale_read(ale, idx, ale_entry);
        type = cpsw_ale_get_entry_type(ale_entry);
        if (type != ALE_TYPE_VLAN)
            continue;

        unreg_members =
            cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK);
        if (add)
            unreg_members |= unreg_mcast_mask;
        else
            unreg_members &= ~unreg_mcast_mask;
        cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK,
                              unreg_members);
        cpsw_ale_write(ale, idx, ale_entry);
    }
}

static void cpsw_ale_vlan_set_unreg_mcast(struct cpsw_ale* ale,
                                          u32* ale_entry,
                                          int allmulti) {
    int unreg_mcast;

    unreg_mcast =
        cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK);
    if (allmulti)
        unreg_mcast |= ALE_PORT_HOST;
    else
        unreg_mcast &= ~ALE_PORT_HOST;

    cpsw_ale_vlan_set_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_MSK,
                          unreg_mcast);
}

static void cpsw_ale_vlan_set_unreg_mcast_idx(struct cpsw_ale* ale,
                                              u32* ale_entry,
                                              int allmulti) {
    int unreg_mcast;
    int idx;

    idx = cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_UNREG_MCAST_IDX);

    unreg_mcast = readl(ale->params.ale_regs + ALE_VLAN_MASK_MUX(idx));

    if (allmulti)
        unreg_mcast |= ALE_PORT_HOST;
    else
        unreg_mcast &= ~ALE_PORT_HOST;

    writel(unreg_mcast, ale->params.ale_regs + ALE_VLAN_MASK_MUX(idx));
}

void cpsw_ale_set_allmulti(struct cpsw_ale* ale, int allmulti, int port) {
    u32 ale_entry[ALE_ENTRY_WORDS];
    int type, idx;

    for (idx = 0; idx < ale->params.ale_entries; idx++) {
        int vlan_members;

        cpsw_ale_read(ale, idx, ale_entry);
        type = cpsw_ale_get_entry_type(ale_entry);
        if (type != ALE_TYPE_VLAN)
            continue;

        vlan_members =
            cpsw_ale_vlan_get_fld(ale, ale_entry, ALE_ENT_VID_MEMBER_LIST);

        if (port != -1 && !(vlan_members & BIT(port)))
            continue;

        if (!ale->params.nu_switch_ale)
            cpsw_ale_vlan_set_unreg_mcast(ale, ale_entry, allmulti);
        else
            cpsw_ale_vlan_set_unreg_mcast_idx(ale, ale_entry, allmulti);

        cpsw_ale_write(ale, idx, ale_entry);
    }
}

struct ale_control_info {
    const char* name;
    int offset, port_offset;
    int shift, port_shift;
    int bits;
};

static struct ale_control_info ale_controls[ALE_NUM_CONTROLS] = {
    [ALE_ENABLE] =
        {
            .name = "enable",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 31,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_CLEAR] =
        {
            .name = "clear",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 30,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_AGEOUT] =
        {
            .name = "ageout",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 29,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_P0_UNI_FLOOD] =
        {
            .name = "port0_unicast_flood",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 8,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_VLAN_NOLEARN] =
        {
            .name = "vlan_nolearn",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 7,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_NO_PORT_VLAN] =
        {
            .name = "no_port_vlan",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 6,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_OUI_DENY] =
        {
            .name = "oui_deny",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 5,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_BYPASS] =
        {
            .name = "bypass",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 4,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_RATE_LIMIT_TX] =
        {
            .name = "rate_limit_tx",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 3,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_VLAN_AWARE] =
        {
            .name = "vlan_aware",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 2,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_AUTH_ENABLE] =
        {
            .name = "auth_enable",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 1,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_RATE_LIMIT] =
        {
            .name = "rate_limit",
            .offset = ALE_CONTROL,
            .port_offset = 0,
            .shift = 0,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_STATE] =
        {
            .name = "port_state",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 0,
            .port_shift = 0,
            .bits = 2,
        },
    [ALE_PORT_DROP_UNTAGGED] =
        {
            .name = "drop_untagged",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 2,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_DROP_UNKNOWN_VLAN] =
        {
            .name = "drop_unknown",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 3,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_NOLEARN] =
        {
            .name = "nolearn",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 4,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_NO_SA_UPDATE] =
        {
            .name = "no_source_update",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 5,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_MACONLY] =
        {
            .name = "mac_only_port_mode",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 11,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_MACONLY_CAF] =
        {
            .name = "mac_only_port_caf",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 13,
            .port_shift = 0,
            .bits = 1,
        },
    [ALE_PORT_MCAST_LIMIT] =
        {
            .name = "mcast_limit",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 16,
            .port_shift = 0,
            .bits = 8,
        },
    [ALE_PORT_BCAST_LIMIT] =
        {
            .name = "bcast_limit",
            .offset = ALE_PORTCTL,
            .port_offset = 4,
            .shift = 24,
            .port_shift = 0,
            .bits = 8,
        },
    [ALE_PORT_UNKNOWN_VLAN_MEMBER] =
        {
            .name = "unknown_vlan_member",
            .offset = ALE_UNKNOWNVLAN,
            .port_offset = 0,
            .shift = 0,
            .port_shift = 0,
            .bits = 6,
        },
    [ALE_PORT_UNKNOWN_MCAST_FLOOD] =
        {
            .name = "unknown_mcast_flood",
            .offset = ALE_UNKNOWNVLAN,
            .port_offset = 0,
            .shift = 8,
            .port_shift = 0,
            .bits = 6,
        },
    [ALE_PORT_UNKNOWN_REG_MCAST_FLOOD] =
        {
            .name = "unknown_reg_flood",
            .offset = ALE_UNKNOWNVLAN,
            .port_offset = 0,
            .shift = 16,
            .port_shift = 0,
            .bits = 6,
        },
    [ALE_PORT_UNTAGGED_EGRESS] =
        {
            .name = "untagged_egress",
            .offset = ALE_UNKNOWNVLAN,
            .port_offset = 0,
            .shift = 24,
            .port_shift = 0,
            .bits = 6,
        },
    [ALE_DEFAULT_THREAD_ID] =
        {
            .name = "default_thread_id",
            .offset = AM65_CPSW_ALE_THREAD_DEF_REG,
            .port_offset = 0,
            .shift = 0,
            .port_shift = 0,
            .bits = 6,
        },
    [ALE_DEFAULT_THREAD_ENABLE] =
        {
            .name = "default_thread_id_enable",
            .offset = AM65_CPSW_ALE_THREAD_DEF_REG,
            .port_offset = 0,
            .shift = 15,
            .port_shift = 0,
            .bits = 1,
        },
};

int cpsw_ale_control_set(struct cpsw_ale* ale,
                         int port,
                         int control,
                         int value) {
    const struct ale_control_info* info;
    int offset, shift;
    u32 tmp, mask;

    if (control < 0 || control >= ARRAY_SIZE(ale_controls))
        return -EINVAL;

    info = &ale_controls[control];
    if (info->port_offset == 0 && info->port_shift == 0)
        port = 0; /* global, port is a dont care */

    if (port < 0 || port >= ale->params.ale_ports)
        return -EINVAL;

    mask = BITMASK(info->bits);
    if (value & ~mask)
        return -EINVAL;

    offset = info->offset + (port * info->port_offset);
    shift = info->shift + (port * info->port_shift);

    tmp = readl_relaxed(ale->params.ale_regs + offset);
    tmp = (tmp & ~(mask << shift)) | (value << shift);
    writel_relaxed(tmp, ale->params.ale_regs + offset);

    return 0;
}

int cpsw_ale_control_get(struct cpsw_ale* ale, int port, int control) {
    const struct ale_control_info* info;
    int offset, shift;
    u32 tmp;

    if (control < 0 || control >= ARRAY_SIZE(ale_controls))
        return -EINVAL;

    info = &ale_controls[control];
    if (info->port_offset == 0 && info->port_shift == 0)
        port = 0; /* global, port is a dont care */

    if (port < 0 || port >= ale->params.ale_ports)
        return -EINVAL;

    offset = info->offset + (port * info->port_offset);
    shift = info->shift + (port * info->port_shift);

    tmp = readl_relaxed(ale->params.ale_regs + offset) >> shift;
    return tmp & BITMASK(info->bits);
}

int cpsw_ale_rx_ratelimit_mc(struct cpsw_ale* ale,
                             int port,
                             unsigned int ratelimit_pps)

{
    int val = ratelimit_pps / ALE_RATE_LIMIT_MIN_PPS;
    u32 remainder = ratelimit_pps % ALE_RATE_LIMIT_MIN_PPS;

    if (ratelimit_pps && !val) {
        dev_err(ale->params.dev, "ALE MC port:%d ratelimit min value 1000pps\n",
                port);
        return -EINVAL;
    }

    if (remainder)
        dev_info(ale->params.dev,
                 "ALE port:%d MC ratelimit set to %dpps (requested %d)\n", port,
                 ratelimit_pps - remainder, ratelimit_pps);

    cpsw_ale_control_set(ale, port, ALE_PORT_MCAST_LIMIT, val);

    dev_dbg(ale->params.dev, "ALE port:%d MC ratelimit set %d\n", port,
            val * ALE_RATE_LIMIT_MIN_PPS);
    return 0;
}

int cpsw_ale_rx_ratelimit_bc(struct cpsw_ale* ale,
                             int port,
                             unsigned int ratelimit_pps)

{
    int val = ratelimit_pps / ALE_RATE_LIMIT_MIN_PPS;
    u32 remainder = ratelimit_pps % ALE_RATE_LIMIT_MIN_PPS;

    if (ratelimit_pps && !val) {
        dev_err(ale->params.dev, "ALE port:%d BC ratelimit min value 1000pps\n",
                port);
        return -EINVAL;
    }

    if (remainder)
        dev_info(ale->params.dev,
                 "ALE port:%d BC ratelimit set to %dpps (requested %d)\n", port,
                 ratelimit_pps - remainder, ratelimit_pps);

    cpsw_ale_control_set(ale, port, ALE_PORT_BCAST_LIMIT, val);

    dev_dbg(ale->params.dev, "ALE port:%d BC ratelimit set %d\n", port,
            val * ALE_RATE_LIMIT_MIN_PPS);
    return 0;
}

static void cpsw_ale_timer(struct timer_list* t) {
    struct cpsw_ale* ale = from_timer(ale, t, timer);

    cpsw_ale_control_set(ale, 0, ALE_AGEOUT, 1);

    if (ale->ageout) {
        ale->timer.expires = jiffies + ale->ageout;
        add_timer(&ale->timer);
    }
}

static void cpsw_ale_hw_aging_timer_start(struct cpsw_ale* ale) {
    u32 aging_timer;

    aging_timer = ale->params.bus_freq / 1000000;
    aging_timer *= ale->params.ale_ageout;

    if (aging_timer & ~ALE_AGING_TIMER_MASK) {
        aging_timer = ALE_AGING_TIMER_MASK;
        dev_warn(ale->params.dev, "ALE aging timer overflow, set to max\n");
    }

    writel(aging_timer, ale->params.ale_regs + ALE_AGING_TIMER);
}

static void cpsw_ale_hw_aging_timer_stop(struct cpsw_ale* ale) {
    writel(0, ale->params.ale_regs + ALE_AGING_TIMER);
}

static void cpsw_ale_aging_start(struct cpsw_ale* ale) {
    if (!ale->params.ale_ageout)
        return;

    if (ale->features & CPSW_ALE_F_HW_AUTOAGING) {
        cpsw_ale_hw_aging_timer_start(ale);
        return;
    }

    timer_setup(&ale->timer, cpsw_ale_timer, 0);
    ale->timer.expires = jiffies + ale->ageout;
    add_timer(&ale->timer);
}

static void cpsw_ale_aging_stop(struct cpsw_ale* ale) {
    if (!ale->params.ale_ageout)
        return;

    if (ale->features & CPSW_ALE_F_HW_AUTOAGING) {
        cpsw_ale_hw_aging_timer_stop(ale);
        return;
    }

    del_timer_sync(&ale->timer);
}

void cpsw_ale_start(struct cpsw_ale* ale) {
    unsigned long ale_prescale;

    /* configure Broadcast and Multicast Rate Limit
     * number_of_packets = (Fclk / ALE_PRESCALE) * port.BCAST/MCAST_LIMIT
     * ALE_PRESCALE width is 19bit and min value 0x10
     * port.BCAST/MCAST_LIMIT is 8bit
     *
     * For multi port configuration support the ALE_PRESCALE is configured to
     * 1ms interval, which allows to configure port.BCAST/MCAST_LIMIT per port
     * and achieve: min number_of_packets = 1000 when port.BCAST/MCAST_LIMIT = 1
     * max number_of_packets = 1000 * 255 = 255000 when port.BCAST/MCAST_LIMIT =
     * 0xFF
     */
    ale_prescale = ale->params.bus_freq / ALE_RATE_LIMIT_MIN_PPS;
    writel((u32)ale_prescale, ale->params.ale_regs + ALE_PRESCALE);

    /* Allow MC/BC rate limiting globally.
     * The actual Rate Limit cfg enabled per-port by port.BCAST/MCAST_LIMIT
     */
    cpsw_ale_control_set(ale, 0, ALE_RATE_LIMIT, 1);

    cpsw_ale_control_set(ale, 0, ALE_ENABLE, 1);
    cpsw_ale_control_set(ale, 0, ALE_CLEAR, 1);

    cpsw_ale_aging_start(ale);
}

void cpsw_ale_stop(struct cpsw_ale* ale) {
    cpsw_ale_aging_stop(ale);
    cpsw_ale_control_set(ale, 0, ALE_CLEAR, 1);
    cpsw_ale_control_set(ale, 0, ALE_ENABLE, 0);
}

static const struct cpsw_ale_dev_id cpsw_ale_id_match[] = {
    {
        /* am3/4/5, dra7. dm814x, 66ak2hk-gbe */
        .dev_id = "cpsw",
        .tbl_entries = 1024,
        .major_ver_mask = 0xff,
        .vlan_entry_tbl = vlan_entry_cpsw,
    },
    {
        /* 66ak2h_xgbe */
        .dev_id = "66ak2h-xgbe",
        .tbl_entries = 2048,
        .major_ver_mask = 0xff,
        .vlan_entry_tbl = vlan_entry_cpsw,
    },
    {
        .dev_id = "66ak2el",
        .features = CPSW_ALE_F_STATUS_REG,
        .major_ver_mask = 0x7,
        .nu_switch_ale = true,
        .vlan_entry_tbl = vlan_entry_nu,
    },
    {
        .dev_id = "66ak2g",
        .features = CPSW_ALE_F_STATUS_REG,
        .tbl_entries = 64,
        .major_ver_mask = 0x7,
        .nu_switch_ale = true,
        .vlan_entry_tbl = vlan_entry_nu,
    },
    {
        .dev_id = "am65x-cpsw2g",
        .features = CPSW_ALE_F_STATUS_REG | CPSW_ALE_F_HW_AUTOAGING,
        .tbl_entries = 64,
        .major_ver_mask = 0x7,
        .nu_switch_ale = true,
        .vlan_entry_tbl = vlan_entry_nu,
    },
    {
        .dev_id = "j721e-cpswxg",
        .features = CPSW_ALE_F_STATUS_REG | CPSW_ALE_F_HW_AUTOAGING,
        .major_ver_mask = 0x7,
        .vlan_entry_tbl = vlan_entry_k3_cpswxg,
    },
    {
        .dev_id = "am64-cpswxg",
        .features = CPSW_ALE_F_STATUS_REG | CPSW_ALE_F_HW_AUTOAGING,
        .major_ver_mask = 0x7,
        .vlan_entry_tbl = vlan_entry_k3_cpswxg,
        .tbl_entries = 512,
    },
    {},
};

static const struct cpsw_ale_dev_id* cpsw_ale_match_id(
    const struct cpsw_ale_dev_id* id,
    const char* dev_id) {
    if (!dev_id)
        return NULL;

    while (id->dev_id) {
        if (strcmp(dev_id, id->dev_id) == 0)
            return id;
        id++;
    }
    return NULL;
}

struct cpsw_ale* cpsw_ale_create(struct cpsw_ale_params* params) {
    const struct cpsw_ale_dev_id* ale_dev_id;
    struct cpsw_ale* ale;
    u32 rev, ale_entries;

    ale_dev_id = cpsw_ale_match_id(cpsw_ale_id_match, params->dev_id);
    if (!ale_dev_id)
        return ERR_PTR(-EINVAL);

    params->ale_entries = ale_dev_id->tbl_entries;
    params->major_ver_mask = ale_dev_id->major_ver_mask;
    params->nu_switch_ale = ale_dev_id->nu_switch_ale;

    ale = devm_kzalloc(params->dev, sizeof(*ale), GFP_KERNEL);
    if (!ale)
        return ERR_PTR(-ENOMEM);

    ale->p0_untag_vid_mask =
        devm_bitmap_zalloc(params->dev, VLAN_N_VID, GFP_KERNEL);
    if (!ale->p0_untag_vid_mask)
        return ERR_PTR(-ENOMEM);

    ale->params = *params;
    ale->ageout = ale->params.ale_ageout * HZ;
    ale->features = ale_dev_id->features;
    ale->vlan_entry_tbl = ale_dev_id->vlan_entry_tbl;

    rev = readl_relaxed(ale->params.ale_regs + ALE_IDVER);
    ale->version = (ALE_VERSION_MAJOR(rev, ale->params.major_ver_mask) << 8) |
                   ALE_VERSION_MINOR(rev);
    dev_info(ale->params.dev, "initialized cpsw ale version %d.%d\n",
             ALE_VERSION_MAJOR(rev, ale->params.major_ver_mask),
             ALE_VERSION_MINOR(rev));

    if (ale->features & CPSW_ALE_F_STATUS_REG && !ale->params.ale_entries) {
        ale_entries = readl_relaxed(ale->params.ale_regs + ALE_STATUS) &
                      ALE_STATUS_SIZE_MASK;
        /* ALE available on newer NetCP switches has introduced
         * a register, ALE_STATUS, to indicate the size of ALE
         * table which shows the size as a multiple of 1024 entries.
         * For these, params.ale_entries will be set to zero. So
         * read the register and update the value of ale_entries.
         * return error if ale_entries is zero in ALE_STATUS.
         */
        if (!ale_entries)
            return ERR_PTR(-EINVAL);

        ale_entries *= ALE_TABLE_SIZE_MULTIPLIER;
        ale->params.ale_entries = ale_entries;
    }
    dev_info(ale->params.dev, "ALE Table size %ld\n", ale->params.ale_entries);

    /* set default bits for existing h/w */
    ale->port_mask_bits = ale->params.ale_ports;
    ale->port_num_bits = order_base_2(ale->params.ale_ports);
    ale->vlan_field_bits = ale->params.ale_ports;

    /* Set defaults override for ALE on NetCP NU switch and for version
     * 1R3
     */
    if (ale->params.nu_switch_ale) {
        /* Separate registers for unknown vlan configuration.
         * Also there are N bits, where N is number of ale
         * ports and shift value should be 0
         */
        ale_controls[ALE_PORT_UNKNOWN_VLAN_MEMBER].bits = ale->params.ale_ports;
        ale_controls[ALE_PORT_UNKNOWN_VLAN_MEMBER].offset =
            ALE_UNKNOWNVLAN_MEMBER;
        ale_controls[ALE_PORT_UNKNOWN_MCAST_FLOOD].bits = ale->params.ale_ports;
        ale_controls[ALE_PORT_UNKNOWN_MCAST_FLOOD].shift = 0;
        ale_controls[ALE_PORT_UNKNOWN_MCAST_FLOOD].offset =
            ALE_UNKNOWNVLAN_UNREG_MCAST_FLOOD;
        ale_controls[ALE_PORT_UNKNOWN_REG_MCAST_FLOOD].bits =
            ale->params.ale_ports;
        ale_controls[ALE_PORT_UNKNOWN_REG_MCAST_FLOOD].shift = 0;
        ale_controls[ALE_PORT_UNKNOWN_REG_MCAST_FLOOD].offset =
            ALE_UNKNOWNVLAN_REG_MCAST_FLOOD;
        ale_controls[ALE_PORT_UNTAGGED_EGRESS].bits = ale->params.ale_ports;
        ale_controls[ALE_PORT_UNTAGGED_EGRESS].shift = 0;
        ale_controls[ALE_PORT_UNTAGGED_EGRESS].offset =
            ALE_UNKNOWNVLAN_FORCE_UNTAG_EGRESS;
    }

    cpsw_ale_control_set(ale, 0, ALE_CLEAR, 1);
    return ale;
}

void cpsw_ale_dump(struct cpsw_ale* ale, u32* data) {
    int i;

    for (i = 0; i < ale->params.ale_entries; i++) {
        cpsw_ale_read(ale, i, data);
        data += ALE_ENTRY_WORDS;
    }
}

u32 cpsw_ale_get_num_entries(struct cpsw_ale* ale) {
    return ale ? ale->params.ale_entries : 0;
}

struct cpsw_hw_stats {
    u32 rxgoodframes;
    u32 rxbroadcastframes;
    u32 rxmulticastframes;
    u32 rxpauseframes;
    u32 rxcrcerrors;
    u32 rxaligncodeerrors;
    u32 rxoversizedframes;
    u32 rxjabberframes;
    u32 rxundersizedframes;
    u32 rxfragments;
    u32 __pad_0[2];
    u32 rxoctets;
    u32 txgoodframes;
    u32 txbroadcastframes;
    u32 txmulticastframes;
    u32 txpauseframes;
    u32 txdeferredframes;
    u32 txcollisionframes;
    u32 txsinglecollframes;
    u32 txmultcollframes;
    u32 txexcessivecollisions;
    u32 txlatecollisions;
    u32 txunderrun;
    u32 txcarriersenseerrors;
    u32 txoctets;
    u32 octetframes64;
    u32 octetframes65t127;
    u32 octetframes128t255;
    u32 octetframes256t511;
    u32 octetframes512t1023;
    u32 octetframes1024tup;
    u32 netoctets;
    u32 rxsofoverruns;
    u32 rxmofoverruns;
    u32 rxdmaoverruns;
};

struct cpsw_stats {
    char stat_string[ETH_GSTRING_LEN];
    int type;
    int sizeof_stat;
    int stat_offset;
};

enum {
    CPSW_STATS,
    CPDMA_RX_STATS,
    CPDMA_TX_STATS,
};

static const struct cpsw_stats cpsw_gstrings_stats[] = {
    {"Good Rx Frames", CPSW_STAT(rxgoodframes)},
    {"Broadcast Rx Frames", CPSW_STAT(rxbroadcastframes)},
    {"Multicast Rx Frames", CPSW_STAT(rxmulticastframes)},
    {"Pause Rx Frames", CPSW_STAT(rxpauseframes)},
    {"Rx CRC Errors", CPSW_STAT(rxcrcerrors)},
    {"Rx Align/Code Errors", CPSW_STAT(rxaligncodeerrors)},
    {"Oversize Rx Frames", CPSW_STAT(rxoversizedframes)},
    {"Rx Jabbers", CPSW_STAT(rxjabberframes)},
    {"Undersize (Short) Rx Frames", CPSW_STAT(rxundersizedframes)},
    {"Rx Fragments", CPSW_STAT(rxfragments)},
    {"Rx Octets", CPSW_STAT(rxoctets)},
    {"Good Tx Frames", CPSW_STAT(txgoodframes)},
    {"Broadcast Tx Frames", CPSW_STAT(txbroadcastframes)},
    {"Multicast Tx Frames", CPSW_STAT(txmulticastframes)},
    {"Pause Tx Frames", CPSW_STAT(txpauseframes)},
    {"Deferred Tx Frames", CPSW_STAT(txdeferredframes)},
    {"Collisions", CPSW_STAT(txcollisionframes)},
    {"Single Collision Tx Frames", CPSW_STAT(txsinglecollframes)},
    {"Multiple Collision Tx Frames", CPSW_STAT(txmultcollframes)},
    {"Excessive Collisions", CPSW_STAT(txexcessivecollisions)},
    {"Late Collisions", CPSW_STAT(txlatecollisions)},
    {"Tx Underrun", CPSW_STAT(txunderrun)},
    {"Carrier Sense Errors", CPSW_STAT(txcarriersenseerrors)},
    {"Tx Octets", CPSW_STAT(txoctets)},
    {"Rx + Tx 64 Octet Frames", CPSW_STAT(octetframes64)},
    {"Rx + Tx 65-127 Octet Frames", CPSW_STAT(octetframes65t127)},
    {"Rx + Tx 128-255 Octet Frames", CPSW_STAT(octetframes128t255)},
    {"Rx + Tx 256-511 Octet Frames", CPSW_STAT(octetframes256t511)},
    {"Rx + Tx 512-1023 Octet Frames", CPSW_STAT(octetframes512t1023)},
    {"Rx + Tx 1024-Up Octet Frames", CPSW_STAT(octetframes1024tup)},
    {"Net Octets", CPSW_STAT(netoctets)},
    {"Rx Start of Frame Overruns", CPSW_STAT(rxsofoverruns)},
    {"Rx Middle of Frame Overruns", CPSW_STAT(rxmofoverruns)},
    {"Rx DMA Overruns", CPSW_STAT(rxdmaoverruns)},
};

static const struct cpsw_stats cpsw_gstrings_ch_stats[] = {
    {"head_enqueue", CPDMA_RX_STAT(head_enqueue)},
    {"tail_enqueue", CPDMA_RX_STAT(tail_enqueue)},
    {"pad_enqueue", CPDMA_RX_STAT(pad_enqueue)},
    {"misqueued", CPDMA_RX_STAT(misqueued)},
    {"desc_alloc_fail", CPDMA_RX_STAT(desc_alloc_fail)},
    {"pad_alloc_fail", CPDMA_RX_STAT(pad_alloc_fail)},
    {"runt_receive_buf", CPDMA_RX_STAT(runt_receive_buff)},
    {"runt_transmit_buf", CPDMA_RX_STAT(runt_transmit_buff)},
    {"empty_dequeue", CPDMA_RX_STAT(empty_dequeue)},
    {"busy_dequeue", CPDMA_RX_STAT(busy_dequeue)},
    {"good_dequeue", CPDMA_RX_STAT(good_dequeue)},
    {"requeue", CPDMA_RX_STAT(requeue)},
    {"teardown_dequeue", CPDMA_RX_STAT(teardown_dequeue)},
};

u32 cpsw_get_msglevel(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);

    return priv->msg_enable;
}

void cpsw_set_msglevel(struct net_device* ndev, u32 value) {
    struct cpsw_priv* priv = netdev_priv(ndev);

    priv->msg_enable = value;
}

int cpsw_get_coalesce(struct net_device* ndev,
                      struct ethtool_coalesce* coal,
                      struct kernel_ethtool_coalesce* kernel_coal,
                      struct netlink_ext_ack* extack) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

    coal->rx_coalesce_usecs = cpsw->coal_intvl;
    return 0;
}

int cpsw_set_coalesce(struct cpsw_priv* priv ,
                      struct ethtool_coalesce* coal,
                      struct kernel_ethtool_coalesce* kernel_coal,
                      struct netlink_ext_ack* extack) {
    u32 int_ctrl;
    u32 num_interrupts = 0;
    u32 prescale = 0;
    u32 addnl_dvdr = 1;
    u32 coal_intvl = 0;
    struct cpsw_common* cpsw = priv->cpsw;

    coal_intvl = coal->rx_coalesce_usecs;

    int_ctrl = readl(&cpsw->wr_regs->int_control);
    prescale = cpsw->bus_freq_mhz * 4;

    if (!coal->rx_coalesce_usecs) {
        int_ctrl &= ~(CPSW_INTPRESCALE_MASK | CPSW_INTPACEEN);
        goto update_return;
    }

    if (coal_intvl < CPSW_CMINTMIN_INTVL)
        coal_intvl = CPSW_CMINTMIN_INTVL;

    if (coal_intvl > CPSW_CMINTMAX_INTVL) {
        /* Interrupt pacer works with 4us Pulse, we can
         * throttle further by dilating the 4us pulse.
         */
        addnl_dvdr = CPSW_INTPRESCALE_MASK / prescale;

        if (addnl_dvdr > 1) {
            prescale *= addnl_dvdr;
            if (coal_intvl > (CPSW_CMINTMAX_INTVL * addnl_dvdr))
                coal_intvl = (CPSW_CMINTMAX_INTVL * addnl_dvdr);
        } else {
            addnl_dvdr = 1;
            coal_intvl = CPSW_CMINTMAX_INTVL;
        }
    }

    num_interrupts = (1000 * addnl_dvdr) / coal_intvl;
    writel(num_interrupts, &cpsw->wr_regs->rx_imax);
    writel(num_interrupts, &cpsw->wr_regs->tx_imax);

    int_ctrl |= CPSW_INTPACEEN;
    int_ctrl &= (~CPSW_INTPRESCALE_MASK);
    int_ctrl |= (prescale & CPSW_INTPRESCALE_MASK);

update_return:
    writel(int_ctrl, &cpsw->wr_regs->int_control);

    cpsw_notice(priv, timer, "Set coalesce to %d usecs.\n", coal_intvl);
    cpsw->coal_intvl = coal_intvl;

    return 0;
}

int cpsw_get_sset_count(struct net_device* ndev, int sset) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

    switch (sset) {
        case ETH_SS_STATS:
            return (CPSW_STATS_COMMON_LEN +
                    (cpsw->rx_ch_num + cpsw->tx_ch_num) * CPSW_STATS_CH_LEN);
        default:
            return -EOPNOTSUPP;
    }
}

static void cpsw_add_ch_strings(u8** p, int ch_num, int rx_dir) {
    int ch_stats_len;
    int line;
    int i;

    ch_stats_len = CPSW_STATS_CH_LEN * ch_num;
    for (i = 0; i < ch_stats_len; i++) {
        line = i % CPSW_STATS_CH_LEN;
        snprintf(*p, ETH_GSTRING_LEN, "%s DMA chan %ld: %s",
                 rx_dir ? "Rx" : "Tx", (long)(i / CPSW_STATS_CH_LEN),
                 cpsw_gstrings_ch_stats[line].stat_string);
        *p += ETH_GSTRING_LEN;
    }
}

void cpsw_get_strings(struct net_device* ndev, u32 stringset, u8* data) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);
    u8* p = data;
    int i;

    switch (stringset) {
        case ETH_SS_STATS:
            for (i = 0; i < CPSW_STATS_COMMON_LEN; i++) {
                memcpy(p, cpsw_gstrings_stats[i].stat_string, ETH_GSTRING_LEN);
                p += ETH_GSTRING_LEN;
            }

            cpsw_add_ch_strings(&p, cpsw->rx_ch_num, 1);
            cpsw_add_ch_strings(&p, cpsw->tx_ch_num, 0);
            break;
    }
}

void cpsw_get_ethtool_stats(struct net_device* ndev,
                            struct ethtool_stats* stats,
                            u64* data) {
    u8* p;
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);
    struct cpdma_chan_stats ch_stats;
    int i, l, ch;

    /* Collect Davinci CPDMA stats for Rx and Tx Channel */
    for (l = 0; l < CPSW_STATS_COMMON_LEN; l++)
        data[l] = readl(cpsw->hw_stats + cpsw_gstrings_stats[l].stat_offset);

    for (ch = 0; ch < cpsw->rx_ch_num; ch++) {
        cpdma_chan_get_stats(cpsw->rxv[ch].ch, &ch_stats);
        for (i = 0; i < CPSW_STATS_CH_LEN; i++, l++) {
            p = (u8*)&ch_stats + cpsw_gstrings_ch_stats[i].stat_offset;
            data[l] = *(u32*)p;
        }
    }

    for (ch = 0; ch < cpsw->tx_ch_num; ch++) {
        cpdma_chan_get_stats(cpsw->txv[ch].ch, &ch_stats);
        for (i = 0; i < CPSW_STATS_CH_LEN; i++, l++) {
            p = (u8*)&ch_stats + cpsw_gstrings_ch_stats[i].stat_offset;
            data[l] = *(u32*)p;
        }
    }
}

void cpsw_get_pauseparam(struct net_device* ndev,
                         struct ethtool_pauseparam* pause) {
    struct cpsw_priv* priv = netdev_priv(ndev);

    pause->autoneg = AUTONEG_DISABLE;
    pause->rx_pause = priv->rx_pause ? true : false;
    pause->tx_pause = priv->tx_pause ? true : false;
}

void cpsw_get_wol(struct net_device* ndev, struct ethtool_wolinfo* wol) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    wol->supported = 0;
    wol->wolopts = 0;

    if (cpsw->slaves[slave_no].phy)
        phy_ethtool_get_wol(cpsw->slaves[slave_no].phy, wol);
}

int cpsw_set_wol(struct net_device* ndev, struct ethtool_wolinfo* wol) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    if (cpsw->slaves[slave_no].phy)
        return phy_ethtool_set_wol(cpsw->slaves[slave_no].phy, wol);
    else
        return -EOPNOTSUPP;
}

int cpsw_get_regs_len(struct net_device* ndev) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

    return cpsw_ale_get_num_entries(cpsw->ale) * ALE_ENTRY_WORDS * sizeof(u32);
}

void cpsw_get_regs(struct net_device* ndev,
                   struct ethtool_regs* regs,
                   void* p) {
    u32* reg = p;
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

    /* update CPSW IP version */
    regs->version = cpsw->version;

    cpsw_ale_dump(cpsw->ale, reg);
}

int cpsw_ethtool_op_begin(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int ret;

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        cpsw_err(priv, drv, "ethtool begin failed %d\n", ret);

    return ret;
}

void cpsw_ethtool_op_complete(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    int ret;

    ret = pm_runtime_put(priv->cpsw->dev);
    if (ret < 0)
        cpsw_err(priv, drv, "ethtool complete failed %d\n", ret);
}

void cpsw_get_channels(struct net_device* ndev, struct ethtool_channels* ch) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

    ch->max_rx = cpsw->quirk_irq ? 1 : CPSW_MAX_QUEUES;
    ch->max_tx = cpsw->quirk_irq ? 1 : CPSW_MAX_QUEUES;
    ch->max_combined = 0;
    ch->max_other = 0;
    ch->other_count = 0;
    ch->rx_count = cpsw->rx_ch_num;
    ch->tx_count = cpsw->tx_ch_num;
    ch->combined_count = 0;
}

int cpsw_get_link_ksettings(struct net_device* ndev,
                            struct ethtool_link_ksettings* ecmd) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    if (!cpsw->slaves[slave_no].phy)
        return -EOPNOTSUPP;

    phy_ethtool_ksettings_get(cpsw->slaves[slave_no].phy, ecmd);
    return 0;
}

int cpsw_set_link_ksettings(struct net_device* ndev,
                            const struct ethtool_link_ksettings* ecmd) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    if (!cpsw->slaves[slave_no].phy)
        return -EOPNOTSUPP;

    return phy_ethtool_ksettings_set(cpsw->slaves[slave_no].phy, ecmd);
}

int cpsw_get_eee(struct net_device* ndev, struct ethtool_eee* edata) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    if (cpsw->slaves[slave_no].phy)
        return phy_ethtool_get_eee(cpsw->slaves[slave_no].phy, edata);
    else
        return -EOPNOTSUPP;
}

int cpsw_set_eee(struct net_device* ndev, struct ethtool_eee* edata) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    if (cpsw->slaves[slave_no].phy)
        return phy_ethtool_set_eee(cpsw->slaves[slave_no].phy, edata);
    else
        return -EOPNOTSUPP;
}

int cpsw_nway_reset(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);

    if (cpsw->slaves[slave_no].phy)
        return genphy_restart_aneg(cpsw->slaves[slave_no].phy);
    else
        return -EOPNOTSUPP;
}

static void cpsw_suspend_data_pass(struct net_device* ndev) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);
    int i;

    /* Disable NAPI scheduling */
    cpsw_intr_disable(cpsw);

    /* Stop all transmit queues for every network device.
     */
    for (i = 0; i < cpsw->data.slaves; i++) {
        ndev = cpsw->slaves[i].ndev;
        if (!(ndev && netif_running(ndev)))
            continue;

        netif_tx_stop_all_queues(ndev);

        /* Barrier, so that stop_queue visible to other cpus */
        smp_mb__after_atomic();
    }

    /* Handle rest of tx packets and stop cpdma channels */
    cpdma_ctlr_stop(cpsw->dma);
}

static int cpsw_resume_data_pass(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int i, ret;

    /* After this receive is started */
    if (cpsw->usage_count) {
        ret = cpsw_fill_rx_channels(priv);
        if (ret)
            return ret;

        cpdma_ctlr_start(cpsw->dma);
        cpsw_intr_enable(cpsw);
    }

    /* Resume transmit for every affected interface */
    for (i = 0; i < cpsw->data.slaves; i++) {
        ndev = cpsw->slaves[i].ndev;
        if (ndev && netif_running(ndev))
            netif_tx_start_all_queues(ndev);
    }

    return 0;
}

static int cpsw_check_ch_settings(struct cpsw_common* cpsw,
                                  struct ethtool_channels* ch) {
    if (cpsw->quirk_irq) {
        dev_err(cpsw->dev, "Maximum one tx/rx queue is allowed");
        return -EOPNOTSUPP;
    }

    if (ch->combined_count)
        return -EINVAL;

    /* verify we have at least one channel in each direction */
    if (!ch->rx_count || !ch->tx_count)
        return -EINVAL;

    if (ch->rx_count > cpsw->data.channels ||
        ch->tx_count > cpsw->data.channels)
        return -EINVAL;

    return 0;
}

static int cpsw_update_channels_res(struct cpsw_priv* priv,
                                    int ch_num,
                                    int rx,
                                    cpdma_handler_fn rx_handler) {
    struct cpsw_common* cpsw = priv->cpsw;
    void (*handler)(void*, int, int);
    struct netdev_queue* queue;
    struct cpsw_vector* vec;
    int ret, *ch, vch;

    if (rx) {
        ch = &cpsw->rx_ch_num;
        vec = cpsw->rxv;
        handler = rx_handler;
    } else {
        ch = &cpsw->tx_ch_num;
        vec = cpsw->txv;
        handler = cpsw_tx_handler;
    }

    while (*ch < ch_num) {
        vch = rx ? *ch : 7 - *ch;
        vec[*ch].ch = cpdma_chan_create(cpsw->dma, vch, handler, rx);
        queue = netdev_get_tx_queue(priv->ndev, *ch);
        queue->tx_maxrate = 0;

        if (IS_ERR(vec[*ch].ch))
            return PTR_ERR(vec[*ch].ch);

        if (!vec[*ch].ch)
            return -EINVAL;

        cpsw_info(priv, ifup, "created new %d %s channel\n", *ch,
                  (rx ? "rx" : "tx"));
        (*ch)++;
    }

    while (*ch > ch_num) {
        (*ch)--;

        ret = cpdma_chan_destroy(vec[*ch].ch);
        if (ret)
            return ret;

        cpsw_info(priv, ifup, "destroyed %d %s channel\n", *ch,
                  (rx ? "rx" : "tx"));
    }

    return 0;
}

static void cpsw_fail(struct cpsw_common* cpsw) {
    struct net_device* ndev;
    int i;

    for (i = 0; i < cpsw->data.slaves; i++) {
        ndev = cpsw->slaves[i].ndev;
        if (ndev)
            dev_close(ndev);
    }
}

int cpsw_set_channels_common(struct net_device* ndev,
                             struct ethtool_channels* chs,
                             cpdma_handler_fn rx_handler) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct net_device* sl_ndev;
    int i, new_pools, ret;

    ret = cpsw_check_ch_settings(cpsw, chs);
    if (ret < 0)
        return ret;

    cpsw_suspend_data_pass(ndev);

    new_pools = (chs->rx_count != cpsw->rx_ch_num) && cpsw->usage_count;

    ret = cpsw_update_channels_res(priv, chs->rx_count, 1, rx_handler);
    if (ret)
        goto err;

    ret = cpsw_update_channels_res(priv, chs->tx_count, 0, rx_handler);
    if (ret)
        goto err;

    for (i = 0; i < cpsw->data.slaves; i++) {
        sl_ndev = cpsw->slaves[i].ndev;
        if (!(sl_ndev && netif_running(sl_ndev)))
            continue;

        /* Inform stack about new count of queues */
        ret = netif_set_real_num_tx_queues(sl_ndev, cpsw->tx_ch_num);
        if (ret) {
            dev_err(priv->dev, "cannot set real number of tx queues\n");
            goto err;
        }

        ret = netif_set_real_num_rx_queues(sl_ndev, cpsw->rx_ch_num);
        if (ret) {
            dev_err(priv->dev, "cannot set real number of rx queues\n");
            goto err;
        }
    }

    cpsw_split_res(cpsw);

    if (new_pools) {
        cpsw_destroy_xdp_rxqs(cpsw);
        ret = cpsw_create_xdp_rxqs(cpsw);
        if (ret)
            goto err;
    }

    ret = cpsw_resume_data_pass(ndev);
    if (!ret)
        return 0;
err:
    dev_err(priv->dev, "cannot update channels number, closing device\n");
    cpsw_fail(cpsw);
    return ret;
}

void cpsw_get_ringparam(struct net_device* ndev,
                        struct ethtool_ringparam* ering,
                        struct kernel_ethtool_ringparam* kernel_ering,
                        struct netlink_ext_ack* extack) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;

    /* not supported */
    ering->tx_max_pending = cpsw->descs_pool_size - CPSW_MAX_QUEUES;
    ering->tx_pending = cpdma_get_num_tx_descs(cpsw->dma);
    ering->rx_max_pending = cpsw->descs_pool_size - CPSW_MAX_QUEUES;
    ering->rx_pending = cpdma_get_num_rx_descs(cpsw->dma);
}

int cpsw_set_ringparam(struct net_device* ndev,
                       struct ethtool_ringparam* ering,
                       struct kernel_ethtool_ringparam* kernel_ering,
                       struct netlink_ext_ack* extack) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);
    int descs_num, ret;

    /* ignore ering->tx_pending - only rx_pending adjustment is supported */

    if (ering->rx_mini_pending || ering->rx_jumbo_pending ||
        ering->rx_pending < CPSW_MAX_QUEUES ||
        ering->rx_pending > (cpsw->descs_pool_size - CPSW_MAX_QUEUES))
        return -EINVAL;

    descs_num = cpdma_get_num_rx_descs(cpsw->dma);
    if (ering->rx_pending == descs_num)
        return 0;

    cpsw_suspend_data_pass(ndev);

    ret = cpdma_set_num_rx_descs(cpsw->dma, ering->rx_pending);
    if (ret) {
        if (cpsw_resume_data_pass(ndev))
            goto err;

        return ret;
    }

    if (cpsw->usage_count) {
        cpsw_destroy_xdp_rxqs(cpsw);
        ret = cpsw_create_xdp_rxqs(cpsw);
        if (ret)
            goto err;
    }

    ret = cpsw_resume_data_pass(ndev);
    if (!ret)
        return 0;
err:
    cpdma_set_num_rx_descs(cpsw->dma, descs_num);
    dev_err(cpsw->dev, "cannot set ring params, closing device\n");
    cpsw_fail(cpsw);
    return ret;
}

#if IS_ENABLED(CONFIG_TI_CPTS)
int cpsw_get_ts_info(struct net_device* ndev, struct ethtool_ts_info* info) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

    info->so_timestamping =
        SOF_TIMESTAMPING_TX_HARDWARE | SOF_TIMESTAMPING_TX_SOFTWARE |
        SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_RX_SOFTWARE |
        SOF_TIMESTAMPING_SOFTWARE | SOF_TIMESTAMPING_RAW_HARDWARE;
    info->phc_index = cpsw->cpts->phc_index;
    info->tx_types = (1 << HWTSTAMP_TX_OFF) | (1 << HWTSTAMP_TX_ON);
    info->rx_filters =
        (1 << HWTSTAMP_FILTER_NONE) | (1 << HWTSTAMP_FILTER_PTP_V2_EVENT);
    return 0;
}
#else
int cpsw_get_ts_info(struct net_device* ndev, struct ethtool_ts_info* info) {
    info->so_timestamping = SOF_TIMESTAMPING_TX_SOFTWARE |
                            SOF_TIMESTAMPING_RX_SOFTWARE |
                            SOF_TIMESTAMPING_SOFTWARE;
    info->phc_index = -1;
    info->tx_types = 0;
    info->rx_filters = 0;
    return 0;
}
#endif


void cpsw_intr_enable(struct cpsw_common* cpsw) {
    writel_relaxed(0xFF, &cpsw->wr_regs->tx_en);
    writel_relaxed(0xFF, &cpsw->wr_regs->rx_en);

    cpdma_ctlr_int_ctrl(cpsw->dma, true);
}

void cpsw_intr_disable(struct cpsw_common* cpsw) {
    writel_relaxed(0, &cpsw->wr_regs->tx_en);
    writel_relaxed(0, &cpsw->wr_regs->rx_en);

    cpdma_ctlr_int_ctrl(cpsw->dma, false);
}

void cpsw_tx_handler(void* token, int len, int status) {
    struct cpsw_meta_xdp* xmeta;
    struct xdp_frame* xdpf;
    struct net_device* ndev;
    struct netdev_queue* txq;
    struct sk_buff* skb;
    int ch;
    if (cpsw_is_xdpf_handle(token)) {
        xdpf = cpsw_handle_to_xdpf(token);
        xmeta = (void*)xdpf + CPSW_XMETA_OFFSET;
        ndev = xmeta->ndev;
        ch = xmeta->ch;
        xdp_return_frame(xdpf);
    } else {
        skb = token;
        ndev = skb->dev;
        ch = skb_get_queue_mapping(skb);
        cpts_tx_timestamp(ndev_to_cpsw(ndev)->cpts, skb);
        dev_kfree_skb_any(skb);
    }

    /* Check whether the queue is stopped due to stalled tx dma, if the
     * queue is stopped then start the queue as we have free desc for tx
     */
    txq = netdev_get_tx_queue(ndev, ch);
    if (unlikely(netif_tx_queue_stopped(txq)))
        netif_tx_wake_queue(txq);

    ndev->stats.tx_packets++;
    ndev->stats.tx_bytes += len;
}

irqreturn_t cpsw_tx_interrupt(int irq, void* dev_id) {
    struct cpsw_common* cpsw = dev_id;

    writel(0, &cpsw->wr_regs->tx_en);
    cpdma_ctlr_eoi(cpsw->dma, CPDMA_EOI_TX);

    if (cpsw->quirk_irq) {
        disable_irq_nosync(cpsw->irqs_table[1]);
        cpsw->tx_irq_disabled = true;
    }

    napi_schedule(&cpsw->napi_tx);
    return IRQ_HANDLED;
}

irqreturn_t cpsw_rx_interrupt(int irq, void* dev_id) {
    struct cpsw_common* cpsw = dev_id;

    writel(0, &cpsw->wr_regs->rx_en);
    cpdma_ctlr_eoi(cpsw->dma, CPDMA_EOI_RX);

    if (cpsw->quirk_irq) {
        disable_irq_nosync(cpsw->irqs_table[0]);
        cpsw->rx_irq_disabled = true;
    }

    napi_schedule(&cpsw->napi_rx);
    return IRQ_HANDLED;
}

irqreturn_t cpsw_misc_interrupt(int irq, void* dev_id) {
    struct cpsw_common* cpsw = dev_id;

    writel(0, &cpsw->wr_regs->misc_en);
    cpdma_ctlr_eoi(cpsw->dma, CPDMA_EOI_MISC);
    cpts_misc_interrupt(cpsw->cpts);
    writel(0x10, &cpsw->wr_regs->misc_en);

    return IRQ_HANDLED;
}

int cpsw_tx_mq_poll(struct napi_struct* napi_tx, int budget) {
    struct cpsw_common* cpsw = napi_to_cpsw(napi_tx);
    int num_tx, cur_budget, ch;
    u32 ch_map;
    struct cpsw_vector* txv;

    /* process every unprocessed channel */
    ch_map = cpdma_ctrl_txchs_state(cpsw->dma);
    for (ch = 0, num_tx = 0; ch_map & 0xff; ch_map <<= 1, ch++) {
        if (!(ch_map & 0x80))
            continue;

        txv = &cpsw->txv[ch];
        if (unlikely(txv->budget > budget - num_tx))
            cur_budget = budget - num_tx;
        else
            cur_budget = txv->budget;

        num_tx += cpdma_chan_process(txv->ch, cur_budget);
        if (num_tx >= budget)
            break;
    }

    if (num_tx < budget) {
        napi_complete(napi_tx);
        writel(0xff, &cpsw->wr_regs->tx_en);
    }

    return num_tx;
}

int cpsw_tx_poll(struct napi_struct* napi_tx, int budget) {
    struct cpsw_common* cpsw = napi_to_cpsw(napi_tx);
    int num_tx;

    num_tx = cpdma_chan_process(cpsw->txv[0].ch, budget);
    if (num_tx < budget) {
        napi_complete(napi_tx);
        writel(0xff, &cpsw->wr_regs->tx_en);
        if (cpsw->tx_irq_disabled) {
            cpsw->tx_irq_disabled = false;
            enable_irq(cpsw->irqs_table[1]);
        }
    }

    return num_tx;
}

int cpsw_rx_mq_poll(struct napi_struct* napi_rx, int budget) {
    struct cpsw_common* cpsw = napi_to_cpsw(napi_rx);
    int num_rx, cur_budget, ch;
    u32 ch_map;
    struct cpsw_vector* rxv;

    /* process every unprocessed channel */
    ch_map = cpdma_ctrl_rxchs_state(cpsw->dma);
    for (ch = 0, num_rx = 0; ch_map; ch_map >>= 1, ch++) {
        if (!(ch_map & 0x01))
            continue;

        rxv = &cpsw->rxv[ch];
        if (unlikely(rxv->budget > budget - num_rx))
            cur_budget = budget - num_rx;
        else
            cur_budget = rxv->budget;

        num_rx += cpdma_chan_process(rxv->ch, cur_budget);
        if (num_rx >= budget)
            break;
    }

    if (num_rx < budget) {
        napi_complete_done(napi_rx, num_rx);
        writel(0xff, &cpsw->wr_regs->rx_en);
    }

    return num_rx;
}

int cpsw_rx_poll(struct napi_struct* napi_rx, int budget) {
    struct cpsw_common* cpsw = napi_to_cpsw(napi_rx);
    int num_rx;

    num_rx = cpdma_chan_process(cpsw->rxv[0].ch, budget);
    if (num_rx < budget) {
        napi_complete_done(napi_rx, num_rx);
        writel(0xff, &cpsw->wr_regs->rx_en);
        if (cpsw->rx_irq_disabled) {
            cpsw->rx_irq_disabled = false;
            enable_irq(cpsw->irqs_table[0]);
        }
    }

    return num_rx;
}

void cpsw_rx_vlan_encap(struct sk_buff* skb) {
    struct cpsw_priv* priv = netdev_priv(skb->dev);
    u32 rx_vlan_encap_hdr = *((u32*)skb->data);
    struct cpsw_common* cpsw = priv->cpsw;
    u16 vtag, vid, prio, pkt_type;

    /* Remove VLAN header encapsulation word */
    skb_pull(skb, CPSW_RX_VLAN_ENCAP_HDR_SIZE);

    pkt_type = (rx_vlan_encap_hdr >> CPSW_RX_VLAN_ENCAP_HDR_PKT_TYPE_SHIFT) &
               CPSW_RX_VLAN_ENCAP_HDR_PKT_TYPE_MSK;
    /* Ignore unknown & Priority-tagged packets*/
    if (pkt_type == CPSW_RX_VLAN_ENCAP_HDR_PKT_RESERV ||
        pkt_type == CPSW_RX_VLAN_ENCAP_HDR_PKT_PRIO_TAG)
        return;

    vid =
        (rx_vlan_encap_hdr >> CPSW_RX_VLAN_ENCAP_HDR_VID_SHIFT) & VLAN_VID_MASK;
    /* Ignore vid 0 and pass packet as is */
    if (!vid)
        return;

    /* Untag P0 packets if set for vlan */
    if (!cpsw_ale_get_vlan_p0_untag(cpsw->ale, vid)) {
        prio = (rx_vlan_encap_hdr >> CPSW_RX_VLAN_ENCAP_HDR_PRIO_SHIFT) &
               CPSW_RX_VLAN_ENCAP_HDR_PRIO_MSK;

        vtag = (prio << VLAN_PRIO_SHIFT) | vid;
        __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vtag);
    }

    /* strip vlan tag for VLAN-tagged packet */
    if (pkt_type == CPSW_RX_VLAN_ENCAP_HDR_PKT_VLAN_TAG) {
        memmove(skb->data + VLAN_HLEN, skb->data, 2 * ETH_ALEN);
        skb_pull(skb, VLAN_HLEN);
    }
}

void cpsw_set_slave_mac(struct cpsw_slave* slave, struct cpsw_priv* priv) {
    slave_write(slave, mac_hi(priv->mac_addr), SA_HI);
    slave_write(slave, mac_lo(priv->mac_addr), SA_LO);
}

void soft_reset(const char* module, void __iomem* reg) {
    unsigned long timeout = jiffies + HZ;

    writel_relaxed(1, reg);
    do {
        cpu_relax();
    } while ((readl_relaxed(reg) & 1) && time_after(timeout, jiffies));

    WARN(readl_relaxed(reg) & 1, "failed to soft-reset %s\n", module);
}

void cpsw_ndo_tx_timeout(struct net_device* ndev, unsigned int txqueue) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int ch;

    cpsw_err(priv, tx_err, "transmit timeout, restarting dma\n");
    ndev->stats.tx_errors++;
    cpsw_intr_disable(cpsw);
    for (ch = 0; ch < cpsw->tx_ch_num; ch++) {
        cpdma_chan_stop(cpsw->txv[ch].ch);
        cpdma_chan_start(cpsw->txv[ch].ch);
    }

    cpsw_intr_enable(cpsw);
    netif_trans_update(ndev);
    netif_tx_wake_all_queues(ndev);
}

static int cpsw_get_common_speed(struct cpsw_common* cpsw) {
    int i, speed;

    for (i = 0, speed = 0; i < cpsw->data.slaves; i++)
        if (cpsw->slaves[i].phy && cpsw->slaves[i].phy->link)
            speed += cpsw->slaves[i].phy->speed;

    return speed;
}

int cpsw_need_resplit(struct cpsw_common* cpsw) {
    int i, rlim_ch_num;
    int speed, ch_rate;

    /* re-split resources only in case speed was changed */
    speed = cpsw_get_common_speed(cpsw);
    if (speed == cpsw->speed || !speed)
        return 0;

    cpsw->speed = speed;

    for (i = 0, rlim_ch_num = 0; i < cpsw->tx_ch_num; i++) {
        ch_rate = cpdma_chan_get_rate(cpsw->txv[i].ch);
        if (!ch_rate)
            break;

        rlim_ch_num++;
    }

    /* cases not dependent on speed */
    if (!rlim_ch_num || rlim_ch_num == cpsw->tx_ch_num)
        return 0;

    return 1;
}

void cpsw_split_res(struct cpsw_common* cpsw) {
    u32 consumed_rate = 0, bigest_rate = 0;
    struct cpsw_vector* txv = cpsw->txv;
    int i, ch_weight, rlim_ch_num = 0;
    int budget, bigest_rate_ch = 0;
    u32 ch_rate, max_rate;
    int ch_budget = 0;

    for (i = 0; i < cpsw->tx_ch_num; i++) {
        ch_rate = cpdma_chan_get_rate(txv[i].ch);
        if (!ch_rate)
            continue;

        rlim_ch_num++;
        consumed_rate += ch_rate;
    }

    if (cpsw->tx_ch_num == rlim_ch_num) {
        max_rate = consumed_rate;
    } else if (!rlim_ch_num) {
        ch_budget = NAPI_POLL_WEIGHT / cpsw->tx_ch_num;
        bigest_rate = 0;
        max_rate = consumed_rate;
    } else {
        max_rate = cpsw->speed * 1000;

        /* if max_rate is less then expected due to reduced link speed,
         * split proportionally according next potential max speed
         */
        if (max_rate < consumed_rate)
            max_rate *= 10;

        if (max_rate < consumed_rate)
            max_rate *= 10;

        ch_budget = (consumed_rate * NAPI_POLL_WEIGHT) / max_rate;
        ch_budget =
            (NAPI_POLL_WEIGHT - ch_budget) / (cpsw->tx_ch_num - rlim_ch_num);
        bigest_rate =
            (max_rate - consumed_rate) / (cpsw->tx_ch_num - rlim_ch_num);
    }

    /* split tx weight/budget */
    budget = NAPI_POLL_WEIGHT;
    for (i = 0; i < cpsw->tx_ch_num; i++) {
        ch_rate = cpdma_chan_get_rate(txv[i].ch);
        if (ch_rate) {
            txv[i].budget = (ch_rate * NAPI_POLL_WEIGHT) / max_rate;
            if (!txv[i].budget)
                txv[i].budget++;
            if (ch_rate > bigest_rate) {
                bigest_rate_ch = i;
                bigest_rate = ch_rate;
            }

            ch_weight = (ch_rate * 100) / max_rate;
            if (!ch_weight)
                ch_weight++;
            cpdma_chan_set_weight(cpsw->txv[i].ch, ch_weight);
        } else {
            txv[i].budget = ch_budget;
            if (!bigest_rate_ch)
                bigest_rate_ch = i;
            cpdma_chan_set_weight(cpsw->txv[i].ch, 0);
        }

        budget -= txv[i].budget;
    }

    if (budget)
        txv[bigest_rate_ch].budget += budget;

    /* split rx budget */
    budget = NAPI_POLL_WEIGHT;
    ch_budget = budget / cpsw->rx_ch_num;
    for (i = 0; i < cpsw->rx_ch_num; i++) {
        cpsw->rxv[i].budget = ch_budget;
        budget -= ch_budget;
    }

    if (budget)
        cpsw->rxv[0].budget += budget;
}

int cpsw_init_common(struct cpsw_common* cpsw,
                     void __iomem* ss_regs,
                     int ale_ageout,
                     phys_addr_t desc_mem_phys,
                     int descs_pool_size) {
    u32 slave_offset, sliver_offset, slave_size;
    struct cpsw_ale_params ale_params;
    struct cpsw_platform_data* data;
    struct cpdma_params dma_params;
    struct device* dev = cpsw->dev;
    struct device_node* cpts_node;
    void __iomem* cpts_regs;
    int ret = 0, i;

    data = &cpsw->data;
    cpsw->rx_ch_num = 1;
    cpsw->tx_ch_num = 1;

    cpsw->version = readl(&cpsw->regs->id_ver);

    memset(&dma_params, 0, sizeof(dma_params));
    memset(&ale_params, 0, sizeof(ale_params));

    switch (cpsw->version) {
        case CPSW_VERSION_1:
            cpsw->host_port_regs = ss_regs + CPSW1_HOST_PORT_OFFSET;
            cpts_regs = ss_regs + CPSW1_CPTS_OFFSET;
            cpsw->hw_stats = ss_regs + CPSW1_HW_STATS;
            dma_params.dmaregs = ss_regs + CPSW1_CPDMA_OFFSET;
            dma_params.txhdp = ss_regs + CPSW1_STATERAM_OFFSET;
            ale_params.ale_regs = ss_regs + CPSW1_ALE_OFFSET;
            slave_offset = CPSW1_SLAVE_OFFSET;
            slave_size = CPSW1_SLAVE_SIZE;
            sliver_offset = CPSW1_SLIVER_OFFSET;
            dma_params.desc_mem_phys = 0;
            break;
        case CPSW_VERSION_2:
        case CPSW_VERSION_3:
        case CPSW_VERSION_4:
            cpsw->host_port_regs = ss_regs + CPSW2_HOST_PORT_OFFSET;
            cpts_regs = ss_regs + CPSW2_CPTS_OFFSET;
            cpsw->hw_stats = ss_regs + CPSW2_HW_STATS;
            dma_params.dmaregs = ss_regs + CPSW2_CPDMA_OFFSET;
            dma_params.txhdp = ss_regs + CPSW2_STATERAM_OFFSET;
            ale_params.ale_regs = ss_regs + CPSW2_ALE_OFFSET;
            slave_offset = CPSW2_SLAVE_OFFSET;
            slave_size = CPSW2_SLAVE_SIZE;
            sliver_offset = CPSW2_SLIVER_OFFSET;
            dma_params.desc_mem_phys = desc_mem_phys;
            break;
        default:
            dev_err(dev, "unknown version 0x%08x\n", cpsw->version);
            return -ENODEV;
    }

    for (i = 0; i < cpsw->data.slaves; i++) {
        struct cpsw_slave* slave = &cpsw->slaves[i];
        void __iomem* regs = cpsw->regs;

        slave->slave_num = i;
        slave->data = &cpsw->data.slave_data[i];
        slave->regs = regs + slave_offset;
        slave->port_vlan = slave->data->dual_emac_res_vlan;
        slave->mac_sl = cpsw_sl_get("cpsw", dev, regs + sliver_offset);
        if (IS_ERR(slave->mac_sl))
            return PTR_ERR(slave->mac_sl);

        slave_offset += slave_size;
        sliver_offset += SLIVER_SIZE;
    }

    ale_params.dev = dev;
    ale_params.ale_ageout = ale_ageout;
    ale_params.ale_ports = CPSW_ALE_PORTS_NUM;
    ale_params.dev_id = "cpsw";
    ale_params.bus_freq = cpsw->bus_freq_mhz * 1000000;

    cpsw->ale = cpsw_ale_create(&ale_params);
    if (IS_ERR(cpsw->ale)) {
        dev_err(dev, "error initializing ale engine\n");
        return PTR_ERR(cpsw->ale);
    }

    dma_params.dev = dev;
    dma_params.rxthresh = dma_params.dmaregs + CPDMA_RXTHRESH;
    dma_params.rxfree = dma_params.dmaregs + CPDMA_RXFREE;
    dma_params.rxhdp = dma_params.txhdp + CPDMA_RXHDP;
    dma_params.txcp = dma_params.txhdp + CPDMA_TXCP;
    dma_params.rxcp = dma_params.txhdp + CPDMA_RXCP;

    dma_params.num_chan = data->channels;
    dma_params.has_soft_reset = true;
    dma_params.min_packet_size = CPSW_MIN_PACKET_SIZE;
    dma_params.desc_mem_size = data->bd_ram_size;
    dma_params.desc_align = 16;
    dma_params.has_ext_regs = true;
    dma_params.desc_hw_addr = dma_params.desc_mem_phys;
    dma_params.bus_freq_mhz = cpsw->bus_freq_mhz;
    dma_params.descs_pool_size = descs_pool_size;

    cpsw->dma = cpdma_ctlr_create(&dma_params);
    if (!cpsw->dma) {
        dev_err(dev, "error initializing dma\n");
        return -ENOMEM;
    }

    cpts_node = of_get_child_by_name(cpsw->dev->of_node, "cpts");
    if (!cpts_node)
        cpts_node = cpsw->dev->of_node;

    cpsw->cpts = cpts_create(cpsw->dev, cpts_regs, cpts_node, CPTS_N_ETX_TS);
    if (IS_ERR(cpsw->cpts)) {
        ret = PTR_ERR(cpsw->cpts);
        cpdma_ctlr_destroy(cpsw->dma);
    }
    of_node_put(cpts_node);

    return ret;
}

#if IS_ENABLED(CONFIG_TI_CPTS)

static void cpsw_hwtstamp_v1(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    u32 ts_en, seq_id;

    if (!priv->tx_ts_enabled && !priv->rx_ts_enabled) {
        slave_write(slave, 0, CPSW1_TS_CTL);
        return;
    }

    seq_id = (30 << CPSW_V1_SEQ_ID_OFS_SHIFT) | ETH_P_1588;
    ts_en = EVENT_MSG_BITS << CPSW_V1_MSG_TYPE_OFS;

    if (priv->tx_ts_enabled)
        ts_en |= CPSW_V1_TS_TX_EN;

    if (priv->rx_ts_enabled)
        ts_en |= CPSW_V1_TS_RX_EN;

    slave_write(slave, ts_en, CPSW1_TS_CTL);
    slave_write(slave, seq_id, CPSW1_TS_SEQ_LTYPE);
}

static void cpsw_hwtstamp_v2(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    u32 ctrl, mtype;

    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];

    ctrl = slave_read(slave, CPSW2_CONTROL);
    switch (cpsw->version) {
        case CPSW_VERSION_2:
            ctrl &= ~CTRL_V2_ALL_TS_MASK;

            if (priv->tx_ts_enabled)
                ctrl |= CTRL_V2_TX_TS_BITS;

            if (priv->rx_ts_enabled)
                ctrl |= CTRL_V2_RX_TS_BITS;
            break;
        case CPSW_VERSION_3:
        default:
            ctrl &= ~CTRL_V3_ALL_TS_MASK;

            if (priv->tx_ts_enabled)
                ctrl |= CTRL_V3_TX_TS_BITS;

            if (priv->rx_ts_enabled)
                ctrl |= CTRL_V3_RX_TS_BITS;
            break;
    }

    mtype = (30 << TS_SEQ_ID_OFFSET_SHIFT) | EVENT_MSG_BITS;

    slave_write(slave, mtype, CPSW2_TS_SEQ_MTYPE);
    slave_write(slave, ctrl, CPSW2_CONTROL);
    writel_relaxed(ETH_P_1588, &cpsw->regs->ts_ltype);
    writel_relaxed(ETH_P_8021Q, &cpsw->regs->vlan_ltype);
}

static int cpsw_hwtstamp_set(struct net_device* dev, struct ifreq* ifr) {
    struct cpsw_priv* priv = netdev_priv(dev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct hwtstamp_config cfg;

    if (cpsw->version != CPSW_VERSION_1 && cpsw->version != CPSW_VERSION_2 &&
        cpsw->version != CPSW_VERSION_3)
        return -EOPNOTSUPP;

    if (copy_from_user(&cfg, ifr->ifr_data, sizeof(cfg)))
        return -EFAULT;

    if (cfg.tx_type != HWTSTAMP_TX_OFF && cfg.tx_type != HWTSTAMP_TX_ON)
        return -ERANGE;

    switch (cfg.rx_filter) {
        case HWTSTAMP_FILTER_NONE:
            priv->rx_ts_enabled = 0;
            break;
        case HWTSTAMP_FILTER_ALL:
        case HWTSTAMP_FILTER_NTP_ALL:
        case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
        case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
        case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
            return -ERANGE;
        case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
        case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
        case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
        case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
        case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
        case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
        case HWTSTAMP_FILTER_PTP_V2_EVENT:
        case HWTSTAMP_FILTER_PTP_V2_SYNC:
        case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
            priv->rx_ts_enabled = HWTSTAMP_FILTER_PTP_V2_EVENT;
            cfg.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
            break;
        default:
            return -ERANGE;
    }

    priv->tx_ts_enabled = cfg.tx_type == HWTSTAMP_TX_ON;

    switch (cpsw->version) {
        case CPSW_VERSION_1:
            cpsw_hwtstamp_v1(priv);
            break;
        case CPSW_VERSION_2:
        case CPSW_VERSION_3:
            cpsw_hwtstamp_v2(priv);
            break;
        default:
            WARN_ON(1);
    }

    return copy_to_user(ifr->ifr_data, &cfg, sizeof(cfg)) ? -EFAULT : 0;
}

static int cpsw_hwtstamp_get(struct net_device* dev, struct ifreq* ifr) {
    struct cpsw_common* cpsw = ndev_to_cpsw(dev);
    struct cpsw_priv* priv = netdev_priv(dev);
    struct hwtstamp_config cfg;

    if (cpsw->version != CPSW_VERSION_1 && cpsw->version != CPSW_VERSION_2 &&
        cpsw->version != CPSW_VERSION_3)
        return -EOPNOTSUPP;

    cfg.flags = 0;
    cfg.tx_type = priv->tx_ts_enabled ? HWTSTAMP_TX_ON : HWTSTAMP_TX_OFF;
    cfg.rx_filter = priv->rx_ts_enabled;

    return copy_to_user(ifr->ifr_data, &cfg, sizeof(cfg)) ? -EFAULT : 0;
}
#else
static int cpsw_hwtstamp_get(struct net_device* dev, struct ifreq* ifr) {
    return -EOPNOTSUPP;
}

static int cpsw_hwtstamp_set(struct net_device* dev, struct ifreq* ifr) {
    return -EOPNOTSUPP;
}
#endif /*CONFIG_TI_CPTS*/

int cpsw_ndo_ioctl(struct net_device* dev, struct ifreq* req, int cmd) {
    struct cpsw_priv* priv = netdev_priv(dev);
    struct cpsw_common* cpsw = priv->cpsw;
    int slave_no = cpsw_slave_index(cpsw, priv);
    struct phy_device* phy;

    if (!netif_running(dev))
        return -EINVAL;

    phy = cpsw->slaves[slave_no].phy;

    if (!phy_has_hwtstamp(phy)) {
        switch (cmd) {
            case SIOCSHWTSTAMP:
                return cpsw_hwtstamp_set(dev, req);
            case SIOCGHWTSTAMP:
                return cpsw_hwtstamp_get(dev, req);
        }
    }

    if (phy)
        return phy_mii_ioctl(phy, req, cmd);

    return -EOPNOTSUPP;
}

int cpsw_ndo_set_tx_maxrate(struct net_device* ndev, int queue, u32 rate) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    u32 min_rate;
    u32 ch_rate;
    int i, ret;

    ch_rate = netdev_get_tx_queue(ndev, queue)->tx_maxrate;
    if (ch_rate == rate)
        return 0;

    ch_rate = rate * 1000;
    min_rate = cpdma_chan_get_min_rate(cpsw->dma);
    if ((ch_rate < min_rate && ch_rate)) {
        dev_err(priv->dev, "The channel rate cannot be less than %dMbps",
                min_rate);
        return -EINVAL;
    }

    if (rate > cpsw->speed) {
        dev_err(priv->dev, "The channel rate cannot be more than 2Gbps");
        return -EINVAL;
    }

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    ret = cpdma_chan_set_rate(cpsw->txv[queue].ch, ch_rate);
    pm_runtime_put(cpsw->dev);

    if (ret)
        return ret;

    /* update rates for slaves tx queues */
    for (i = 0; i < cpsw->data.slaves; i++) {
        slave = &cpsw->slaves[i];
        if (!slave->ndev)
            continue;

        netdev_get_tx_queue(slave->ndev, queue)->tx_maxrate = rate;
    }

    cpsw_split_res(cpsw);
    return ret;
}

static int cpsw_tc_to_fifo(int tc, int num_tc) {
    if (tc == num_tc - 1)
        return 0;

    return CPSW_FIFO_SHAPERS_NUM - tc;
}

bool cpsw_shp_is_off(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    u32 shift, mask, val;

    val = readl_relaxed(&cpsw->regs->ptype);

    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    shift = CPSW_FIFO_SHAPE_EN_SHIFT + 3 * slave->slave_num;
    mask = 7 << shift;
    val = val & mask;

    return !val;
}

static void cpsw_fifo_shp_on(struct cpsw_priv* priv, int fifo, int on) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    u32 shift, mask, val;

    val = readl_relaxed(&cpsw->regs->ptype);

    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    shift = CPSW_FIFO_SHAPE_EN_SHIFT + 3 * slave->slave_num;
    mask = (1 << --fifo) << shift;
    val = on ? val | mask : val & ~mask;

    writel_relaxed(val, &cpsw->regs->ptype);
}

static int cpsw_set_fifo_bw(struct cpsw_priv* priv, int fifo, int bw) {
    struct cpsw_common* cpsw = priv->cpsw;
    u32 val = 0, send_pct, shift;
    struct cpsw_slave* slave;
    int pct = 0, i;

    if (bw > priv->shp_cfg_speed * 1000)
        goto err;

    /* shaping has to stay enabled for highest fifos linearly
     * and fifo bw no more then interface can allow
     */
    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    send_pct = slave_read(slave, SEND_PERCENT);
    for (i = CPSW_FIFO_SHAPERS_NUM; i > 0; i--) {
        if (!bw) {
            if (i >= fifo || !priv->fifo_bw[i])
                continue;

            dev_warn(priv->dev, "Prev FIFO%d is shaped", i);
            continue;
        }

        if (!priv->fifo_bw[i] && i > fifo) {
            dev_err(priv->dev, "Upper FIFO%d is not shaped", i);
            return -EINVAL;
        }

        shift = (i - 1) * 8;
        if (i == fifo) {
            send_pct &= ~(CPSW_PCT_MASK << shift);
            val = DIV_ROUND_UP(bw, priv->shp_cfg_speed * 10);
            if (!val)
                val = 1;

            send_pct |= val << shift;
            pct += val;
            continue;
        }

        if (priv->fifo_bw[i])
            pct += (send_pct >> shift) & CPSW_PCT_MASK;
    }

    if (pct >= 100)
        goto err;

    slave_write(slave, send_pct, SEND_PERCENT);
    priv->fifo_bw[fifo] = bw;

    dev_warn(priv->dev, "set FIFO%d bw = %d\n", fifo,
             DIV_ROUND_CLOSEST(val * priv->shp_cfg_speed, 100));

    return 0;
err:
    dev_err(priv->dev, "Bandwidth doesn't fit in tc configuration");
    return -EINVAL;
}

static int cpsw_set_fifo_rlimit(struct cpsw_priv* priv, int fifo, int bw) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    u32 tx_in_ctl_rg, val;
    int ret;

    ret = cpsw_set_fifo_bw(priv, fifo, bw);
    if (ret)
        return ret;

    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    tx_in_ctl_rg =
        cpsw->version == CPSW_VERSION_1 ? CPSW1_TX_IN_CTL : CPSW2_TX_IN_CTL;

    if (!bw)
        cpsw_fifo_shp_on(priv, fifo, bw);

    val = slave_read(slave, tx_in_ctl_rg);
    if (cpsw_shp_is_off(priv)) {
        /* disable FIFOs rate limited queues */
        val &= ~(0xf << CPSW_FIFO_RATE_EN_SHIFT);

        /* set type of FIFO queues to normal priority mode */
        val &= ~(3 << CPSW_FIFO_QUEUE_TYPE_SHIFT);

        /* set type of FIFO queues to be rate limited */
        if (bw)
            val |= 2 << CPSW_FIFO_QUEUE_TYPE_SHIFT;
        else
            priv->shp_cfg_speed = 0;
    }

    /* toggle a FIFO rate limited queue */
    if (bw)
        val |= BIT(fifo + CPSW_FIFO_RATE_EN_SHIFT);
    else
        val &= ~BIT(fifo + CPSW_FIFO_RATE_EN_SHIFT);
    slave_write(slave, val, tx_in_ctl_rg);

    /* FIFO transmit shape enable */
    cpsw_fifo_shp_on(priv, fifo, bw);
    return 0;
}

/* Defaults:
 * class A - prio 3
 * class B - prio 2
 * shaping for class A should be set first
 */
static int cpsw_set_cbs(struct net_device* ndev,
                        struct tc_cbs_qopt_offload* qopt) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    int prev_speed = 0;
    int tc, ret, fifo;
    u32 bw = 0;

    tc = netdev_txq_to_tc(priv->ndev, qopt->queue);

    /* enable channels in backward order, as highest FIFOs must be rate
     * limited first and for compliance with CPDMA rate limited channels
     * that also used in bacward order. FIFO0 cannot be rate limited.
     */
    fifo = cpsw_tc_to_fifo(tc, ndev->num_tc);
    if (!fifo) {
        dev_err(priv->dev, "Last tc%d can't be rate limited", tc);
        return -EINVAL;
    }

    /* do nothing, it's disabled anyway */
    if (!qopt->enable && !priv->fifo_bw[fifo])
        return 0;

    /* shapers can be set if link speed is known */
    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    if (slave->phy && slave->phy->link) {
        if (priv->shp_cfg_speed && priv->shp_cfg_speed != slave->phy->speed)
            prev_speed = priv->shp_cfg_speed;

        priv->shp_cfg_speed = slave->phy->speed;
    }

    if (!priv->shp_cfg_speed) {
        dev_err(priv->dev, "Link speed is not known");
        return -1;
    }

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    bw = qopt->enable ? qopt->idleslope : 0;
    ret = cpsw_set_fifo_rlimit(priv, fifo, bw);
    if (ret) {
        priv->shp_cfg_speed = prev_speed;
        prev_speed = 0;
    }

    if (bw && prev_speed)
        dev_warn(priv->dev,
                 "Speed was changed, CBS shaper speeds are changed!");

    pm_runtime_put_sync(cpsw->dev);
    return ret;
}

static int cpsw_set_mqprio(struct net_device* ndev, void* type_data) {
    struct tc_mqprio_qopt_offload* mqprio = type_data;
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int fifo, num_tc, count, offset;
    struct cpsw_slave* slave;
    u32 tx_prio_map = 0;
    int i, tc, ret;

    num_tc = mqprio->qopt.num_tc;
    if (num_tc > CPSW_TC_NUM)
        return -EINVAL;

    if (mqprio->mode != TC_MQPRIO_MODE_DCB)
        return -EINVAL;

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    if (num_tc) {
        for (i = 0; i < 8; i++) {
            tc = mqprio->qopt.prio_tc_map[i];
            fifo = cpsw_tc_to_fifo(tc, num_tc);
            tx_prio_map |= fifo << (4 * i);
        }

        netdev_set_num_tc(ndev, num_tc);
        for (i = 0; i < num_tc; i++) {
            count = mqprio->qopt.count[i];
            offset = mqprio->qopt.offset[i];
            netdev_set_tc_queue(ndev, i, count, offset);
        }
    }

    if (!mqprio->qopt.hw) {
        /* restore default configuration */
        netdev_reset_tc(ndev);
        tx_prio_map = TX_PRIORITY_MAPPING;
    }

    priv->mqprio_hw = mqprio->qopt.hw;

    offset =
        cpsw->version == CPSW_VERSION_1 ? CPSW1_TX_PRI_MAP : CPSW2_TX_PRI_MAP;

    slave = &cpsw->slaves[cpsw_slave_index(cpsw, priv)];
    slave_write(slave, tx_prio_map, offset);

    pm_runtime_put_sync(cpsw->dev);

    return 0;
}

static int cpsw_qos_setup_tc_block(struct net_device* ndev,
                                   struct flow_block_offload* f);

int cpsw_ndo_setup_tc(struct net_device* ndev,
                      enum tc_setup_type type,
                      void* type_data) {
    switch (type) {
        case TC_SETUP_QDISC_CBS:
            return cpsw_set_cbs(ndev, type_data);

        case TC_SETUP_QDISC_MQPRIO:
            return cpsw_set_mqprio(ndev, type_data);

        case TC_SETUP_BLOCK:
            return cpsw_qos_setup_tc_block(ndev, type_data);

        default:
            return -EOPNOTSUPP;
    }
}

void cpsw_cbs_resume(struct cpsw_slave* slave, struct cpsw_priv* priv) {
    int fifo, bw;

    for (fifo = CPSW_FIFO_SHAPERS_NUM; fifo > 0; fifo--) {
        bw = priv->fifo_bw[fifo];
        if (!bw)
            continue;

        cpsw_set_fifo_rlimit(priv, fifo, bw);
    }
}

void cpsw_mqprio_resume(struct cpsw_slave* slave, struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    u32 tx_prio_map = 0;
    int i, tc, fifo;
    u32 tx_prio_rg;

    if (!priv->mqprio_hw)
        return;

    for (i = 0; i < 8; i++) {
        tc = netdev_get_prio_tc_map(priv->ndev, i);
        fifo = CPSW_FIFO_SHAPERS_NUM - tc;
        tx_prio_map |= fifo << (4 * i);
    }

    tx_prio_rg =
        cpsw->version == CPSW_VERSION_1 ? CPSW1_TX_PRI_MAP : CPSW2_TX_PRI_MAP;

    slave_write(slave, tx_prio_map, tx_prio_rg);
}

int cpsw_fill_rx_channels(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_meta_xdp* xmeta;
    struct page_pool* pool;
    struct page* page;
    int ch_buf_num;
    int ch, i, ret;
    dma_addr_t dma;

    for (ch = 0; ch < cpsw->rx_ch_num; ch++) {
        pool = cpsw->page_pool[ch];
        ch_buf_num = cpdma_chan_get_rx_buf_num(cpsw->rxv[ch].ch);
        for (i = 0; i < ch_buf_num; i++) {
            page = page_pool_dev_alloc_pages(pool);
            if (!page) {
                cpsw_err(priv, ifup, "allocate rx page err\n");
                return -ENOMEM;
            }

            xmeta = page_address(page) + CPSW_XMETA_OFFSET;
            xmeta->ndev = priv->ndev;
            xmeta->ch = ch;

            dma = page_pool_get_dma_addr(page) + CPSW_HEADROOM_NA;
            ret = cpdma_chan_idle_submit_mapped(cpsw->rxv[ch].ch, page, dma,
                                                cpsw->rx_packet_max, 0);
            if (ret < 0) {
                cpsw_err(priv, ifup,
                         "cannot submit page to channel %d rx, error %d\n", ch,
                         ret);
                page_pool_recycle_direct(pool, page);
                return ret;
            }
        }

        cpsw_info(priv, ifup, "ch %d rx, submitted %d descriptors\n", ch,
                  ch_buf_num);
    }

    return 0;
}

static struct page_pool* cpsw_create_page_pool(struct cpsw_common* cpsw,
                                               int size) {
    struct page_pool_params pp_params = {};
    struct page_pool* pool;

    pp_params.order = 0;
    pp_params.flags = PP_FLAG_DMA_MAP;
    pp_params.pool_size = size;
    pp_params.nid = NUMA_NO_NODE;
    pp_params.dma_dir = DMA_BIDIRECTIONAL;
    pp_params.dev = cpsw->dev;

    pool = page_pool_create(&pp_params);
    if (IS_ERR(pool))
        dev_err(cpsw->dev, "cannot create rx page pool\n");

    return pool;
}

static int cpsw_create_rx_pool(struct cpsw_common* cpsw, int ch) {
    struct page_pool* pool;
    int ret = 0, pool_size;

    pool_size = cpdma_chan_get_rx_buf_num(cpsw->rxv[ch].ch);
    pool = cpsw_create_page_pool(cpsw, pool_size);
    if (IS_ERR(pool))
        ret = PTR_ERR(pool);
    else
        cpsw->page_pool[ch] = pool;

    return ret;
}

static int cpsw_ndev_create_xdp_rxq(struct cpsw_priv* priv, int ch) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct xdp_rxq_info* rxq;
    struct page_pool* pool;
    int ret;

    pool = cpsw->page_pool[ch];
    rxq = &priv->xdp_rxq[ch];

    ret = xdp_rxq_info_reg(rxq, priv->ndev, ch, 0);
    if (ret)
        return ret;

    ret = xdp_rxq_info_reg_mem_model(rxq, MEM_TYPE_PAGE_POOL, pool);
    if (ret)
        xdp_rxq_info_unreg(rxq);

    return ret;
}

static void cpsw_ndev_destroy_xdp_rxq(struct cpsw_priv* priv, int ch) {
    struct xdp_rxq_info* rxq = &priv->xdp_rxq[ch];

    if (!xdp_rxq_info_is_reg(rxq))
        return;

    xdp_rxq_info_unreg(rxq);
}

void cpsw_destroy_xdp_rxqs(struct cpsw_common* cpsw) {
    struct net_device* ndev;
    int i, ch;

    for (ch = 0; ch < cpsw->rx_ch_num; ch++) {
        for (i = 0; i < cpsw->data.slaves; i++) {
            ndev = cpsw->slaves[i].ndev;
            if (!ndev)
                continue;

            cpsw_ndev_destroy_xdp_rxq(netdev_priv(ndev), ch);
        }

        page_pool_destroy(cpsw->page_pool[ch]);
        cpsw->page_pool[ch] = NULL;
    }
}

int cpsw_create_xdp_rxqs(struct cpsw_common* cpsw) {
    struct net_device* ndev;
    int i, ch, ret;

    for (ch = 0; ch < cpsw->rx_ch_num; ch++) {
        ret = cpsw_create_rx_pool(cpsw, ch);
        if (ret)
            goto err_cleanup;

        /* using same page pool is allowed as no running rx handlers
         * simultaneously for both ndevs
         */
        for (i = 0; i < cpsw->data.slaves; i++) {
            ndev = cpsw->slaves[i].ndev;
            if (!ndev)
                continue;

            ret = cpsw_ndev_create_xdp_rxq(netdev_priv(ndev), ch);
            if (ret)
                goto err_cleanup;
        }
    }

    return 0;

err_cleanup:
    cpsw_destroy_xdp_rxqs(cpsw);

    return ret;
}

static int cpsw_xdp_prog_setup(struct cpsw_priv* priv, struct netdev_bpf* bpf) {
    struct bpf_prog* prog = bpf->prog;

    if (!priv->xdpi.prog && !prog)
        return 0;

    WRITE_ONCE(priv->xdp_prog, prog);

    xdp_attachment_setup(&priv->xdpi, bpf);

    return 0;
}

int cpsw_ndo_bpf(struct net_device* ndev, struct netdev_bpf* bpf) {
    struct cpsw_priv* priv = netdev_priv(ndev);

    switch (bpf->command) {
        case XDP_SETUP_PROG:
            return cpsw_xdp_prog_setup(priv, bpf);

        default:
            return -EINVAL;
    }
}

int cpsw_xdp_tx_frame(struct cpsw_priv* priv,
                      struct xdp_frame* xdpf,
                      struct page* page,
                      int port) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_meta_xdp* xmeta;
    struct cpdma_chan* txch;
    dma_addr_t dma;
    int ret;

    xmeta = (void*)xdpf + CPSW_XMETA_OFFSET;
    xmeta->ndev = priv->ndev;
    xmeta->ch = 0;
    txch = cpsw->txv[0].ch;

    if (page) {
        dma = page_pool_get_dma_addr(page);
        dma += xdpf->headroom + sizeof(struct xdp_frame);
        ret = cpdma_chan_submit_mapped(txch, cpsw_xdpf_to_handle(xdpf), dma,
                                       xdpf->len, port);
    } else {
        if (sizeof(*xmeta) > xdpf->headroom)
            return -EINVAL;

        ret = cpdma_chan_submit(txch, cpsw_xdpf_to_handle(xdpf), xdpf->data,
                                xdpf->len, port);
    }

    if (ret)
        priv->ndev->stats.tx_dropped++;

    return ret;
}

int cpsw_run_xdp(struct cpsw_priv* priv,
                 int ch,
                 struct xdp_buff* xdp,
                 struct page* page,
                 int port,
                 int* len) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct net_device* ndev = priv->ndev;
    int ret = CPSW_XDP_CONSUMED;
    struct xdp_frame* xdpf;
    struct bpf_prog* prog;
    u32 act;

    prog = READ_ONCE(priv->xdp_prog);
    if (!prog)
        return CPSW_XDP_PASS;

    act = bpf_prog_run_xdp(prog, xdp);
    /* XDP prog might have changed packet data and boundaries */
    *len = xdp->data_end - xdp->data;

    switch (act) {
        case XDP_PASS:
            ret = CPSW_XDP_PASS;
            goto out;
        case XDP_TX:
            xdpf = xdp_convert_buff_to_frame(xdp);
            if (unlikely(!xdpf))
                goto drop;

            if (cpsw_xdp_tx_frame(priv, xdpf, page, port))
                xdp_return_frame_rx_napi(xdpf);
            break;
        case XDP_REDIRECT:
            if (xdp_do_redirect(ndev, xdp, prog))
                goto drop;

            /*  Have to flush here, per packet, instead of doing it in bulk
             *  at the end of the napi handler. The RX devices on this
             *  particular hardware is sharing a common queue, so the
             *  incoming device might change per packet.
             */
            xdp_do_flush_map();
            break;
        default:
            bpf_warn_invalid_xdp_action(ndev, prog, act);
            fallthrough;
        case XDP_ABORTED:
            trace_xdp_exception(ndev, prog, act);
            fallthrough; /* handle aborts by dropping packet */
        case XDP_DROP:
            ndev->stats.rx_bytes += *len;
            ndev->stats.rx_packets++;
            goto drop;
    }

    ndev->stats.rx_bytes += *len;
    ndev->stats.rx_packets++;
out:
    return ret;
drop:
    page_pool_recycle_direct(cpsw->page_pool[ch], page);
    return ret;
}

static int cpsw_qos_clsflower_add_policer(struct cpsw_priv* priv,
                                          struct netlink_ext_ack* extack,
                                          struct flow_cls_offload* cls,
                                          u64 rate_pkt_ps) {
    struct flow_rule* rule = flow_cls_offload_flow_rule(cls);
    struct flow_dissector* dissector = rule->match.dissector;
    static const u8 mc_mac[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct flow_match_eth_addrs match;
    u32 port_id;
    int ret;

    if (dissector->used_keys &
        ~(BIT(FLOW_DISSECTOR_KEY_BASIC) | BIT(FLOW_DISSECTOR_KEY_CONTROL) |
          BIT(FLOW_DISSECTOR_KEY_ETH_ADDRS))) {
        NL_SET_ERR_MSG_MOD(extack, "Unsupported keys used");
        return -EOPNOTSUPP;
    }

    if (!flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_ETH_ADDRS)) {
        NL_SET_ERR_MSG_MOD(extack, "Not matching on eth address");
        return -EOPNOTSUPP;
    }

    flow_rule_match_eth_addrs(rule, &match);

    if (!is_zero_ether_addr(match.mask->src)) {
        NL_SET_ERR_MSG_MOD(extack, "Matching on source MAC not supported");
        return -EOPNOTSUPP;
    }

    port_id = cpsw_slave_index(priv->cpsw, priv) + 1;

    if (is_broadcast_ether_addr(match.key->dst) &&
        is_broadcast_ether_addr(match.mask->dst)) {
        ret = cpsw_ale_rx_ratelimit_bc(priv->cpsw->ale, port_id, rate_pkt_ps);
        if (ret)
            return ret;

        priv->ale_bc_ratelimit.cookie = cls->cookie;
        priv->ale_bc_ratelimit.rate_packet_ps = rate_pkt_ps;
    } else if (ether_addr_equal_unaligned(match.key->dst, mc_mac) &&
               ether_addr_equal_unaligned(match.mask->dst, mc_mac)) {
        ret = cpsw_ale_rx_ratelimit_mc(priv->cpsw->ale, port_id, rate_pkt_ps);
        if (ret)
            return ret;

        priv->ale_mc_ratelimit.cookie = cls->cookie;
        priv->ale_mc_ratelimit.rate_packet_ps = rate_pkt_ps;
    } else {
        NL_SET_ERR_MSG_MOD(extack, "Not supported matching key");
        return -EOPNOTSUPP;
    }

    return 0;
}

static int cpsw_qos_clsflower_policer_validate(
    const struct flow_action* action,
    const struct flow_action_entry* act,
    struct netlink_ext_ack* extack) {
    if (act->police.exceed.act_id != FLOW_ACTION_DROP) {
        NL_SET_ERR_MSG_MOD(
            extack, "Offload not supported when exceed action is not drop");
        return -EOPNOTSUPP;
    }

    if (act->police.notexceed.act_id != FLOW_ACTION_PIPE &&
        act->police.notexceed.act_id != FLOW_ACTION_ACCEPT) {
        NL_SET_ERR_MSG_MOD(
            extack,
            "Offload not supported when conform action is not pipe or ok");
        return -EOPNOTSUPP;
    }

    if (act->police.notexceed.act_id == FLOW_ACTION_ACCEPT &&
        !flow_action_is_last_entry(action, act)) {
        NL_SET_ERR_MSG_MOD(extack,
                           "Offload not supported when conform action is ok, "
                           "but action is not last");
        return -EOPNOTSUPP;
    }

    if (act->police.rate_bytes_ps || act->police.peakrate_bytes_ps ||
        act->police.avrate || act->police.overhead) {
        NL_SET_ERR_MSG_MOD(extack,
                           "Offload not supported when bytes per "
                           "second/peakrate/avrate/overhead is configured");
        return -EOPNOTSUPP;
    }

    return 0;
}

static int cpsw_qos_configure_clsflower(struct cpsw_priv* priv,
                                        struct flow_cls_offload* cls) {
    struct flow_rule* rule = flow_cls_offload_flow_rule(cls);
    struct netlink_ext_ack* extack = cls->common.extack;
    const struct flow_action_entry* act;
    int i, ret;

    flow_action_for_each(i, act, &rule->action) {
        switch (act->id) {
            case FLOW_ACTION_POLICE:
                ret = cpsw_qos_clsflower_policer_validate(&rule->action, act,
                                                          extack);
                if (ret)
                    return ret;

                return cpsw_qos_clsflower_add_policer(priv, extack, cls,
                                                      act->police.rate_pkt_ps);
            default:
                NL_SET_ERR_MSG_MOD(extack, "Action not supported");
                return -EOPNOTSUPP;
        }
    }
    return -EOPNOTSUPP;
}

static int cpsw_qos_delete_clsflower(struct cpsw_priv* priv,
                                     struct flow_cls_offload* cls) {
    u32 port_id = cpsw_slave_index(priv->cpsw, priv) + 1;

    if (cls->cookie == priv->ale_bc_ratelimit.cookie) {
        priv->ale_bc_ratelimit.cookie = 0;
        priv->ale_bc_ratelimit.rate_packet_ps = 0;
        cpsw_ale_rx_ratelimit_bc(priv->cpsw->ale, port_id, 0);
    }

    if (cls->cookie == priv->ale_mc_ratelimit.cookie) {
        priv->ale_mc_ratelimit.cookie = 0;
        priv->ale_mc_ratelimit.rate_packet_ps = 0;
        cpsw_ale_rx_ratelimit_mc(priv->cpsw->ale, port_id, 0);
    }

    return 0;
}

static int cpsw_qos_setup_tc_clsflower(struct cpsw_priv* priv,
                                       struct flow_cls_offload* cls_flower) {
    switch (cls_flower->command) {
        case FLOW_CLS_REPLACE:
            return cpsw_qos_configure_clsflower(priv, cls_flower);
        case FLOW_CLS_DESTROY:
            return cpsw_qos_delete_clsflower(priv, cls_flower);
        default:
            return -EOPNOTSUPP;
    }
}

static int cpsw_qos_setup_tc_block_cb(enum tc_setup_type type,
                                      void* type_data,
                                      void* cb_priv) {
    struct cpsw_priv* priv = cb_priv;
    int ret;

    if (!tc_cls_can_offload_and_chain0(priv->ndev, type_data))
        return -EOPNOTSUPP;

    ret = pm_runtime_get_sync(priv->dev);
    if (ret < 0) {
        pm_runtime_put_noidle(priv->dev);
        return ret;
    }

    switch (type) {
        case TC_SETUP_CLSFLOWER:
            ret = cpsw_qos_setup_tc_clsflower(priv, type_data);
            break;
        default:
            ret = -EOPNOTSUPP;
    }

    pm_runtime_put(priv->dev);
    return ret;
}

static LIST_HEAD(cpsw_qos_block_cb_list);

static int cpsw_qos_setup_tc_block(struct net_device* ndev,
                                   struct flow_block_offload* f) {
    struct cpsw_priv* priv = netdev_priv(ndev);

    return flow_block_cb_setup_simple(f, &cpsw_qos_block_cb_list,
                                      cpsw_qos_setup_tc_block_cb, priv, priv,
                                      true);
}

void cpsw_qos_clsflower_resume(struct cpsw_priv* priv) {
    u32 port_id = cpsw_slave_index(priv->cpsw, priv) + 1;

    if (priv->ale_bc_ratelimit.cookie)
        cpsw_ale_rx_ratelimit_bc(priv->cpsw->ale, port_id,
                                 priv->ale_bc_ratelimit.rate_packet_ps);

    if (priv->ale_mc_ratelimit.cookie)
        cpsw_ale_rx_ratelimit_mc(priv->cpsw->ale, port_id,
                                 priv->ale_mc_ratelimit.rate_packet_ps);
}
// SPDX-License-Identifier: GPL-2.0
/*
 * Texas Instruments Ethernet Switch media-access-controller (MAC) submodule/
 * Ethernet MAC Sliver (CPGMAC_SL)
 *
 * Copyright (C) 2019 Texas Instruments
 *
 */

static const u16 cpsw_sl_reg_map_cpsw[] = {
    [CPSW_SL_IDVER] = 0x00,     [CPSW_SL_MACCONTROL] = 0x04,
    [CPSW_SL_MACSTATUS] = 0x08, [CPSW_SL_SOFT_RESET] = 0x0c,
    [CPSW_SL_RX_MAXLEN] = 0x10, [CPSW_SL_BOFFTEST] = 0x14,
    [CPSW_SL_RX_PAUSE] = 0x18,  [CPSW_SL_TX_PAUSE] = 0x1c,
    [CPSW_SL_EMCONTROL] = 0x20, [CPSW_SL_RX_PRI_MAP] = 0x24,
    [CPSW_SL_TX_GAP] = 0x28,
};

static const u16 cpsw_sl_reg_map_66ak2hk[] = {
    [CPSW_SL_IDVER] = 0x00,
    [CPSW_SL_MACCONTROL] = 0x04,
    [CPSW_SL_MACSTATUS] = 0x08,
    [CPSW_SL_SOFT_RESET] = 0x0c,
    [CPSW_SL_RX_MAXLEN] = 0x10,
    [CPSW_SL_BOFFTEST] = CPSW_SL_REG_NOTUSED,
    [CPSW_SL_RX_PAUSE] = 0x18,
    [CPSW_SL_TX_PAUSE] = 0x1c,
    [CPSW_SL_EMCONTROL] = 0x20,
    [CPSW_SL_RX_PRI_MAP] = 0x24,
    [CPSW_SL_TX_GAP] = CPSW_SL_REG_NOTUSED,
};

static const u16 cpsw_sl_reg_map_66ak2x_xgbe[] = {
    [CPSW_SL_IDVER] = 0x00,     [CPSW_SL_MACCONTROL] = 0x04,
    [CPSW_SL_MACSTATUS] = 0x08, [CPSW_SL_SOFT_RESET] = 0x0c,
    [CPSW_SL_RX_MAXLEN] = 0x10, [CPSW_SL_BOFFTEST] = CPSW_SL_REG_NOTUSED,
    [CPSW_SL_RX_PAUSE] = 0x18,  [CPSW_SL_TX_PAUSE] = 0x1c,
    [CPSW_SL_EMCONTROL] = 0x20, [CPSW_SL_RX_PRI_MAP] = CPSW_SL_REG_NOTUSED,
    [CPSW_SL_TX_GAP] = 0x28,
};

static const u16 cpsw_sl_reg_map_66ak2elg_am65[] = {
    [CPSW_SL_IDVER] = CPSW_SL_REG_NOTUSED,
    [CPSW_SL_MACCONTROL] = 0x00,
    [CPSW_SL_MACSTATUS] = 0x04,
    [CPSW_SL_SOFT_RESET] = 0x08,
    [CPSW_SL_RX_MAXLEN] = CPSW_SL_REG_NOTUSED,
    [CPSW_SL_BOFFTEST] = 0x0c,
    [CPSW_SL_RX_PAUSE] = 0x10,
    [CPSW_SL_TX_PAUSE] = 0x40,
    [CPSW_SL_EMCONTROL] = 0x70,
    [CPSW_SL_RX_PRI_MAP] = CPSW_SL_REG_NOTUSED,
    [CPSW_SL_TX_GAP] = 0x74,
};

struct cpsw_sl {
    struct device* dev;
    void __iomem* sl_base;
    const u16* regs;
    u32 control_features;
    u32 idle_mask;
};

struct cpsw_sl_dev_id {
    const char* device_id;
    const u16* regs;
    const u32 control_features;
    const u32 regs_offset;
    const u32 idle_mask;
};

static const struct cpsw_sl_dev_id cpsw_sl_id_match[] = {
    {
        .device_id = "cpsw",
        .regs = cpsw_sl_reg_map_cpsw,
        .control_features = CPSW_SL_CTL_FUNC_BASE | CPSW_SL_CTL_MTEST |
                            CPSW_SL_CTL_TX_SHORT_GAP_EN |
                            CPSW_SL_CTL_TX_SG_LIM_EN,
        .idle_mask = CPSW_SL_STATUS_IDLE_MASK_BASE,
    },
    {
        .device_id = "66ak2hk",
        .regs = cpsw_sl_reg_map_66ak2hk,
        .control_features = CPSW_SL_CTL_FUNC_BASE | CPSW_SL_CTL_TX_SHORT_GAP_EN,
        .idle_mask = CPSW_SL_STATUS_IDLE_MASK_BASE,
    },
    {
        .device_id = "66ak2x_xgbe",
        .regs = cpsw_sl_reg_map_66ak2x_xgbe,
        .control_features = CPSW_SL_CTL_FUNC_BASE | CPSW_SL_CTL_XGIG |
                            CPSW_SL_CTL_TX_SHORT_GAP_EN | CPSW_SL_CTL_CRC_TYPE |
                            CPSW_SL_CTL_XGMII_EN,
        .idle_mask = CPSW_SL_STATUS_IDLE_MASK_BASE,
    },
    {
        .device_id = "66ak2el",
        .regs = cpsw_sl_reg_map_66ak2elg_am65,
        .regs_offset = 0x330,
        .control_features = CPSW_SL_CTL_FUNC_BASE | CPSW_SL_CTL_MTEST |
                            CPSW_SL_CTL_TX_SHORT_GAP_EN | CPSW_SL_CTL_CRC_TYPE |
                            CPSW_SL_CTL_EXT_EN_RX_FLO |
                            CPSW_SL_CTL_EXT_EN_TX_FLO |
                            CPSW_SL_CTL_TX_SG_LIM_EN,
        .idle_mask = CPSW_SL_STATUS_IDLE_MASK_BASE,
    },
    {
        .device_id = "66ak2g",
        .regs = cpsw_sl_reg_map_66ak2elg_am65,
        .regs_offset = 0x330,
        .control_features = CPSW_SL_CTL_FUNC_BASE | CPSW_SL_CTL_MTEST |
                            CPSW_SL_CTL_CRC_TYPE | CPSW_SL_CTL_EXT_EN_RX_FLO |
                            CPSW_SL_CTL_EXT_EN_TX_FLO,
    },
    {
        .device_id = "am65",
        .regs = cpsw_sl_reg_map_66ak2elg_am65,
        .regs_offset = 0x330,
        .control_features = CPSW_SL_CTL_FUNC_BASE | CPSW_SL_CTL_MTEST |
                            CPSW_SL_CTL_XGIG | CPSW_SL_CTL_TX_SHORT_GAP_EN |
                            CPSW_SL_CTL_CRC_TYPE | CPSW_SL_CTL_XGMII_EN |
                            CPSW_SL_CTL_EXT_EN_RX_FLO |
                            CPSW_SL_CTL_EXT_EN_TX_FLO |
                            CPSW_SL_CTL_TX_SG_LIM_EN | CPSW_SL_CTL_EXT_EN_XGIG,
        .idle_mask = CPSW_SL_STATUS_IDLE_MASK_K3,
    },
    {},
};

u32 cpsw_sl_reg_read(struct cpsw_sl* sl, enum cpsw_sl_regs reg) {
    int val;

    if (sl->regs[reg] == CPSW_SL_REG_NOTUSED) {
        dev_err(sl->dev, "cpsw_sl: not sup r reg: %04X\n", sl->regs[reg]);
        return 0;
    }

    val = readl(sl->sl_base + sl->regs[reg]);
    dev_dbg(sl->dev, "cpsw_sl: reg: %04X r 0x%08X\n", sl->regs[reg], val);
    return val;
}

void cpsw_sl_reg_write(struct cpsw_sl* sl, enum cpsw_sl_regs reg, u32 val) {
    if (sl->regs[reg] == CPSW_SL_REG_NOTUSED) {
        dev_err(sl->dev, "cpsw_sl: not sup w reg: %04X\n", sl->regs[reg]);
        return;
    }

    dev_dbg(sl->dev, "cpsw_sl: reg: %04X w 0x%08X\n", sl->regs[reg], val);
    writel(val, sl->sl_base + sl->regs[reg]);
}

static const struct cpsw_sl_dev_id* cpsw_sl_match_id(
    const struct cpsw_sl_dev_id* id,
    const char* device_id) {
    if (!id || !device_id)
        return NULL;

    while (id->device_id) {
        if (strcmp(device_id, id->device_id) == 0)
            return id;
        id++;
    }
    return NULL;
}

struct cpsw_sl* cpsw_sl_get(const char* device_id,
                            struct device* dev,
                            void __iomem* sl_base) {
    const struct cpsw_sl_dev_id* sl_dev_id;
    struct cpsw_sl* sl;

    sl = devm_kzalloc(dev, sizeof(struct cpsw_sl), GFP_KERNEL);
    if (!sl)
        return ERR_PTR(-ENOMEM);
    sl->dev = dev;
    sl->sl_base = sl_base;

    sl_dev_id = cpsw_sl_match_id(cpsw_sl_id_match, device_id);
    if (!sl_dev_id) {
        dev_err(sl->dev, "cpsw_sl: dev_id %s not found.\n", device_id);
        return ERR_PTR(-EINVAL);
    }
    sl->regs = sl_dev_id->regs;
    sl->control_features = sl_dev_id->control_features;
    sl->idle_mask = sl_dev_id->idle_mask;
    sl->sl_base += sl_dev_id->regs_offset;

    return sl;
}

void cpsw_sl_reset(struct cpsw_sl* sl, unsigned long tmo) {
    unsigned long timeout = jiffies + msecs_to_jiffies(tmo);

    /* Set the soft reset bit */
    cpsw_sl_reg_write(sl, CPSW_SL_SOFT_RESET, CPSW_SL_SOFT_RESET_BIT);

    /* Wait for the bit to clear */
    do {
        usleep_range(100, 200);
    } while (
        (cpsw_sl_reg_read(sl, CPSW_SL_SOFT_RESET) & CPSW_SL_SOFT_RESET_BIT) &&
        time_after(timeout, jiffies));

    if (cpsw_sl_reg_read(sl, CPSW_SL_SOFT_RESET) & CPSW_SL_SOFT_RESET_BIT)
        dev_err(sl->dev, "cpsw_sl failed to soft-reset.\n");
}

u32 cpsw_sl_ctl_set(struct cpsw_sl* sl, u32 ctl_funcs) {
    u32 val;

    if (ctl_funcs & ~sl->control_features) {
        dev_err(sl->dev, "cpsw_sl: unsupported func 0x%08X\n",
                ctl_funcs & (~sl->control_features));
        return -EINVAL;
    }

    val = cpsw_sl_reg_read(sl, CPSW_SL_MACCONTROL);
    val |= ctl_funcs;
    cpsw_sl_reg_write(sl, CPSW_SL_MACCONTROL, val);

    return 0;
}

u32 cpsw_sl_ctl_clr(struct cpsw_sl* sl, u32 ctl_funcs) {
    u32 val;

    if (ctl_funcs & ~sl->control_features) {
        dev_err(sl->dev, "cpsw_sl: unsupported func 0x%08X\n",
                ctl_funcs & (~sl->control_features));
        return -EINVAL;
    }

    val = cpsw_sl_reg_read(sl, CPSW_SL_MACCONTROL);
    val &= ~ctl_funcs;
    cpsw_sl_reg_write(sl, CPSW_SL_MACCONTROL, val);

    return 0;
}

void cpsw_sl_ctl_reset(struct cpsw_sl* sl) {
    cpsw_sl_reg_write(sl, CPSW_SL_MACCONTROL, 0);
}

int cpsw_sl_wait_for_idle(struct cpsw_sl* sl, unsigned long tmo) {
    unsigned long timeout = jiffies + msecs_to_jiffies(tmo);

    do {
        usleep_range(100, 200);
    } while (!(cpsw_sl_reg_read(sl, CPSW_SL_MACSTATUS) & sl->idle_mask) &&
             time_after(timeout, jiffies));

    if (!(cpsw_sl_reg_read(sl, CPSW_SL_MACSTATUS) & sl->idle_mask)) {
        dev_err(sl->dev, "cpsw_sl failed to soft-reset.\n");
        return -ETIMEDOUT;
    }

    return 0;
}

struct cpsw_switchdev_event_work {
    struct work_struct work;
    struct switchdev_notifier_fdb_info fdb_info;
    struct cpsw_priv* priv;
    unsigned long event;
};

static int cpsw_port_stp_state_set(struct cpsw_priv* priv, u8 state) {
    struct cpsw_common* cpsw = priv->cpsw;
    u8 cpsw_state;
    int ret = 0;

    switch (state) {
        case BR_STATE_FORWARDING:
            cpsw_state = ALE_PORT_STATE_FORWARD;
            break;
        case BR_STATE_LEARNING:
            cpsw_state = ALE_PORT_STATE_LEARN;
            break;
        case BR_STATE_DISABLED:
            cpsw_state = ALE_PORT_STATE_DISABLE;
            break;
        case BR_STATE_LISTENING:
        case BR_STATE_BLOCKING:
            cpsw_state = ALE_PORT_STATE_BLOCK;
            break;
        default:
            return -EOPNOTSUPP;
    }

    ret = cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_STATE,
                               cpsw_state);
    dev_dbg(priv->dev, "ale state: %u\n", cpsw_state);

    return ret;
}

static int cpsw_port_attr_br_flags_set(struct cpsw_priv* priv,
                                       struct net_device* orig_dev,
                                       struct switchdev_brport_flags flags) {
    struct cpsw_common* cpsw = priv->cpsw;

    if (flags.mask & BR_MCAST_FLOOD) {
        bool unreg_mcast_add = false;

        if (flags.val & BR_MCAST_FLOOD)
            unreg_mcast_add = true;

        dev_dbg(priv->dev, "BR_MCAST_FLOOD: %d port %u\n", unreg_mcast_add,
                priv->emac_port);

        cpsw_ale_set_unreg_mcast(cpsw->ale, BIT(priv->emac_port),
                                 unreg_mcast_add);
    }

    return 0;
}

static int cpsw_port_attr_br_flags_pre_set(
    struct net_device* netdev,
    struct switchdev_brport_flags flags) {
    if (flags.mask & ~(BR_LEARNING | BR_MCAST_FLOOD))
        return -EINVAL;

    return 0;
}

static int cpsw_port_attr_set(struct net_device* ndev,
                              const void* ctx,
                              const struct switchdev_attr* attr,
                              struct netlink_ext_ack* extack) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    int ret;

    dev_dbg(priv->dev, "attr: id %u port: %u\n", attr->id, priv->emac_port);

    switch (attr->id) {
        case SWITCHDEV_ATTR_ID_PORT_PRE_BRIDGE_FLAGS:
            ret = cpsw_port_attr_br_flags_pre_set(ndev, attr->u.brport_flags);
            break;
        case SWITCHDEV_ATTR_ID_PORT_STP_STATE:
            ret = cpsw_port_stp_state_set(priv, attr->u.stp_state);
            dev_dbg(priv->dev, "stp state: %u\n", attr->u.stp_state);
            break;
        case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
            ret = cpsw_port_attr_br_flags_set(priv, attr->orig_dev,
                                              attr->u.brport_flags);
            break;
        default:
            ret = -EOPNOTSUPP;
            break;
    }

    return ret;
}

static u16 cpsw_get_pvid(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    u32 __iomem* port_vlan_reg;
    u32 pvid;

    if (priv->emac_port) {
        int reg = CPSW2_PORT_VLAN;

        if (cpsw->version == CPSW_VERSION_1)
            reg = CPSW1_PORT_VLAN;
        pvid = slave_read(cpsw->slaves + (priv->emac_port - 1), reg);
    } else {
        port_vlan_reg = &cpsw->host_port_regs->port_vlan;
        pvid = readl(port_vlan_reg);
    }

    pvid = pvid & 0xfff;

    return pvid;
}

static void cpsw_set_pvid(struct cpsw_priv* priv, u16 vid, bool cfi, u32 cos) {
    struct cpsw_common* cpsw = priv->cpsw;
    void __iomem* port_vlan_reg;
    u32 pvid;

    pvid = vid;
    pvid |= cfi ? BIT(12) : 0;
    pvid |= (cos & 0x7) << 13;

    if (priv->emac_port) {
        int reg = CPSW2_PORT_VLAN;

        if (cpsw->version == CPSW_VERSION_1)
            reg = CPSW1_PORT_VLAN;
        /* no barrier */
        slave_write(cpsw->slaves + (priv->emac_port - 1), pvid, reg);
    } else {
        /* CPU port */
        port_vlan_reg = &cpsw->host_port_regs->port_vlan;
        writel(pvid, port_vlan_reg);
    }
}

static int cpsw_port_vlan_add(struct cpsw_priv* priv,
                              bool untag,
                              bool pvid,
                              u16 vid,
                              struct net_device* orig_dev) {
    bool cpu_port = netif_is_bridge_master(orig_dev);
    struct cpsw_common* cpsw = priv->cpsw;
    int unreg_mcast_mask = 0;
    int reg_mcast_mask = 0;
    int untag_mask = 0;
    int port_mask;
    int ret = 0;
    u32 flags;

    if (cpu_port) {
        port_mask = BIT(HOST_PORT_NUM);
        flags = orig_dev->flags;
        unreg_mcast_mask = port_mask;
    } else {
        port_mask = BIT(priv->emac_port);
        flags = priv->ndev->flags;
    }

    if (flags & IFF_MULTICAST)
        reg_mcast_mask = port_mask;

    if (untag)
        untag_mask = port_mask;

    ret = cpsw_ale_vlan_add_modify(cpsw->ale, vid, port_mask, untag_mask,
                                   reg_mcast_mask, unreg_mcast_mask);
    if (ret) {
        dev_err(priv->dev, "Unable to add vlan\n");
        return ret;
    }

    if (cpu_port)
        cpsw_ale_add_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM, ALE_VLAN,
                           vid);
    if (!pvid)
        return ret;

    cpsw_set_pvid(priv, vid, 0, 0);

    dev_dbg(priv->dev, "VID add: %s: vid:%u ports:%X\n", priv->ndev->name, vid,
            port_mask);
    return ret;
}

static int cpsw_port_vlan_del(struct cpsw_priv* priv,
                              u16 vid,
                              struct net_device* orig_dev) {
    bool cpu_port = netif_is_bridge_master(orig_dev);
    struct cpsw_common* cpsw = priv->cpsw;
    int port_mask;
    int ret = 0;

    if (cpu_port)
        port_mask = BIT(HOST_PORT_NUM);
    else
        port_mask = BIT(priv->emac_port);

    ret = cpsw_ale_vlan_del_modify(cpsw->ale, vid, port_mask);
    if (ret != 0)
        return ret;

    /* We don't care for the return value here, error is returned only if
     * the unicast entry is not present
     */
    if (cpu_port)
        cpsw_ale_del_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM, ALE_VLAN,
                           vid);

    if (vid == cpsw_get_pvid(priv))
        cpsw_set_pvid(priv, 0, 0, 0);

    /* We don't care for the return value here, error is returned only if
     * the multicast entry is not present
     */
    cpsw_ale_del_mcast(cpsw->ale, priv->ndev->broadcast, port_mask, ALE_VLAN,
                       vid);
    dev_dbg(priv->dev, "VID del: %s: vid:%u ports:%X\n", priv->ndev->name, vid,
            port_mask);

    return ret;
}

static int cpsw_port_vlans_add(struct cpsw_priv* priv,
                               const struct switchdev_obj_port_vlan* vlan) {
    bool untag = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    struct net_device* orig_dev = vlan->obj.orig_dev;
    bool pvid = vlan->flags & BRIDGE_VLAN_INFO_PVID;

    dev_dbg(priv->dev, "VID add: %s: vid:%u flags:%X\n", priv->ndev->name,
            vlan->vid, vlan->flags);

    return cpsw_port_vlan_add(priv, untag, pvid, vlan->vid, orig_dev);
}

static int cpsw_port_mdb_add(struct cpsw_priv* priv,
                             struct switchdev_obj_port_mdb* mdb)

{
    struct net_device* orig_dev = mdb->obj.orig_dev;
    bool cpu_port = netif_is_bridge_master(orig_dev);
    struct cpsw_common* cpsw = priv->cpsw;
    int port_mask;
    int err;

    if (cpu_port)
        port_mask = BIT(HOST_PORT_NUM);
    else
        port_mask = BIT(priv->emac_port);

    err = cpsw_ale_add_mcast(cpsw->ale, mdb->addr, port_mask, ALE_VLAN,
                             mdb->vid, 0);
    dev_dbg(priv->dev, "MDB add: %s: vid %u:%pM  ports: %X\n", priv->ndev->name,
            mdb->vid, mdb->addr, port_mask);

    return err;
}

static int cpsw_port_mdb_del(struct cpsw_priv* priv,
                             struct switchdev_obj_port_mdb* mdb)

{
    struct net_device* orig_dev = mdb->obj.orig_dev;
    bool cpu_port = netif_is_bridge_master(orig_dev);
    struct cpsw_common* cpsw = priv->cpsw;
    int del_mask;
    int err;

    if (cpu_port)
        del_mask = BIT(HOST_PORT_NUM);
    else
        del_mask = BIT(priv->emac_port);

    err =
        cpsw_ale_del_mcast(cpsw->ale, mdb->addr, del_mask, ALE_VLAN, mdb->vid);
    dev_dbg(priv->dev, "MDB del: %s: vid %u:%pM  ports: %X\n", priv->ndev->name,
            mdb->vid, mdb->addr, del_mask);

    return err;
}

static int cpsw_port_obj_add(struct net_device* ndev,
                             const void* ctx,
                             const struct switchdev_obj* obj,
                             struct netlink_ext_ack* extack) {
    struct switchdev_obj_port_vlan* vlan = SWITCHDEV_OBJ_PORT_VLAN(obj);
    struct switchdev_obj_port_mdb* mdb = SWITCHDEV_OBJ_PORT_MDB(obj);
    struct cpsw_priv* priv = netdev_priv(ndev);
    int err = 0;

    dev_dbg(priv->dev, "obj_add: id %u port: %u\n", obj->id, priv->emac_port);

    switch (obj->id) {
        case SWITCHDEV_OBJ_ID_PORT_VLAN:
            err = cpsw_port_vlans_add(priv, vlan);
            break;
        case SWITCHDEV_OBJ_ID_PORT_MDB:
        case SWITCHDEV_OBJ_ID_HOST_MDB:
            err = cpsw_port_mdb_add(priv, mdb);
            break;
        default:
            err = -EOPNOTSUPP;
            break;
    }

    return err;
}

static int cpsw_port_obj_del(struct net_device* ndev,
                             const void* ctx,
                             const struct switchdev_obj* obj) {
    struct switchdev_obj_port_vlan* vlan = SWITCHDEV_OBJ_PORT_VLAN(obj);
    struct switchdev_obj_port_mdb* mdb = SWITCHDEV_OBJ_PORT_MDB(obj);
    struct cpsw_priv* priv = netdev_priv(ndev);
    int err = 0;

    dev_dbg(priv->dev, "obj_del: id %u port: %u\n", obj->id, priv->emac_port);

    switch (obj->id) {
        case SWITCHDEV_OBJ_ID_PORT_VLAN:
            err = cpsw_port_vlan_del(priv, vlan->vid, vlan->obj.orig_dev);
            break;
        case SWITCHDEV_OBJ_ID_PORT_MDB:
        case SWITCHDEV_OBJ_ID_HOST_MDB:
            err = cpsw_port_mdb_del(priv, mdb);
            break;
        default:
            err = -EOPNOTSUPP;
            break;
    }

    return err;
}

static void cpsw_fdb_offload_notify(struct net_device* ndev,
                                    struct switchdev_notifier_fdb_info* rcv) {
    struct switchdev_notifier_fdb_info info = {};

    info.addr = rcv->addr;
    info.vid = rcv->vid;
    info.offloaded = true;
    call_switchdev_notifiers(SWITCHDEV_FDB_OFFLOADED, ndev, &info.info, NULL);
}

static void cpsw_switchdev_event_work(struct work_struct* work) {
    struct cpsw_switchdev_event_work* switchdev_work =
        container_of(work, struct cpsw_switchdev_event_work, work);
    struct cpsw_priv* priv = switchdev_work->priv;
    struct switchdev_notifier_fdb_info* fdb;
    struct cpsw_common* cpsw = priv->cpsw;
    int port = priv->emac_port;

    rtnl_lock();
    switch (switchdev_work->event) {
        case SWITCHDEV_FDB_ADD_TO_DEVICE:
            fdb = &switchdev_work->fdb_info;

            dev_dbg(
                cpsw->dev,
                "cpsw_fdb_add: MACID = %pM vid = %u flags = %u %u -- port %d\n",
                fdb->addr, fdb->vid, fdb->added_by_user, fdb->offloaded, port);

            if (!fdb->added_by_user || fdb->is_local)
                break;
            if (memcmp(priv->mac_addr, (u8*)fdb->addr, ETH_ALEN) == 0)
                port = HOST_PORT_NUM;

            cpsw_ale_add_ucast(cpsw->ale, (u8*)fdb->addr, port,
                               fdb->vid ? ALE_VLAN : 0, fdb->vid);
            cpsw_fdb_offload_notify(priv->ndev, fdb);
            break;
        case SWITCHDEV_FDB_DEL_TO_DEVICE:
            fdb = &switchdev_work->fdb_info;

            dev_dbg(
                cpsw->dev,
                "cpsw_fdb_del: MACID = %pM vid = %u flags = %u %u -- port %d\n",
                fdb->addr, fdb->vid, fdb->added_by_user, fdb->offloaded, port);

            if (!fdb->added_by_user || fdb->is_local)
                break;
            if (memcmp(priv->mac_addr, (u8*)fdb->addr, ETH_ALEN) == 0)
                port = HOST_PORT_NUM;

            cpsw_ale_del_ucast(cpsw->ale, (u8*)fdb->addr, port,
                               fdb->vid ? ALE_VLAN : 0, fdb->vid);
            break;
        default:
            break;
    }
    rtnl_unlock();

    kfree(switchdev_work->fdb_info.addr);
    kfree(switchdev_work);
    dev_put(priv->ndev);
}

/* called under rcu_read_lock() */
static int cpsw_switchdev_event(struct notifier_block* unused,
                                unsigned long event,
                                void* ptr) {
    struct net_device* ndev = switchdev_notifier_info_to_dev(ptr);
    struct switchdev_notifier_fdb_info* fdb_info = ptr;
    struct cpsw_switchdev_event_work* switchdev_work;
    struct cpsw_priv* priv = netdev_priv(ndev);
    int err;

    if (event == SWITCHDEV_PORT_ATTR_SET) {
        err = switchdev_handle_port_attr_set(ndev, ptr, cpsw_port_dev_check,
                                             cpsw_port_attr_set);
        return notifier_from_errno(err);
    }

    if (!cpsw_port_dev_check(ndev))
        return NOTIFY_DONE;

    switchdev_work = kzalloc(sizeof(*switchdev_work), GFP_ATOMIC);
    if (WARN_ON(!switchdev_work))
        return NOTIFY_BAD;

    INIT_WORK(&switchdev_work->work, cpsw_switchdev_event_work);
    switchdev_work->priv = priv;
    switchdev_work->event = event;

    switch (event) {
        case SWITCHDEV_FDB_ADD_TO_DEVICE:
        case SWITCHDEV_FDB_DEL_TO_DEVICE:
            memcpy(&switchdev_work->fdb_info, ptr,
                   sizeof(switchdev_work->fdb_info));
            switchdev_work->fdb_info.addr = kzalloc(ETH_ALEN, GFP_ATOMIC);
            if (!switchdev_work->fdb_info.addr)
                goto err_addr_alloc;
            ether_addr_copy((u8*)switchdev_work->fdb_info.addr, fdb_info->addr);
            dev_hold(ndev);
            break;
        default:
            kfree(switchdev_work);
            return NOTIFY_DONE;
    }

    queue_work(system_long_wq, &switchdev_work->work);

    return NOTIFY_DONE;

err_addr_alloc:
    kfree(switchdev_work);
    return NOTIFY_BAD;
}

static struct notifier_block cpsw_switchdev_notifier = {
    .notifier_call = cpsw_switchdev_event,
};

static int cpsw_switchdev_blocking_event(struct notifier_block* unused,
                                         unsigned long event,
                                         void* ptr) {
    struct net_device* dev = switchdev_notifier_info_to_dev(ptr);
    int err;

    switch (event) {
        case SWITCHDEV_PORT_OBJ_ADD:
            err = switchdev_handle_port_obj_add(dev, ptr, cpsw_port_dev_check,
                                                cpsw_port_obj_add);
            return notifier_from_errno(err);
        case SWITCHDEV_PORT_OBJ_DEL:
            err = switchdev_handle_port_obj_del(dev, ptr, cpsw_port_dev_check,
                                                cpsw_port_obj_del);
            return notifier_from_errno(err);
        case SWITCHDEV_PORT_ATTR_SET:
            err = switchdev_handle_port_attr_set(dev, ptr, cpsw_port_dev_check,
                                                 cpsw_port_attr_set);
            return notifier_from_errno(err);
        default:
            break;
    }

    return NOTIFY_DONE;
}

static struct notifier_block cpsw_switchdev_bl_notifier = {
    .notifier_call = cpsw_switchdev_blocking_event,
};

int cpsw_switchdev_register_notifiers(struct cpsw_common* cpsw) {
    int ret = 0;

    ret = register_switchdev_notifier(&cpsw_switchdev_notifier);
    if (ret) {
        dev_err(cpsw->dev, "register switchdev notifier fail ret:%d\n", ret);
        return ret;
    }

    ret = register_switchdev_blocking_notifier(&cpsw_switchdev_bl_notifier);
    if (ret) {
        dev_err(cpsw->dev, "register switchdev blocking notifier ret:%d\n",
                ret);
        unregister_switchdev_notifier(&cpsw_switchdev_notifier);
    }

    return ret;
}

void cpsw_switchdev_unregister_notifiers(struct cpsw_common* cpsw) {
    unregister_switchdev_blocking_notifier(&cpsw_switchdev_bl_notifier);
    unregister_switchdev_notifier(&cpsw_switchdev_notifier);
}

struct cpdma_desc {
    /* hardware fields */
    u32 hw_next;
    u32 hw_buffer;
    u32 hw_len;
    u32 hw_mode;
    /* software fields */
    void* sw_token;
    u32 sw_buffer;
    u32 sw_len;
};

struct cpdma_desc_pool {
    phys_addr_t phys;
    dma_addr_t hw_addr;
    void __iomem* iomap; /* ioremap map */
    void* cpumap;        /* dma_alloc map */
    int desc_size, mem_size;
    int num_desc;
    struct device* dev;
    struct gen_pool* gen_pool;
};

enum cpdma_state {
    CPDMA_STATE_IDLE,
    CPDMA_STATE_ACTIVE,
    CPDMA_STATE_TEARDOWN,
};

struct cpdma_ctlr {
    enum cpdma_state state;
    struct cpdma_params params;
    struct device* dev;
    struct cpdma_desc_pool* pool;
    spinlock_t lock;
    struct cpdma_chan* channels[2 * CPDMA_MAX_CHANNELS];
    int chan_num;
    int num_rx_desc; /* RX descriptors number */
    int num_tx_desc; /* TX descriptors number */
};

struct cpdma_chan {
    struct cpdma_desc __iomem *head, *tail;
    void __iomem *hdp, *cp, *rxfree;
    enum cpdma_state state;
    struct cpdma_ctlr* ctlr;
    int chan_num;
    spinlock_t lock;
    int count;
    u32 desc_num;
    u32 mask;
    cpdma_handler_fn handler;
    enum dma_data_direction dir;
    struct cpdma_chan_stats stats;
    /* offsets into dmaregs */
    int int_set, int_clear, td;
    int weight;
    u32 rate_factor;
    u32 rate;
};

struct cpdma_control_info {
    u32 reg;
    u32 shift, mask;
    int access;
#define ACCESS_RO BIT(0)
#define ACCESS_WO BIT(1)
#define ACCESS_RW (ACCESS_RO | ACCESS_WO)
};

struct submit_info {
    struct cpdma_chan* chan;
    int directed;
    void* token;
    void* data_virt;
    dma_addr_t data_dma;
    int len;
};

static struct cpdma_control_info controls[] = {
    [CPDMA_TX_RLIM] = {CPDMA_DMACONTROL, 8, 0xffff, ACCESS_RW},
    [CPDMA_CMD_IDLE] = {CPDMA_DMACONTROL, 3, 1, ACCESS_WO},
    [CPDMA_COPY_ERROR_FRAMES] = {CPDMA_DMACONTROL, 4, 1, ACCESS_RW},
    [CPDMA_RX_OFF_LEN_UPDATE] = {CPDMA_DMACONTROL, 2, 1, ACCESS_RW},
    [CPDMA_RX_OWNERSHIP_FLIP] = {CPDMA_DMACONTROL, 1, 1, ACCESS_RW},
    [CPDMA_TX_PRIO_FIXED] = {CPDMA_DMACONTROL, 0, 1, ACCESS_RW},
    [CPDMA_STAT_IDLE] = {CPDMA_DMASTATUS, 31, 1, ACCESS_RO},
    [CPDMA_STAT_TX_ERR_CODE] = {CPDMA_DMASTATUS, 20, 0xf, ACCESS_RW},
    [CPDMA_STAT_TX_ERR_CHAN] = {CPDMA_DMASTATUS, 16, 0x7, ACCESS_RW},
    [CPDMA_STAT_RX_ERR_CODE] = {CPDMA_DMASTATUS, 12, 0xf, ACCESS_RW},
    [CPDMA_STAT_RX_ERR_CHAN] = {CPDMA_DMASTATUS, 8, 0x7, ACCESS_RW},
    [CPDMA_RX_BUFFER_OFFSET] = {CPDMA_RXBUFFOFS, 0, 0xffff, ACCESS_RW},
};

#define tx_chan_num(chan) (chan)
#define rx_chan_num(chan) ((chan) + CPDMA_MAX_CHANNELS)
#define is_rx_chan(chan) ((chan)->chan_num >= CPDMA_MAX_CHANNELS)
#define is_tx_chan(chan) (!is_rx_chan(chan))
#define __chan_linear(chan_num) ((chan_num) & (CPDMA_MAX_CHANNELS - 1))
#define chan_linear(chan) __chan_linear((chan)->chan_num)

/* The following make access to common cpdma_ctlr params more readable */
#define dmaregs params.dmaregs
#define num_chan params.num_chan

/* various accessors */
#define dma_reg_read(ctlr, ofs) readl((ctlr)->dmaregs + (ofs))
#define chan_read(chan, fld) readl((chan)->fld)
#define desc_read(desc, fld) readl(&(desc)->fld)
#define dma_reg_write(ctlr, ofs, v) writel(v, (ctlr)->dmaregs + (ofs))
#define chan_write(chan, fld, v) writel(v, (chan)->fld)
#define desc_write(desc, fld, v) writel((u32)(v), &(desc)->fld)

#define cpdma_desc_to_port(chan, mode, directed)                             \
    do {                                                                     \
        if (!is_rx_chan(chan) && ((directed == 1) || (directed == 2)))       \
            mode |=                                                          \
                (CPDMA_DESC_TO_PORT_EN | (directed << CPDMA_TO_PORT_SHIFT)); \
    } while (0)

#define CPDMA_DMA_EXT_MAP BIT(16)

static void cpdma_desc_pool_destroy(struct cpdma_ctlr* ctlr) {
    struct cpdma_desc_pool* pool = ctlr->pool;

    if (!pool)
        return;

    WARN(gen_pool_size(pool->gen_pool) != gen_pool_avail(pool->gen_pool),
         "cpdma_desc_pool size %zd != avail %zd", gen_pool_size(pool->gen_pool),
         gen_pool_avail(pool->gen_pool));
    if (pool->cpumap)
        dma_free_coherent(ctlr->dev, pool->mem_size, pool->cpumap, pool->phys);
}

/*
 * Utility constructs for a cpdma descriptor pool.  Some devices (e.g. davinci
 * emac) have dedicated on-chip memory for these descriptors.  Some other
 * devices (e.g. cpsw switches) use plain old memory.  Descriptor pools
 * abstract out these details
 */
static int cpdma_desc_pool_create(struct cpdma_ctlr* ctlr) {
    struct cpdma_params* cpdma_params = &ctlr->params;
    struct cpdma_desc_pool* pool;
    int ret = -ENOMEM;

    pool = devm_kzalloc(ctlr->dev, sizeof(*pool), GFP_KERNEL);
    if (!pool)
        goto gen_pool_create_fail;
    ctlr->pool = pool;

    pool->mem_size = cpdma_params->desc_mem_size;
    pool->desc_size =
        ALIGN(sizeof(struct cpdma_desc), cpdma_params->desc_align);
    pool->num_desc = pool->mem_size / pool->desc_size;

    if (cpdma_params->descs_pool_size) {
        /* recalculate memory size required cpdma descriptor pool
         * basing on number of descriptors specified by user and
         * if memory size > CPPI internal RAM size (desc_mem_size)
         * then switch to use DDR
         */
        pool->num_desc = cpdma_params->descs_pool_size;
        pool->mem_size = pool->desc_size * pool->num_desc;
        if (pool->mem_size > cpdma_params->desc_mem_size)
            cpdma_params->desc_mem_phys = 0;
    }

    pool->gen_pool =
        devm_gen_pool_create(ctlr->dev, ilog2(pool->desc_size), -1, "cpdma");
    if (IS_ERR(pool->gen_pool)) {
        ret = PTR_ERR(pool->gen_pool);
        dev_err(ctlr->dev, "pool create failed %d\n", ret);
        goto gen_pool_create_fail;
    }

    if (cpdma_params->desc_mem_phys) {
        pool->phys = cpdma_params->desc_mem_phys;
        pool->iomap = devm_ioremap(ctlr->dev, pool->phys, pool->mem_size);
        pool->hw_addr = cpdma_params->desc_hw_addr;
    } else {
        pool->cpumap = dma_alloc_coherent(ctlr->dev, pool->mem_size,
                                          &pool->hw_addr, GFP_KERNEL);
        pool->iomap = (void __iomem __force*)pool->cpumap;
        pool->phys = pool->hw_addr; /* assumes no IOMMU, don't use this value */
    }

    if (!pool->iomap)
        goto gen_pool_create_fail;

    ret = gen_pool_add_virt(pool->gen_pool, (unsigned long)pool->iomap,
                            pool->phys, pool->mem_size, -1);
    if (ret < 0) {
        dev_err(ctlr->dev, "pool add failed %d\n", ret);
        goto gen_pool_add_virt_fail;
    }

    return 0;

gen_pool_add_virt_fail:
    cpdma_desc_pool_destroy(ctlr);
gen_pool_create_fail:
    ctlr->pool = NULL;
    return ret;
}

static inline dma_addr_t desc_phys(struct cpdma_desc_pool* pool,
                                   struct cpdma_desc __iomem* desc) {
    if (!desc)
        return 0;
    return pool->hw_addr + (__force long)desc - (__force long)pool->iomap;
}

static inline struct cpdma_desc __iomem* desc_from_phys(
    struct cpdma_desc_pool* pool,
    dma_addr_t dma) {
    return dma ? pool->iomap + dma - pool->hw_addr : NULL;
}

static struct cpdma_desc __iomem* cpdma_desc_alloc(
    struct cpdma_desc_pool* pool) {
    return (struct cpdma_desc __iomem*)gen_pool_alloc(pool->gen_pool,
                                                      pool->desc_size);
}

static void cpdma_desc_free(struct cpdma_desc_pool* pool,
                            struct cpdma_desc __iomem* desc,
                            int num_desc) {
    gen_pool_free(pool->gen_pool, (unsigned long)desc, pool->desc_size);
}

static int _cpdma_control_set(struct cpdma_ctlr* ctlr, int control, int value) {
    struct cpdma_control_info* info = &controls[control];
    u32 val;

    if (!ctlr->params.has_ext_regs)
        return -ENOTSUPP;

    if (ctlr->state != CPDMA_STATE_ACTIVE)
        return -EINVAL;

    if (control < 0 || control >= ARRAY_SIZE(controls))
        return -ENOENT;

    if ((info->access & ACCESS_WO) != ACCESS_WO)
        return -EPERM;

    val = dma_reg_read(ctlr, info->reg);
    val &= ~(info->mask << info->shift);
    val |= (value & info->mask) << info->shift;
    dma_reg_write(ctlr, info->reg, val);

    return 0;
}

static int _cpdma_control_get(struct cpdma_ctlr* ctlr, int control) {
    struct cpdma_control_info* info = &controls[control];
    int ret;

    if (!ctlr->params.has_ext_regs)
        return -ENOTSUPP;

    if (ctlr->state != CPDMA_STATE_ACTIVE)
        return -EINVAL;

    if (control < 0 || control >= ARRAY_SIZE(controls))
        return -ENOENT;

    if ((info->access & ACCESS_RO) != ACCESS_RO)
        return -EPERM;

    ret = (dma_reg_read(ctlr, info->reg) >> info->shift) & info->mask;
    return ret;
}

/* cpdma_chan_set_chan_shaper - set shaper for a channel
 * Has to be called under ctlr lock
 */
static int cpdma_chan_set_chan_shaper(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    u32 rate_reg;
    u32 rmask;
    int ret;

    if (!chan->rate)
        return 0;

    rate_reg = CPDMA_TX_PRI0_RATE + 4 * chan->chan_num;
    dma_reg_write(ctlr, rate_reg, chan->rate_factor);

    rmask = _cpdma_control_get(ctlr, CPDMA_TX_RLIM);
    rmask |= chan->mask;

    ret = _cpdma_control_set(ctlr, CPDMA_TX_RLIM, rmask);
    return ret;
}

static int cpdma_chan_on(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    struct cpdma_desc_pool* pool = ctlr->pool;
    unsigned long flags;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state != CPDMA_STATE_IDLE) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EBUSY;
    }
    if (ctlr->state != CPDMA_STATE_ACTIVE) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }
    dma_reg_write(ctlr, chan->int_set, chan->mask);
    chan->state = CPDMA_STATE_ACTIVE;
    if (chan->head) {
        chan_write(chan, hdp, desc_phys(pool, chan->head));
        if (chan->rxfree)
            chan_write(chan, rxfree, chan->count);
    }

    spin_unlock_irqrestore(&chan->lock, flags);
    return 0;
}

/* cpdma_chan_fit_rate - set rate for a channel and check if it's possible.
 * rmask - mask of rate limited channels
 * Returns min rate in Kb/s
 */
static int cpdma_chan_fit_rate(struct cpdma_chan* ch,
                               u32 rate,
                               u32* rmask,
                               int* prio_mode) {
    struct cpdma_ctlr* ctlr = ch->ctlr;
    struct cpdma_chan* chan;
    u32 old_rate = ch->rate;
    u32 new_rmask = 0;
    int rlim = 0;
    int i;

    for (i = tx_chan_num(0); i < tx_chan_num(CPDMA_MAX_CHANNELS); i++) {
        chan = ctlr->channels[i];
        if (!chan)
            continue;

        if (chan == ch)
            chan->rate = rate;

        if (chan->rate) {
            rlim = 1;
            new_rmask |= chan->mask;
            continue;
        }

        if (rlim)
            goto err;
    }

    *rmask = new_rmask;
    *prio_mode = rlim;
    return 0;

err:
    ch->rate = old_rate;
    dev_err(ctlr->dev, "Upper cpdma ch%d is not rate limited\n",
            chan->chan_num);
    return -EINVAL;
}

static u32 cpdma_chan_set_factors(struct cpdma_ctlr* ctlr,
                                  struct cpdma_chan* ch) {
    u32 delta = UINT_MAX, prev_delta = UINT_MAX, best_delta = UINT_MAX;
    u32 best_send_cnt = 0, best_idle_cnt = 0;
    u32 new_rate, best_rate = 0, rate_reg;
    u64 send_cnt, idle_cnt;
    u32 min_send_cnt, freq;
    u64 divident, divisor;

    if (!ch->rate) {
        ch->rate_factor = 0;
        goto set_factor;
    }

    freq = ctlr->params.bus_freq_mhz * 1000 * 32;
    if (!freq) {
        dev_err(ctlr->dev, "The bus frequency is not set\n");
        return -EINVAL;
    }

    min_send_cnt = freq - ch->rate;
    send_cnt = DIV_ROUND_UP(min_send_cnt, ch->rate);
    while (send_cnt <= CPDMA_MAX_RLIM_CNT) {
        divident = ch->rate * send_cnt;
        divisor = min_send_cnt;
        idle_cnt = DIV_ROUND_CLOSEST_ULL(divident, divisor);

        divident = freq * idle_cnt;
        divisor = idle_cnt + send_cnt;
        new_rate = DIV_ROUND_CLOSEST_ULL(divident, divisor);

        delta = new_rate >= ch->rate ? new_rate - ch->rate : delta;
        if (delta < best_delta) {
            best_delta = delta;
            best_send_cnt = send_cnt;
            best_idle_cnt = idle_cnt;
            best_rate = new_rate;

            if (!delta)
                break;
        }

        if (prev_delta >= delta) {
            prev_delta = delta;
            send_cnt++;
            continue;
        }

        idle_cnt++;
        divident = freq * idle_cnt;
        send_cnt = DIV_ROUND_CLOSEST_ULL(divident, ch->rate);
        send_cnt -= idle_cnt;
        prev_delta = UINT_MAX;
    }

    ch->rate = best_rate;
    ch->rate_factor = best_send_cnt | (best_idle_cnt << 16);

set_factor:
    rate_reg = CPDMA_TX_PRI0_RATE + 4 * ch->chan_num;
    dma_reg_write(ctlr, rate_reg, ch->rate_factor);
    return 0;
}

struct cpdma_ctlr* cpdma_ctlr_create(struct cpdma_params* params) {
    struct cpdma_ctlr* ctlr;
    ctlr = devm_kzalloc(params->dev, sizeof(*ctlr), GFP_KERNEL);
    if (!ctlr)
        return NULL;

    ctlr->state = CPDMA_STATE_IDLE;
    ctlr->params = *params;
    ctlr->dev = params->dev;
    ctlr->chan_num = 0;
    spin_lock_init(&ctlr->lock);

    if (cpdma_desc_pool_create(ctlr))
        return NULL;
    /* split pool equally between RX/TX by default */
    ctlr->num_tx_desc = ctlr->pool->num_desc / 2;
    ctlr->num_rx_desc = ctlr->pool->num_desc - ctlr->num_tx_desc;

    if (WARN_ON(ctlr->num_chan > CPDMA_MAX_CHANNELS))
        ctlr->num_chan = CPDMA_MAX_CHANNELS;
    return ctlr;
}

int cpdma_ctlr_start(struct cpdma_ctlr* ctlr) {
    struct cpdma_chan* chan;
    unsigned long flags;
    int i, prio_mode;

    spin_lock_irqsave(&ctlr->lock, flags);
    if (ctlr->state != CPDMA_STATE_IDLE) {
        spin_unlock_irqrestore(&ctlr->lock, flags);
        return -EBUSY;
    }

    if (ctlr->params.has_soft_reset) {
        unsigned timeout = 10 * 100;

        dma_reg_write(ctlr, CPDMA_SOFTRESET, 1);
        while (timeout) {
            if (dma_reg_read(ctlr, CPDMA_SOFTRESET) == 0)
                break;
            udelay(10);
            timeout--;
        }
        WARN_ON(!timeout);
    }

    for (i = 0; i < ctlr->num_chan; i++) {
        writel(0, ctlr->params.txhdp + 4 * i);
        writel(0, ctlr->params.rxhdp + 4 * i);
        writel(0, ctlr->params.txcp + 4 * i);
        writel(0, ctlr->params.rxcp + 4 * i);
    }

    dma_reg_write(ctlr, CPDMA_RXINTMASKCLEAR, 0xffffffff);
    dma_reg_write(ctlr, CPDMA_TXINTMASKCLEAR, 0xffffffff);

    dma_reg_write(ctlr, CPDMA_TXCONTROL, 1);
    dma_reg_write(ctlr, CPDMA_RXCONTROL, 1);

    ctlr->state = CPDMA_STATE_ACTIVE;

    prio_mode = 0;
    for (i = 0; i < ARRAY_SIZE(ctlr->channels); i++) {
        chan = ctlr->channels[i];
        if (chan) {
            cpdma_chan_set_chan_shaper(chan);
            cpdma_chan_on(chan);

            /* off prio mode if all tx channels are rate limited */
            if (is_tx_chan(chan) && !chan->rate)
                prio_mode = 1;
        }
    }

    _cpdma_control_set(ctlr, CPDMA_TX_PRIO_FIXED, prio_mode);
    _cpdma_control_set(ctlr, CPDMA_RX_BUFFER_OFFSET, 0);

    spin_unlock_irqrestore(&ctlr->lock, flags);
    return 0;
}

int cpdma_ctlr_stop(struct cpdma_ctlr* ctlr) {
    unsigned long flags;
    int i;

    spin_lock_irqsave(&ctlr->lock, flags);
    if (ctlr->state != CPDMA_STATE_ACTIVE) {
        spin_unlock_irqrestore(&ctlr->lock, flags);
        return -EINVAL;
    }

    ctlr->state = CPDMA_STATE_TEARDOWN;
    spin_unlock_irqrestore(&ctlr->lock, flags);

    for (i = 0; i < ARRAY_SIZE(ctlr->channels); i++) {
        if (ctlr->channels[i])
            cpdma_chan_stop(ctlr->channels[i]);
    }

    spin_lock_irqsave(&ctlr->lock, flags);
    dma_reg_write(ctlr, CPDMA_RXINTMASKCLEAR, 0xffffffff);
    dma_reg_write(ctlr, CPDMA_TXINTMASKCLEAR, 0xffffffff);

    dma_reg_write(ctlr, CPDMA_TXCONTROL, 0);
    dma_reg_write(ctlr, CPDMA_RXCONTROL, 0);

    ctlr->state = CPDMA_STATE_IDLE;

    spin_unlock_irqrestore(&ctlr->lock, flags);
    return 0;
}

int cpdma_ctlr_destroy(struct cpdma_ctlr* ctlr) {
    int ret = 0, i;

    if (!ctlr)
        return -EINVAL;

    if (ctlr->state != CPDMA_STATE_IDLE)
        cpdma_ctlr_stop(ctlr);

    for (i = 0; i < ARRAY_SIZE(ctlr->channels); i++)
        cpdma_chan_destroy(ctlr->channels[i]);

    cpdma_desc_pool_destroy(ctlr);
    return ret;
}

int cpdma_ctlr_int_ctrl(struct cpdma_ctlr* ctlr, bool enable) {
    unsigned long flags;
    int i;

    spin_lock_irqsave(&ctlr->lock, flags);
    if (ctlr->state != CPDMA_STATE_ACTIVE) {
        spin_unlock_irqrestore(&ctlr->lock, flags);
        return -EINVAL;
    }

    for (i = 0; i < ARRAY_SIZE(ctlr->channels); i++) {
        if (ctlr->channels[i])
            cpdma_chan_int_ctrl(ctlr->channels[i], enable);
    }

    spin_unlock_irqrestore(&ctlr->lock, flags);
    return 0;
}

void cpdma_ctlr_eoi(struct cpdma_ctlr* ctlr, u32 value) {
    dma_reg_write(ctlr, CPDMA_MACEOIVECTOR, value);
}

u32 cpdma_ctrl_rxchs_state(struct cpdma_ctlr* ctlr) {
    return dma_reg_read(ctlr, CPDMA_RXINTSTATMASKED);
}

u32 cpdma_ctrl_txchs_state(struct cpdma_ctlr* ctlr) {
    return dma_reg_read(ctlr, CPDMA_TXINTSTATMASKED);
}

static void cpdma_chan_set_descs(struct cpdma_ctlr* ctlr,
                                 int rx,
                                 int desc_num,
                                 int per_ch_desc) {
    struct cpdma_chan *chan, *most_chan = NULL;
    int desc_cnt = desc_num;
    int most_dnum = 0;
    int min, max, i;

    if (!desc_num)
        return;

    if (rx) {
        min = rx_chan_num(0);
        max = rx_chan_num(CPDMA_MAX_CHANNELS);
    } else {
        min = tx_chan_num(0);
        max = tx_chan_num(CPDMA_MAX_CHANNELS);
    }

    for (i = min; i < max; i++) {
        chan = ctlr->channels[i];
        if (!chan)
            continue;

        if (chan->weight)
            chan->desc_num = (chan->weight * desc_num) / 100;
        else
            chan->desc_num = per_ch_desc;

        desc_cnt -= chan->desc_num;

        if (most_dnum < chan->desc_num) {
            most_dnum = chan->desc_num;
            most_chan = chan;
        }
    }
    /* use remains */
    if (most_chan)
        most_chan->desc_num += desc_cnt;
}

/*
 * cpdma_chan_split_pool - Splits ctrl pool between all channels.
 * Has to be called under ctlr lock
 */
static int cpdma_chan_split_pool(struct cpdma_ctlr* ctlr) {
    int tx_per_ch_desc = 0, rx_per_ch_desc = 0;
    int free_rx_num = 0, free_tx_num = 0;
    int rx_weight = 0, tx_weight = 0;
    int tx_desc_num, rx_desc_num;
    struct cpdma_chan* chan;
    int i;

    if (!ctlr->chan_num)
        return 0;

    for (i = 0; i < ARRAY_SIZE(ctlr->channels); i++) {
        chan = ctlr->channels[i];
        if (!chan)
            continue;

        if (is_rx_chan(chan)) {
            if (!chan->weight)
                free_rx_num++;
            rx_weight += chan->weight;
        } else {
            if (!chan->weight)
                free_tx_num++;
            tx_weight += chan->weight;
        }
    }

    if (rx_weight > 100 || tx_weight > 100)
        return -EINVAL;

    tx_desc_num = ctlr->num_tx_desc;
    rx_desc_num = ctlr->num_rx_desc;

    if (free_tx_num) {
        tx_per_ch_desc = tx_desc_num - (tx_weight * tx_desc_num) / 100;
        tx_per_ch_desc /= free_tx_num;
    }
    if (free_rx_num) {
        rx_per_ch_desc = rx_desc_num - (rx_weight * rx_desc_num) / 100;
        rx_per_ch_desc /= free_rx_num;
    }

    cpdma_chan_set_descs(ctlr, 0, tx_desc_num, tx_per_ch_desc);
    cpdma_chan_set_descs(ctlr, 1, rx_desc_num, rx_per_ch_desc);

    return 0;
}

/* cpdma_chan_set_weight - set weight of a channel in percentage.
 * Tx and Rx channels have separate weights. That is 100% for RX
 * and 100% for Tx. The weight is used to split cpdma resources
 * in correct proportion required by the channels, including number
 * of descriptors. The channel rate is not enough to know the
 * weight of a channel as the maximum rate of an interface is needed.
 * If weight = 0, then channel uses rest of descriptors leaved by
 * weighted channels.
 */
int cpdma_chan_set_weight(struct cpdma_chan* ch, int weight) {
    struct cpdma_ctlr* ctlr = ch->ctlr;
    unsigned long flags, ch_flags;
    int ret;

    spin_lock_irqsave(&ctlr->lock, flags);
    spin_lock_irqsave(&ch->lock, ch_flags);
    if (ch->weight == weight) {
        spin_unlock_irqrestore(&ch->lock, ch_flags);
        spin_unlock_irqrestore(&ctlr->lock, flags);
        return 0;
    }
    ch->weight = weight;
    spin_unlock_irqrestore(&ch->lock, ch_flags);

    /* re-split pool using new channel weight */
    ret = cpdma_chan_split_pool(ctlr);
    spin_unlock_irqrestore(&ctlr->lock, flags);
    return ret;
}

/* cpdma_chan_get_min_rate - get minimum allowed rate for channel
 * Should be called before cpdma_chan_set_rate.
 * Returns min rate in Kb/s
 */
u32 cpdma_chan_get_min_rate(struct cpdma_ctlr* ctlr) {
    unsigned int divident, divisor;

    divident = ctlr->params.bus_freq_mhz * 32 * 1000;
    divisor = 1 + CPDMA_MAX_RLIM_CNT;

    return DIV_ROUND_UP(divident, divisor);
}

/* cpdma_chan_set_rate - limits bandwidth for transmit channel.
 * The bandwidth * limited channels have to be in order beginning from lowest.
 * ch - transmit channel the bandwidth is configured for
 * rate - bandwidth in Kb/s, if 0 - then off shaper
 */
int cpdma_chan_set_rate(struct cpdma_chan* ch, u32 rate) {
    unsigned long flags, ch_flags;
    struct cpdma_ctlr* ctlr;
    int ret, prio_mode;
    u32 rmask;

    if (!ch || !is_tx_chan(ch))
        return -EINVAL;

    if (ch->rate == rate)
        return rate;

    ctlr = ch->ctlr;
    spin_lock_irqsave(&ctlr->lock, flags);
    spin_lock_irqsave(&ch->lock, ch_flags);

    ret = cpdma_chan_fit_rate(ch, rate, &rmask, &prio_mode);
    if (ret)
        goto err;

    ret = cpdma_chan_set_factors(ctlr, ch);
    if (ret)
        goto err;

    spin_unlock_irqrestore(&ch->lock, ch_flags);

    /* on shapers */
    _cpdma_control_set(ctlr, CPDMA_TX_RLIM, rmask);
    _cpdma_control_set(ctlr, CPDMA_TX_PRIO_FIXED, prio_mode);
    spin_unlock_irqrestore(&ctlr->lock, flags);
    return ret;

err:
    spin_unlock_irqrestore(&ch->lock, ch_flags);
    spin_unlock_irqrestore(&ctlr->lock, flags);
    return ret;
}

u32 cpdma_chan_get_rate(struct cpdma_chan* ch) {
    unsigned long flags;
    u32 rate;

    spin_lock_irqsave(&ch->lock, flags);
    rate = ch->rate;
    spin_unlock_irqrestore(&ch->lock, flags);

    return rate;
}

struct cpdma_chan* cpdma_chan_create(struct cpdma_ctlr* ctlr,
                                     int chan_num,
                                     cpdma_handler_fn handler,
                                     int rx_type) {
    int offset = chan_num * 4;
    struct cpdma_chan* chan;
    unsigned long flags;

    chan_num = rx_type ? rx_chan_num(chan_num) : tx_chan_num(chan_num);

    if (__chan_linear(chan_num) >= ctlr->num_chan)
        return ERR_PTR(-EINVAL);

    chan = devm_kzalloc(ctlr->dev, sizeof(*chan), GFP_KERNEL);
    if (!chan)
        return ERR_PTR(-ENOMEM);

    spin_lock_irqsave(&ctlr->lock, flags);
    if (ctlr->channels[chan_num]) {
        spin_unlock_irqrestore(&ctlr->lock, flags);
        devm_kfree(ctlr->dev, chan);
        return ERR_PTR(-EBUSY);
    }

    chan->ctlr = ctlr;
    chan->state = CPDMA_STATE_IDLE;
    chan->chan_num = chan_num;
    chan->handler = handler;
    chan->rate = 0;
    chan->weight = 0;

    if (is_rx_chan(chan)) {
        chan->hdp = ctlr->params.rxhdp + offset;
        chan->cp = ctlr->params.rxcp + offset;
        chan->rxfree = ctlr->params.rxfree + offset;
        chan->int_set = CPDMA_RXINTMASKSET;
        chan->int_clear = CPDMA_RXINTMASKCLEAR;
        chan->td = CPDMA_RXTEARDOWN;
        chan->dir = DMA_FROM_DEVICE;
    } else {
        chan->hdp = ctlr->params.txhdp + offset;
        chan->cp = ctlr->params.txcp + offset;
        chan->int_set = CPDMA_TXINTMASKSET;
        chan->int_clear = CPDMA_TXINTMASKCLEAR;
        chan->td = CPDMA_TXTEARDOWN;
        chan->dir = DMA_TO_DEVICE;
    }
    chan->mask = BIT(chan_linear(chan));

    spin_lock_init(&chan->lock);

    ctlr->channels[chan_num] = chan;
    ctlr->chan_num++;

    cpdma_chan_split_pool(ctlr);

    spin_unlock_irqrestore(&ctlr->lock, flags);
    return chan;
}

int cpdma_chan_get_rx_buf_num(struct cpdma_chan* chan) {
    unsigned long flags;
    int desc_num;

    spin_lock_irqsave(&chan->lock, flags);
    desc_num = chan->desc_num;
    spin_unlock_irqrestore(&chan->lock, flags);

    return desc_num;
}

int cpdma_chan_destroy(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr;
    unsigned long flags;

    if (!chan)
        return -EINVAL;
    ctlr = chan->ctlr;

    spin_lock_irqsave(&ctlr->lock, flags);
    if (chan->state != CPDMA_STATE_IDLE)
        cpdma_chan_stop(chan);
    ctlr->channels[chan->chan_num] = NULL;
    ctlr->chan_num--;
    devm_kfree(ctlr->dev, chan);
    cpdma_chan_split_pool(ctlr);

    spin_unlock_irqrestore(&ctlr->lock, flags);
    return 0;
}

int cpdma_chan_get_stats(struct cpdma_chan* chan,
                         struct cpdma_chan_stats* stats) {
    unsigned long flags;
    if (!chan)
        return -EINVAL;
    spin_lock_irqsave(&chan->lock, flags);
    memcpy(stats, &chan->stats, sizeof(*stats));
    spin_unlock_irqrestore(&chan->lock, flags);
    return 0;
}

static void __cpdma_chan_submit(struct cpdma_chan* chan,
                                struct cpdma_desc __iomem* desc) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    struct cpdma_desc __iomem* prev = chan->tail;
    struct cpdma_desc_pool* pool = ctlr->pool;
    dma_addr_t desc_dma;
    u32 mode;

    desc_dma = desc_phys(pool, desc);

    /* simple case - idle channel */
    if (!chan->head) {
        chan->stats.head_enqueue++;
        chan->head = desc;
        chan->tail = desc;
        if (chan->state == CPDMA_STATE_ACTIVE)
            chan_write(chan, hdp, desc_dma);
        return;
    }

    /* first chain the descriptor at the tail of the list */
    desc_write(prev, hw_next, desc_dma);
    chan->tail = desc;
    chan->stats.tail_enqueue++;

    /* next check if EOQ has been triggered already */
    mode = desc_read(prev, hw_mode);
    if (((mode & (CPDMA_DESC_EOQ | CPDMA_DESC_OWNER)) == CPDMA_DESC_EOQ) &&
        (chan->state == CPDMA_STATE_ACTIVE)) {
        desc_write(prev, hw_mode, mode & ~CPDMA_DESC_EOQ);
        chan_write(chan, hdp, desc_dma);
        chan->stats.misqueued++;
    }
}

static int cpdma_chan_submit_si(struct submit_info* si) {
    struct cpdma_chan* chan = si->chan;
    struct cpdma_ctlr* ctlr = chan->ctlr;
    int len = si->len;
    struct cpdma_desc __iomem* desc;
    dma_addr_t buffer;
    u32 mode;
    int ret;
    // pr_info("Debug1\n");
    if (chan->count >= chan->desc_num) {
        chan->stats.desc_alloc_fail++;
        return -ENOMEM;
    }

    // pr_info("Debug2\n");
    desc = cpdma_desc_alloc(ctlr->pool);
    if (!desc) {
        chan->stats.desc_alloc_fail++;
        return -ENOMEM;
    }

    // pr_info("Debug3\n");
    if (len < ctlr->params.min_packet_size) {
        len = ctlr->params.min_packet_size;
        chan->stats.runt_transmit_buff++;
    }
    // pr_info("Debug4\n");

    mode = CPDMA_DESC_OWNER | CPDMA_DESC_SOP | CPDMA_DESC_EOP;
    // pr_info("Debug5\n");
    cpdma_desc_to_port(chan, mode, si->directed);
    // pr_info("Debug6\n");

    if (si->data_dma) {
    // pr_info("Debug7\n");
        buffer = si->data_dma;
        dma_sync_single_for_device(ctlr->dev, buffer, len, chan->dir);
    } else {
    // pr_info("Debug8\n");
        buffer = dma_map_single(ctlr->dev, si->data_virt, len, chan->dir);
        ret = dma_mapping_error(ctlr->dev, buffer);
        if (ret) {
            cpdma_desc_free(ctlr->pool, desc, 1);
            return -EINVAL;
        }
    }
    // pr_info("Debug9\n");

    /* Relaxed IO accessors can be used here as there is read barrier
     * at the end of write sequence.
     */
    writel_relaxed(0, &desc->hw_next);
    // pr_info("Debug10\n");
    writel_relaxed(buffer, &desc->hw_buffer);
    // pr_info("Debug11\n");
    writel_relaxed(len, &desc->hw_len);
    // pr_info("Debug12\n");
    writel_relaxed(mode | len, &desc->hw_mode);
    // pr_info("Debug13\n");
    writel_relaxed((uintptr_t)si->token, &desc->sw_token);
    // pr_info("Debug14\n");
    writel_relaxed(buffer, &desc->sw_buffer);
    // pr_info("Debug15\n");
    writel_relaxed(si->data_dma ? len | CPDMA_DMA_EXT_MAP : len, &desc->sw_len);
    desc_read(desc, sw_len);

    __cpdma_chan_submit(chan, desc);

    if (chan->state == CPDMA_STATE_ACTIVE && chan->rxfree)
        chan_write(chan, rxfree, 1);

    chan->count++;
    return 0;
}

int cpdma_chan_idle_submit(struct cpdma_chan* chan,
                           void* token,
                           void* data,
                           int len,
                           int directed) {
    struct submit_info si;
    unsigned long flags;
    int ret;

    si.chan = chan;
    si.token = token;
    si.data_virt = data;
    si.data_dma = 0;
    si.len = len;
    si.directed = directed;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state == CPDMA_STATE_TEARDOWN) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }

    ret = cpdma_chan_submit_si(&si);
    spin_unlock_irqrestore(&chan->lock, flags);
    return ret;
}

int cpdma_chan_idle_submit_mapped(struct cpdma_chan* chan,
                                  void* token,
                                  dma_addr_t data,
                                  int len,
                                  int directed) {
    struct submit_info si;
    unsigned long flags;
    int ret;

    si.chan = chan;
    si.token = token;
    si.data_virt = NULL;
    si.data_dma = data;
    si.len = len;
    si.directed = directed;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state == CPDMA_STATE_TEARDOWN) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }

    ret = cpdma_chan_submit_si(&si);
    spin_unlock_irqrestore(&chan->lock, flags);
    return ret;
}

int cpdma_chan_submit(struct cpdma_chan* chan,
                      void* token,
                      void* data,
                      int len,
                      int directed) {
    struct submit_info si;
    unsigned long flags;
    int ret;

    si.chan = chan;
    si.token = token;
    si.data_virt = data;
    si.data_dma = 0;
    si.len = len;
    si.directed = directed;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state != CPDMA_STATE_ACTIVE) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }

    ret = cpdma_chan_submit_si(&si);
    spin_unlock_irqrestore(&chan->lock, flags);
    return ret;
}

int cpdma_chan_submit_mapped(struct cpdma_chan* chan,
                             void* token,
                             dma_addr_t data,
                             int len,
                             int directed) {
    struct submit_info si;
    unsigned long flags;
    int ret;

    si.chan = chan;
    si.token = token;
    si.data_virt = NULL;
    si.data_dma = data;
    si.len = len;
    si.directed = directed;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state != CPDMA_STATE_ACTIVE) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }

    ret = cpdma_chan_submit_si(&si);
    spin_unlock_irqrestore(&chan->lock, flags);
    return ret;
}

bool cpdma_check_free_tx_desc(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    struct cpdma_desc_pool* pool = ctlr->pool;
    bool free_tx_desc;
    unsigned long flags;

    spin_lock_irqsave(&chan->lock, flags);
    free_tx_desc =
        (chan->count < chan->desc_num) && gen_pool_avail(pool->gen_pool);
    spin_unlock_irqrestore(&chan->lock, flags);
    return free_tx_desc;
}

static void __cpdma_chan_free(struct cpdma_chan* chan,
                              struct cpdma_desc __iomem* desc,
                              int outlen,
                              int status) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    struct cpdma_desc_pool* pool = ctlr->pool;
    dma_addr_t buff_dma;
    int origlen;
    uintptr_t token;

    token = desc_read(desc, sw_token);
    origlen = desc_read(desc, sw_len);

    buff_dma = desc_read(desc, sw_buffer);
    if (origlen & CPDMA_DMA_EXT_MAP) {
        origlen &= ~CPDMA_DMA_EXT_MAP;
        dma_sync_single_for_cpu(ctlr->dev, buff_dma, origlen, chan->dir);
    } else {
        dma_unmap_single(ctlr->dev, buff_dma, origlen, chan->dir);
    }

    cpdma_desc_free(pool, desc, 1);
    (*chan->handler)((void*)token, outlen, status);
}

static int __cpdma_chan_process(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    struct cpdma_desc __iomem* desc;
    int status, outlen;
    int cb_status = 0;
    struct cpdma_desc_pool* pool = ctlr->pool;
    dma_addr_t desc_dma;
    unsigned long flags;

    spin_lock_irqsave(&chan->lock, flags);

    desc = chan->head;
    if (!desc) {
        chan->stats.empty_dequeue++;
        status = -ENOENT;
        goto unlock_ret;
    }
    desc_dma = desc_phys(pool, desc);

    status = desc_read(desc, hw_mode);
    outlen = status & 0x7ff;
    if (status & CPDMA_DESC_OWNER) {
        chan->stats.busy_dequeue++;
        status = -EBUSY;
        goto unlock_ret;
    }

    if (status & CPDMA_DESC_PASS_CRC)
        outlen -= CPDMA_DESC_CRC_LEN;

    status = status & (CPDMA_DESC_EOQ | CPDMA_DESC_TD_COMPLETE |
                       CPDMA_DESC_PORT_MASK | CPDMA_RX_VLAN_ENCAP);

    chan->head = desc_from_phys(pool, desc_read(desc, hw_next));
    chan_write(chan, cp, desc_dma);
    chan->count--;
    chan->stats.good_dequeue++;

    if ((status & CPDMA_DESC_EOQ) && chan->head) {
        chan->stats.requeue++;
        chan_write(chan, hdp, desc_phys(pool, chan->head));
    }

    spin_unlock_irqrestore(&chan->lock, flags);
    if (unlikely(status & CPDMA_DESC_TD_COMPLETE))
        cb_status = -ENOSYS;
    else
        cb_status = status;

    __cpdma_chan_free(chan, desc, outlen, cb_status);
    return status;

unlock_ret:
    spin_unlock_irqrestore(&chan->lock, flags);
    return status;
}

int cpdma_chan_process(struct cpdma_chan* chan, int quota) {
    int used = 0, ret = 0;

    if (chan->state != CPDMA_STATE_ACTIVE)
        return -EINVAL;

    while (used < quota) {
        ret = __cpdma_chan_process(chan);
        if (ret < 0)
            break;
        used++;
    }
    return used;
}

int cpdma_chan_start(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&ctlr->lock, flags);
    ret = cpdma_chan_set_chan_shaper(chan);
    spin_unlock_irqrestore(&ctlr->lock, flags);
    if (ret)
        return ret;

    ret = cpdma_chan_on(chan);
    if (ret)
        return ret;

    return 0;
}

int cpdma_chan_stop(struct cpdma_chan* chan) {
    struct cpdma_ctlr* ctlr = chan->ctlr;
    struct cpdma_desc_pool* pool = ctlr->pool;
    unsigned long flags;
    int ret;
    unsigned timeout;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state == CPDMA_STATE_TEARDOWN) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }

    chan->state = CPDMA_STATE_TEARDOWN;
    dma_reg_write(ctlr, chan->int_clear, chan->mask);

    /* trigger teardown */
    dma_reg_write(ctlr, chan->td, chan_linear(chan));

    /* wait for teardown complete */
    timeout = 100 * 100; /* 100 ms */
    while (timeout) {
        u32 cp = chan_read(chan, cp);
        if ((cp & CPDMA_TEARDOWN_VALUE) == CPDMA_TEARDOWN_VALUE)
            break;
        udelay(10);
        timeout--;
    }
    WARN_ON(!timeout);
    chan_write(chan, cp, CPDMA_TEARDOWN_VALUE);

    /* handle completed packets */
    spin_unlock_irqrestore(&chan->lock, flags);
    do {
        ret = __cpdma_chan_process(chan);
        if (ret < 0)
            break;
    } while ((ret & CPDMA_DESC_TD_COMPLETE) == 0);
    spin_lock_irqsave(&chan->lock, flags);

    /* remaining packets haven't been tx/rx'ed, clean them up */
    while (chan->head) {
        struct cpdma_desc __iomem* desc = chan->head;
        dma_addr_t next_dma;

        next_dma = desc_read(desc, hw_next);
        chan->head = desc_from_phys(pool, next_dma);
        chan->count--;
        chan->stats.teardown_dequeue++;

        /* issue callback without locks held */
        spin_unlock_irqrestore(&chan->lock, flags);
        __cpdma_chan_free(chan, desc, 0, -ENOSYS);
        spin_lock_irqsave(&chan->lock, flags);
    }

    chan->state = CPDMA_STATE_IDLE;
    spin_unlock_irqrestore(&chan->lock, flags);
    return 0;
}

int cpdma_chan_int_ctrl(struct cpdma_chan* chan, bool enable) {
    unsigned long flags;

    spin_lock_irqsave(&chan->lock, flags);
    if (chan->state != CPDMA_STATE_ACTIVE) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -EINVAL;
    }

    dma_reg_write(chan->ctlr, enable ? chan->int_set : chan->int_clear,
                  chan->mask);
    spin_unlock_irqrestore(&chan->lock, flags);

    return 0;
}

int cpdma_control_get(struct cpdma_ctlr* ctlr, int control) {
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&ctlr->lock, flags);
    ret = _cpdma_control_get(ctlr, control);
    spin_unlock_irqrestore(&ctlr->lock, flags);

    return ret;
}

int cpdma_control_set(struct cpdma_ctlr* ctlr, int control, int value) {
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&ctlr->lock, flags);
    ret = _cpdma_control_set(ctlr, control, value);
    spin_unlock_irqrestore(&ctlr->lock, flags);

    return ret;
}

int cpdma_get_num_rx_descs(struct cpdma_ctlr* ctlr) {
    return ctlr->num_rx_desc;
}

int cpdma_get_num_tx_descs(struct cpdma_ctlr* ctlr) {
    return ctlr->num_tx_desc;
}

int cpdma_set_num_rx_descs(struct cpdma_ctlr* ctlr, int num_rx_desc) {
    unsigned long flags;
    int temp, ret;

    spin_lock_irqsave(&ctlr->lock, flags);

    temp = ctlr->num_rx_desc;
    ctlr->num_rx_desc = num_rx_desc;
    ctlr->num_tx_desc = ctlr->pool->num_desc - ctlr->num_rx_desc;
    ret = cpdma_chan_split_pool(ctlr);
    if (ret) {
        ctlr->num_rx_desc = temp;
        ctlr->num_tx_desc = ctlr->pool->num_desc - ctlr->num_rx_desc;
    }

    spin_unlock_irqrestore(&ctlr->lock, flags);

    return ret;
}

// May not be required


struct cpsw_devlink {
    struct cpsw_common* cpsw;
};

enum cpsw_devlink_param_id {
    CPSW_DEVLINK_PARAM_ID_BASE = DEVLINK_PARAM_GENERIC_ID_MAX,
    CPSW_DL_PARAM_SWITCH_MODE,
    CPSW_DL_PARAM_ALE_BYPASS,
};

/* struct cpsw_common is not needed, kept here for compatibility
 * reasons witrh the old driver
 */
int cpsw_slave_index (struct cpsw_common* cpsw,
                                 struct cpsw_priv* priv) {
    if (priv->emac_port == HOST_PORT_NUM)
        return -1;

    return priv->emac_port - 1;
}

static bool cpsw_is_switch_en(struct cpsw_common* cpsw) {
    return !cpsw->data.dual_emac;
}

/**
 * cpsw_set_mc - adds multicast entry to the table if it's not added or deletes
 * if it's not deleted
 * @ndev: device to sync
 * @addr: address to be added or deleted
 * @vid: vlan id, if vid < 0 set/unset address for real device
 * @add: add address if the flag is set or remove otherwise
 */
static int cpsw_set_mc(struct net_device* ndev,
                       const u8* addr,
                       int vid,
                       int add) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int mask, flags, ret, slave_no;

    slave_no = cpsw_slave_index(cpsw, priv);
    if (vid < 0)
        vid = cpsw->slaves[slave_no].port_vlan;

    mask = ALE_PORT_HOST;
    flags = vid ? ALE_VLAN : 0;

    if (add)
        ret = cpsw_ale_add_mcast(cpsw->ale, addr, mask, flags, vid, 0);
    else
        ret = cpsw_ale_del_mcast(cpsw->ale, addr, 0, flags, vid);

    return ret;
}

static int cpsw_purge_vlan_mc(struct net_device* vdev, int vid, void* ctx) {
    struct addr_sync_ctx* sync_ctx = ctx;
    struct netdev_hw_addr* ha;
    int found = 0;

    if (!vdev || !(vdev->flags & IFF_UP))
        return 0;

    /* vlan address is relevant if its sync_cnt != 0 */
    netdev_for_each_mc_addr(ha, vdev) {
        if (ether_addr_equal(ha->addr, sync_ctx->addr)) {
            found = ha->sync_cnt;
            break;
        }
    }

    if (!found)
        return 0;

    sync_ctx->consumed++;
    cpsw_set_mc(sync_ctx->ndev, sync_ctx->addr, vid, 0);
    return 0;
}

static int cpsw_purge_all_mc(struct net_device* ndev, const u8* addr, int num) {
    struct addr_sync_ctx sync_ctx;

    sync_ctx.addr = addr;
    sync_ctx.ndev = ndev;
    sync_ctx.consumed = 0;

    vlan_for_each(ndev, cpsw_purge_vlan_mc, &sync_ctx);
    if (sync_ctx.consumed < num)
        cpsw_set_mc(ndev, addr, -1, 0);

    return 0;
}

static unsigned int cpsw_rxbuf_total_len(unsigned int len) {
    len += CPSW_HEADROOM_NA;
    len += SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

    return SKB_DATA_ALIGN(len);
}

void cpsw_rx_handler(void* token, int len, int status) {
    struct page *new_page, *page = token;
    void* pa = page_address(page);
    int headroom = CPSW_HEADROOM_NA;
    struct cpsw_meta_xdp* xmeta;
    struct cpsw_common* cpsw;
    struct net_device* ndev;
    int port, ch, pkt_size;
    struct cpsw_priv* priv;
    struct page_pool* pool;
    struct sk_buff* skb;
    struct xdp_buff xdp;
    int ret = 0;
    dma_addr_t dma;

    xmeta = pa + CPSW_XMETA_OFFSET;
    cpsw = ndev_to_cpsw(xmeta->ndev);
    ndev = xmeta->ndev;
    pkt_size = cpsw->rx_packet_max;
    ch = xmeta->ch;

    if (status >= 0) {
        port = CPDMA_RX_SOURCE_PORT(status);
        if (port)
            ndev = cpsw->slaves[--port].ndev;
    }

    priv = netdev_priv(ndev);
    pool = cpsw->page_pool[ch];

    if (unlikely(status < 0) || unlikely(!netif_running(ndev))) {
        /* In dual emac mode check for all interfaces */
        if (cpsw->usage_count && status >= 0) {
            /* The packet received is for the interface which
             * is already down and the other interface is up
             * and running, instead of freeing which results
             * in reducing of the number of rx descriptor in
             * DMA engine, requeue page back to cpdma.
             */
            new_page = page;
            goto requeue;
        }

        /* the interface is going down, pages are purged */
        page_pool_recycle_direct(pool, page);
        return;
    }

    new_page = page_pool_dev_alloc_pages(pool);
    if (unlikely(!new_page)) {
        new_page = page;
        ndev->stats.rx_dropped++;
        goto requeue;
    }

    if (priv->xdp_prog) {
        int size = len;

        xdp_init_buff(&xdp, PAGE_SIZE, &priv->xdp_rxq[ch]);
        if (status & CPDMA_RX_VLAN_ENCAP) {
            headroom += CPSW_RX_VLAN_ENCAP_HDR_SIZE;
            size -= CPSW_RX_VLAN_ENCAP_HDR_SIZE;
        }

        xdp_prepare_buff(&xdp, pa, headroom, size, false);

        ret = cpsw_run_xdp(priv, ch, &xdp, page, priv->emac_port, &len);
        if (ret != CPSW_XDP_PASS)
            goto requeue;

        headroom = xdp.data - xdp.data_hard_start;

        /* XDP prog can modify vlan tag, so can't use encap header */
        status &= ~CPDMA_RX_VLAN_ENCAP;
    }

    /* pass skb to netstack if no XDP prog or returned XDP_PASS */
    skb = build_skb(pa, cpsw_rxbuf_total_len(pkt_size));
    if (!skb) {
        ndev->stats.rx_dropped++;
        page_pool_recycle_direct(pool, page);
        goto requeue;
    }

    skb->offload_fwd_mark = priv->offload_fwd_mark;
    skb_reserve(skb, headroom);
    skb_put(skb, len);
    skb->dev = ndev;
    if (status & CPDMA_RX_VLAN_ENCAP)
        cpsw_rx_vlan_encap(skb);
    if (priv->rx_ts_enabled)
        cpts_rx_timestamp(cpsw->cpts, skb);
    skb->protocol = eth_type_trans(skb, ndev);

    /* mark skb for recycling */
    skb_mark_for_recycle(skb);
    netif_receive_skb(skb);

    ndev->stats.rx_bytes += len;
    ndev->stats.rx_packets++;

requeue:
    xmeta = page_address(new_page) + CPSW_XMETA_OFFSET;
    xmeta->ndev = ndev;
    xmeta->ch = ch;

    dma = page_pool_get_dma_addr(new_page) + CPSW_HEADROOM_NA;
    ret =
        cpdma_chan_submit_mapped(cpsw->rxv[ch].ch, new_page, dma, pkt_size, 0);
    if (ret < 0) {
        WARN_ON(ret == -ENOMEM);
        page_pool_recycle_direct(pool, new_page);
    }
}

static int cpsw_add_vlan_ale_entry(struct cpsw_priv* priv, unsigned short vid) {
    struct cpsw_common* cpsw = priv->cpsw;
    int unreg_mcast_mask = 0;
    int mcast_mask;
    u32 port_mask;
    int ret;

    port_mask = (1 << priv->emac_port) | ALE_PORT_HOST;

    mcast_mask = ALE_PORT_HOST;
    if (priv->ndev->flags & IFF_ALLMULTI)
        unreg_mcast_mask = mcast_mask;

    ret = cpsw_ale_add_vlan(cpsw->ale, vid, port_mask, 0, port_mask,
                            unreg_mcast_mask);
    if (ret != 0)
        return ret;

    ret = cpsw_ale_add_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM, ALE_VLAN,
                             vid);
    if (ret != 0)
        goto clean_vid;

    ret = cpsw_ale_add_mcast(cpsw->ale, priv->ndev->broadcast, mcast_mask,
                             ALE_VLAN, vid, 0);
    if (ret != 0)
        goto clean_vlan_ucast;
    return 0;

clean_vlan_ucast:
    cpsw_ale_del_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM, ALE_VLAN, vid);
clean_vid:
    cpsw_ale_del_vlan(cpsw->ale, vid, 0);
    return ret;
}

static int cpsw_ndo_vlan_rx_add_vid(struct net_device* ndev,
                                    __be16 proto,
                                    u16 vid) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int ret, i;

    if (cpsw_is_switch_en(cpsw)) {
        dev_dbg(cpsw->dev, ".ndo_vlan_rx_add_vid called in switch mode\n");
        return 0;
    }

    if (vid == cpsw->data.default_vlan)
        return 0;

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    /* In dual EMAC, reserved VLAN id should not be used for
     * creating VLAN interfaces as this can break the dual
     * EMAC port separation
     */
    for (i = 0; i < cpsw->data.slaves; i++) {
        if (cpsw->slaves[i].ndev && vid == cpsw->slaves[i].port_vlan) {
            ret = -EINVAL;
            goto err;
        }
    }

    dev_dbg(priv->dev, "Adding vlanid %d to vlan filter\n", vid);
    ret = cpsw_add_vlan_ale_entry(priv, vid);
err:
    pm_runtime_put(cpsw->dev);
    return ret;
}

static int cpsw_restore_vlans(struct net_device* vdev, int vid, void* arg) {
    struct cpsw_priv* priv = arg;

    if (!vdev || !vid)
        return 0;

    cpsw_ndo_vlan_rx_add_vid(priv->ndev, 0, vid);
    return 0;
}

/* restore resources after port reset */
static void cpsw_restore(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;

    /* restore vlan configurations */
    vlan_for_each(priv->ndev, cpsw_restore_vlans, priv);

    /* restore MQPRIO offload */
    cpsw_mqprio_resume(&cpsw->slaves[priv->emac_port - 1], priv);

    /* restore CBS offload */
    cpsw_cbs_resume(&cpsw->slaves[priv->emac_port - 1], priv);

    cpsw_qos_clsflower_resume(priv);
}

static void cpsw_init_stp_ale_entry(struct cpsw_common* cpsw) {
    static const char stpa[] = {0x01, 0x80, 0xc2, 0x0, 0x0, 0x0};

    cpsw_ale_add_mcast(cpsw->ale, stpa, ALE_PORT_HOST, ALE_SUPER, 0,
                       ALE_MCAST_BLOCK_LEARN_FWD);
}

static void cpsw_init_host_port_switch(struct cpsw_common* cpsw) {
    int vlan = cpsw->data.default_vlan;

    writel(CPSW_FIFO_NORMAL_MODE, &cpsw->host_port_regs->tx_in_ctl);

    writel(vlan, &cpsw->host_port_regs->port_vlan);

    cpsw_ale_add_vlan(cpsw->ale, vlan, ALE_ALL_PORTS, ALE_ALL_PORTS,
                      ALE_ALL_PORTS, ALE_PORT_1 | ALE_PORT_2);

    cpsw_init_stp_ale_entry(cpsw);

    cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_P0_UNI_FLOOD, 1);
    dev_dbg(cpsw->dev, "Set P0_UNI_FLOOD\n");
    cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_PORT_NOLEARN, 0);
}

static void cpsw_init_host_port_dual_mac(struct cpsw_common* cpsw) {
    int vlan = cpsw->data.default_vlan;

    writel(CPSW_FIFO_DUAL_MAC_MODE, &cpsw->host_port_regs->tx_in_ctl);

    cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_P0_UNI_FLOOD, 0);
    dev_dbg(cpsw->dev, "unset P0_UNI_FLOOD\n");

    writel(vlan, &cpsw->host_port_regs->port_vlan);

    cpsw_ale_add_vlan(cpsw->ale, vlan, ALE_ALL_PORTS, ALE_ALL_PORTS, 0, 0);
    /* learning make no sense in dual_mac mode */
    cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_PORT_NOLEARN, 1);
}

static void cpsw_init_host_port(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    u32 control_reg;

    /* soft reset the controller and initialize ale */
    soft_reset("cpsw", &cpsw->regs->soft_reset);
    cpsw_ale_start(cpsw->ale);

    /* switch to vlan unaware mode */
    cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_VLAN_AWARE,
                         CPSW_ALE_VLAN_AWARE);
    control_reg = readl(&cpsw->regs->control);
    control_reg |= CPSW_VLAN_AWARE | CPSW_RX_VLAN_ENCAP;
    writel(control_reg, &cpsw->regs->control);

    /* setup host port priority mapping */
    writel_relaxed(CPDMA_TX_PRIORITY_MAP,
                   &cpsw->host_port_regs->cpdma_tx_pri_map);
    writel_relaxed(0, &cpsw->host_port_regs->cpdma_rx_chan_map);

    /* disable priority elevation */
    writel_relaxed(0, &cpsw->regs->ptype);

    /* enable statistics collection only on all ports */
    writel_relaxed(0x7, &cpsw->regs->stat_port_en);

    /* Enable internal fifo flow control */
    writel(0x7, &cpsw->regs->flow_control);

    if (cpsw_is_switch_en(cpsw))
        cpsw_init_host_port_switch(cpsw);
    else
        cpsw_init_host_port_dual_mac(cpsw);

    cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_PORT_STATE,
                         ALE_PORT_STATE_FORWARD);
}

static void cpsw_port_add_dual_emac_def_ale_entries(struct cpsw_priv* priv,
                                                    struct cpsw_slave* slave) {
    u32 port_mask = 1 << priv->emac_port | ALE_PORT_HOST;
    struct cpsw_common* cpsw = priv->cpsw;
    u32 reg;

    reg = (cpsw->version == CPSW_VERSION_1) ? CPSW1_PORT_VLAN : CPSW2_PORT_VLAN;
    slave_write(slave, slave->port_vlan, reg);

    cpsw_ale_add_vlan(cpsw->ale, slave->port_vlan, port_mask, port_mask,
                      port_mask, 0);
    cpsw_ale_add_mcast(cpsw->ale, priv->ndev->broadcast, ALE_PORT_HOST,
                       ALE_VLAN, slave->port_vlan, ALE_MCAST_FWD);
    cpsw_ale_add_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM,
                       ALE_VLAN | ALE_SECURE, slave->port_vlan);
    cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_DROP_UNKNOWN_VLAN,
                         1);
    /* learning make no sense in dual_mac mode */
    cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_NOLEARN, 1);
}

static void cpsw_port_add_switch_def_ale_entries(struct cpsw_priv* priv,
                                                 struct cpsw_slave* slave) {
    u32 port_mask = 1 << priv->emac_port | ALE_PORT_HOST;
    struct cpsw_common* cpsw = priv->cpsw;
    u32 reg;

    cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_DROP_UNKNOWN_VLAN,
                         0);
    cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_NOLEARN, 0);
    /* disabling SA_UPDATE required to make stp work, without this setting
     * Host MAC addresses will jump between ports.
     * As per TRM MAC address can be defined as unicast supervisory (super)
     * by setting both (ALE_BLOCKED | ALE_SECURE) which should prevent
     * SA_UPDATE, but HW seems works incorrectly and setting ALE_SECURE
     * causes STP packets to be dropped due to ingress filter
     *	if (source address found) and (secure) and
     *	   (receive port number != port_number))
     *	   then discard the packet
     */
    cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_NO_SA_UPDATE, 1);

    cpsw_ale_add_mcast(cpsw->ale, priv->ndev->broadcast, port_mask, ALE_VLAN,
                       slave->port_vlan, ALE_MCAST_FWD_2);
    cpsw_ale_add_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM, ALE_VLAN,
                       slave->port_vlan);

    reg = (cpsw->version == CPSW_VERSION_1) ? CPSW1_PORT_VLAN : CPSW2_PORT_VLAN;
    slave_write(slave, slave->port_vlan, reg);
}

static void cpsw_adjust_link(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;
    struct phy_device* phy;
    u32 mac_control = 0;

    slave = &cpsw->slaves[priv->emac_port - 1];
    phy = slave->phy;

    if (!phy)
        return;

    if (phy->link) {
        mac_control = CPSW_SL_CTL_GMII_EN;

        if (phy->speed == 1000)
            mac_control |= CPSW_SL_CTL_GIG;
        if (phy->duplex)
            mac_control |= CPSW_SL_CTL_FULLDUPLEX;

        /* set speed_in input in case RMII mode is used in 100Mbps */
        if (phy->speed == 100)
            mac_control |= CPSW_SL_CTL_IFCTL_A;
        /* in band mode only works in 10Mbps RGMII mode */
        else if ((phy->speed == 10) && phy_interface_is_rgmii(phy))
            mac_control |= CPSW_SL_CTL_EXT_EN; /* In Band mode */

        if (priv->rx_pause)
            mac_control |= CPSW_SL_CTL_RX_FLOW_EN;

        if (priv->tx_pause)
            mac_control |= CPSW_SL_CTL_TX_FLOW_EN;

        if (mac_control != slave->mac_control)
            cpsw_sl_ctl_set(slave->mac_sl, mac_control);

        /* enable forwarding */
        cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_STATE,
                             ALE_PORT_STATE_FORWARD);

        netif_tx_wake_all_queues(ndev);

        if (priv->shp_cfg_speed && priv->shp_cfg_speed != slave->phy->speed &&
            !cpsw_shp_is_off(priv))
            dev_warn(priv->dev,
                     "Speed was changed, CBS shaper speeds are changed!");
    } else {
        netif_tx_stop_all_queues(ndev);

        mac_control = 0;
        /* disable forwarding */
        cpsw_ale_control_set(cpsw->ale, priv->emac_port, ALE_PORT_STATE,
                             ALE_PORT_STATE_DISABLE);

        cpsw_sl_wait_for_idle(slave->mac_sl, 100);

        cpsw_sl_ctl_reset(slave->mac_sl);
    }

    if (mac_control != slave->mac_control)
        phy_print_status(phy);

    slave->mac_control = mac_control;

    if (phy->link && cpsw_need_resplit(cpsw))
        cpsw_split_res(cpsw);
}

static void cpsw_slave_open(struct cpsw_slave* slave, struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;
    struct phy_device* phy;

    cpsw_sl_reset(slave->mac_sl, 100);
    cpsw_sl_ctl_reset(slave->mac_sl);

    /* setup priority mapping */
    cpsw_sl_reg_write(slave->mac_sl, CPSW_SL_RX_PRI_MAP, RX_PRIORITY_MAPPING);

    switch (cpsw->version) {
        case CPSW_VERSION_1:
            slave_write(slave, TX_PRIORITY_MAPPING, CPSW1_TX_PRI_MAP);
            /* Increase RX FIFO size to 5 for supporting fullduplex
             * flow control mode
             */
            slave_write(
                slave,
                (CPSW_MAX_BLKS_TX << CPSW_MAX_BLKS_TX_SHIFT) | CPSW_MAX_BLKS_RX,
                CPSW1_MAX_BLKS);
            break;
        case CPSW_VERSION_2:
        case CPSW_VERSION_3:
        case CPSW_VERSION_4:
            slave_write(slave, TX_PRIORITY_MAPPING, CPSW2_TX_PRI_MAP);
            /* Increase RX FIFO size to 5 for supporting fullduplex
             * flow control mode
             */
            slave_write(
                slave,
                (CPSW_MAX_BLKS_TX << CPSW_MAX_BLKS_TX_SHIFT) | CPSW_MAX_BLKS_RX,
                CPSW2_MAX_BLKS);
            break;
    }

    /* setup max packet size, and mac address */
    cpsw_sl_reg_write(slave->mac_sl, CPSW_SL_RX_MAXLEN, cpsw->rx_packet_max);
    cpsw_set_slave_mac(slave, priv);

    slave->mac_control = 0; /* no link yet */

    if (cpsw_is_switch_en(cpsw))
        cpsw_port_add_switch_def_ale_entries(priv, slave);
    else
        cpsw_port_add_dual_emac_def_ale_entries(priv, slave);

    if (!slave->data->phy_node)
        dev_err(priv->dev, "no phy found on slave %d\n", slave->slave_num);
    phy = of_phy_connect(priv->ndev, slave->data->phy_node, &cpsw_adjust_link,
                         0, slave->data->phy_if);
    if (!phy) {
        dev_err(priv->dev, "phy \"%pOF\" not found on slave %d\n",
                slave->data->phy_node, slave->slave_num);
        return;
    }
    slave->phy = phy;

    phy_attached_info(slave->phy);

    phy_start(slave->phy);

    /* Configure GMII_SEL register */
    phy_set_mode_ext(slave->data->ifphy, PHY_MODE_ETHERNET,
                     slave->data->phy_if);
}

int cpsw_ndo_stop(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpsw_slave* slave;

    cpsw_info(priv, ifdown, "shutting down ndev\n");
    slave = &cpsw->slaves[priv->emac_port - 1];
    if (slave->phy)
        phy_stop(slave->phy);

    netif_tx_stop_all_queues(priv->ndev);

    if (slave->phy) {
        phy_disconnect(slave->phy);
        slave->phy = NULL;
    }

    __hw_addr_ref_unsync_dev(&ndev->mc, ndev, cpsw_purge_all_mc);

    if (cpsw->usage_count <= 1) {
        napi_disable(&cpsw->napi_rx);
        napi_disable(&cpsw->napi_tx);
        cpts_unregister(cpsw->cpts);
        cpsw_intr_disable(cpsw);
        cpdma_ctlr_stop(cpsw->dma);
        cpsw_ale_stop(cpsw->ale);
        cpsw_destroy_xdp_rxqs(cpsw);
    }

    if (cpsw_need_resplit(cpsw))
        cpsw_split_res(cpsw);

    cpsw->usage_count--;
    pm_runtime_put_sync(cpsw->dev);
    return 0;
}

int cpsw_ndo_open(struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int ret;

    dev_info(priv->dev, "starting ndev. mode: %s\n",
             cpsw_is_switch_en(cpsw) ? "switch" : "dual_mac");
    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    /* Notify the stack of the actual queue counts. */
    ret = netif_set_real_num_tx_queues(ndev, cpsw->tx_ch_num);
    if (ret) {
        dev_err(priv->dev,
                "cannot set real number of tx queues: ch_num: %d, ndev: %p\n",
                cpsw->tx_ch_num, ndev);
        goto pm_cleanup;
    }

    ret = netif_set_real_num_rx_queues(ndev, cpsw->rx_ch_num);
    if (ret) {
        dev_err(priv->dev, "cannot set real number of rx queues\n");
        goto pm_cleanup;
    }

    /* Initialize host and slave ports */
    if (!cpsw->usage_count)
        cpsw_init_host_port(priv);
    cpsw_slave_open(&cpsw->slaves[priv->emac_port - 1], priv);

    /* initialize shared resources for every ndev */
    if (!cpsw->usage_count) {
        /* create rxqs for both infs in dual mac as they use same pool
         * and must be destroyed together when no users.
         */
        ret = cpsw_create_xdp_rxqs(cpsw);
        if (ret < 0)
            goto err_cleanup;

        ret = cpsw_fill_rx_channels(priv);
        if (ret < 0)
            goto err_cleanup;

        if (cpsw->cpts) {
            if (cpts_register(cpsw->cpts))
                dev_err(priv->dev, "error registering cpts device\n");
            else
                writel(0x10, &cpsw->wr_regs->misc_en);
        }

        napi_enable(&cpsw->napi_rx);
        napi_enable(&cpsw->napi_tx);

        if (cpsw->tx_irq_disabled) {
            cpsw->tx_irq_disabled = false;
            enable_irq(cpsw->irqs_table[1]);
        }

        if (cpsw->rx_irq_disabled) {
            cpsw->rx_irq_disabled = false;
            enable_irq(cpsw->irqs_table[0]);
        }
    }

    cpsw_restore(priv);

    /* Enable Interrupt pacing if configured */
    if (cpsw->coal_intvl != 0) {
        struct ethtool_coalesce coal;

        coal.rx_coalesce_usecs = cpsw->coal_intvl;
        cpsw_set_coalesce(priv, &coal, NULL, NULL);
    }

    cpdma_ctlr_start(cpsw->dma);
    cpsw_intr_enable(cpsw);
    cpsw->usage_count++;

    return 0;

err_cleanup:
    cpsw_ndo_stop(ndev);

pm_cleanup:
    pm_runtime_put_sync(cpsw->dev);
    return ret;
}

netdev_tx_t cpsw_ndo_start_xmit(struct sk_buff* skb,
                                       struct net_device* ndev) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    struct cpts* cpts = cpsw->cpts;
    struct netdev_queue* txq;
    struct cpdma_chan* txch;
    int ret, q_idx;

    if (skb_put_padto(skb, READ_ONCE(priv->tx_packet_min))) {
        cpsw_err(priv, tx_err, "packet pad failed\n");
        ndev->stats.tx_dropped++;
        return NET_XMIT_DROP;
    }

    if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP && priv->tx_ts_enabled &&
        cpts_can_timestamp(cpts, skb))
        skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;

    q_idx = skb_get_queue_mapping(skb);
    if (q_idx >= cpsw->tx_ch_num)
        q_idx = q_idx % cpsw->tx_ch_num;

    txch = cpsw->txv[q_idx].ch;
    txq = netdev_get_tx_queue(ndev, q_idx);
    skb_tx_timestamp(skb);
    ret = cpdma_chan_submit(txch, skb, skb->data, skb->len, priv->emac_port);
    if (unlikely(ret != 0)) {
        cpsw_err(priv, tx_err, "desc submit failed\n");
        goto fail;
    }

    /* If there is no more tx desc left free then we need to
     * tell the kernel to stop sending us tx frames.
     */
    if (unlikely(!cpdma_check_free_tx_desc(txch))) {
        netif_tx_stop_queue(txq);

        /* Barrier, so that stop_queue visible to other cpus */
        smp_mb__after_atomic();

        if (cpdma_check_free_tx_desc(txch))
            netif_tx_wake_queue(txq);
    }

    return NETDEV_TX_OK;
fail:
    ndev->stats.tx_dropped++;
    netif_tx_stop_queue(txq);

    /* Barrier, so that stop_queue visible to other cpus */
    smp_mb__after_atomic();

    if (cpdma_check_free_tx_desc(txch))
        netif_tx_wake_queue(txq);

    return NETDEV_TX_BUSY;
}
// netdev_tx_t cpsw_ndo_start_xmit(struct sk_buff* skb,
//                                        const struct cpsw_priv* priv,
//                                        struct netdev_queue* txq) {
//     struct cpsw_common* cpsw = priv->cpsw;
//     struct cpts* cpts = cpsw->cpts;
//     struct cpdma_chan* txch;
//     int ret, q_idx;

//     if (skb_put_padto(skb, READ_ONCE(priv->tx_packet_min))) {
//         cpsw_err(priv, tx_err, "packet pad failed\n");
//         // TODO
//         // ndev->stats.tx_dropped++;
//         return NET_XMIT_DROP;
//     }

//     // can be ignored considering cpts is inactive
//     if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP && priv->tx_ts_enabled &&
//         cpts_can_timestamp(cpts, skb))
//         skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;

//     q_idx = skb_get_queue_mapping(skb);
//     if (q_idx >= cpsw->tx_ch_num)
//         q_idx = q_idx % cpsw->tx_ch_num;

//     txch = cpsw->txv[q_idx].ch;
//     // txq = netdev_get_tx_queue(ndev, q_idx);
//     skb_tx_timestamp(skb);
//     ret = cpdma_chan_submit(txch, skb, skb->data, skb->len, priv->emac_port);
//     if (unlikely(ret != 0)) {
//         cpsw_err(priv, tx_err, "desc submit failed\n");
//         goto fail;
//     }

//     /* If there is no more tx desc left free then we need to
//      * tell the kernel to stop sending us tx frames.
//      */
//     if (unlikely(!cpdma_check_free_tx_desc(txch))) {
//         netif_tx_stop_queue(txq);

//         /* Barrier, so that stop_queue visible to other cpus */
//         smp_mb__after_atomic();

//         if (cpdma_check_free_tx_desc(txch))
//             netif_tx_wake_queue(txq);
//     }

//     return NETDEV_TX_OK;
// fail:
//     // ndev->stats.tx_dropped++;
//     netif_tx_stop_queue(txq);

//     /* Barrier, so that stop_queue visible to other cpus */
//     smp_mb__after_atomic();

//     if (cpdma_check_free_tx_desc(txch))
//         netif_tx_wake_queue(txq);

//     return NETDEV_TX_BUSY;
// }


static int cpsw_ndo_vlan_rx_kill_vid(struct net_device* ndev,
                                     __be16 proto,
                                     u16 vid) {
    struct cpsw_priv* priv = netdev_priv(ndev);
    struct cpsw_common* cpsw = priv->cpsw;
    int ret;
    int i;

    if (cpsw_is_switch_en(cpsw)) {
        dev_dbg(cpsw->dev, "ndo del vlan is called in switch mode\n");
        return 0;
    }

    if (vid == cpsw->data.default_vlan)
        return 0;

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    /* reset the return code as pm_runtime_get_sync() can return
     * non zero values as well.
     */
    ret = 0;
    for (i = 0; i < cpsw->data.slaves; i++) {
        if (cpsw->slaves[i].ndev && vid == cpsw->slaves[i].port_vlan) {
            ret = -EINVAL;
            goto err;
        }
    }

    dev_dbg(priv->dev, "removing vlanid %d from vlan filter\n", vid);
    ret = cpsw_ale_del_vlan(cpsw->ale, vid, 0);
    if (ret)
        dev_err(priv->dev, "cpsw_ale_del_vlan() failed: ret %d\n", ret);
    ret = cpsw_ale_del_ucast(cpsw->ale, priv->mac_addr, HOST_PORT_NUM, ALE_VLAN,
                             vid);
    if (ret)
        dev_err(priv->dev, "cpsw_ale_del_ucast() failed: ret %d\n", ret);
    ret =
        cpsw_ale_del_mcast(cpsw->ale, priv->ndev->broadcast, 0, ALE_VLAN, vid);
    if (ret)
        dev_err(priv->dev, "cpsw_ale_del_mcast failed. ret %d\n", ret);
    cpsw_ale_flush_multicast(cpsw->ale, ALE_PORT_HOST, vid);
    ret = 0;
err:
    pm_runtime_put(cpsw->dev);
    return ret;
}
static const struct net_device_ops cpsw_netdev_ops = {
    .ndo_open = cpsw_ndo_open,
    .ndo_stop = cpsw_ndo_stop,
    .ndo_start_xmit = cpsw_ndo_start_xmit,
    // .ndo_set_mac_address = cpsw_ndo_set_mac_address,
    .ndo_vlan_rx_kill_vid = cpsw_ndo_vlan_rx_kill_vid,
    .ndo_vlan_rx_add_vid = cpsw_ndo_vlan_rx_add_vid,
};


int cpsw_probe_dt(struct cpsw_common* cpsw) {
    struct device_node *node = cpsw->dev->of_node, *tmp_node, *port_np;
    struct cpsw_platform_data* data = &cpsw->data;
    struct device* dev = cpsw->dev;
    int ret;
    u32 prop;

    if (!node)
        return -EINVAL;

    tmp_node = of_get_child_by_name(node, "ethernet-ports");
    if (!tmp_node)
        return -ENOENT;
    data->slaves = of_get_child_count(tmp_node);
    if (data->slaves != CPSW_SLAVE_PORTS_NUM) {
        of_node_put(tmp_node);
        return -ENOENT;
    }

    data->active_slave = 0;
    data->channels = CPSW_MAX_QUEUES;
    data->dual_emac = true;
    data->bd_ram_size = CPSW_BD_RAM_SIZE;
    data->mac_control = 0;

    data->slave_data = devm_kcalloc(dev, CPSW_SLAVE_PORTS_NUM,
                                    sizeof(struct cpsw_slave_data), GFP_KERNEL);
    if (!data->slave_data) {
        of_node_put(tmp_node);
        return -ENOMEM;
    }

    /* Populate all the child nodes here...
     */
    ret = devm_of_platform_populate(dev);
    /* We do not want to force this, as in some cases may not have child */
    if (ret)
        dev_warn(dev, "Doesn't have any child node\n");

    for_each_child_of_node(tmp_node, port_np) {
        struct cpsw_slave_data* slave_data;
        u32 port_id;

        ret = of_property_read_u32(port_np, "reg", &port_id);
        if (ret < 0) {
            dev_err(dev, "%pOF error reading port_id %d\n", port_np, ret);
            goto err_node_put;
        }

        if (!port_id || port_id > CPSW_SLAVE_PORTS_NUM) {
            dev_err(dev, "%pOF has invalid port_id %u\n", port_np, port_id);
            ret = -EINVAL;
            goto err_node_put;
        }

        slave_data = &data->slave_data[port_id - 1];

        slave_data->disabled = !of_device_is_available(port_np);
        if (slave_data->disabled)
            continue;

        slave_data->slave_node = port_np;
        slave_data->ifphy = devm_of_phy_get(dev, port_np, NULL);
        if (IS_ERR(slave_data->ifphy)) {
            ret = PTR_ERR(slave_data->ifphy);
            dev_err(dev, "%pOF: Error retrieving port phy: %d\n", port_np, ret);
            goto err_node_put;
        }

        if (of_phy_is_fixed_link(port_np)) {
            ret = of_phy_register_fixed_link(port_np);
            if (ret) {
                dev_err_probe(dev, ret,
                              "%pOF failed to register fixed-link phy\n",
                              port_np);
                goto err_node_put;
            }
            slave_data->phy_node = of_node_get(port_np);
        } else {
            slave_data->phy_node = of_parse_phandle(port_np, "phy-handle", 0);
        }

        if (!slave_data->phy_node) {
            dev_err(dev, "%pOF no phy found\n", port_np);
            ret = -ENODEV;
            goto err_node_put;
        }

        ret = of_get_phy_mode(port_np, &slave_data->phy_if);
        if (ret) {
            dev_err(dev, "%pOF read phy-mode err %d\n", port_np, ret);
            goto err_node_put;
        }

        ret = of_get_mac_address(port_np, slave_data->mac_addr);
        if (ret) {
            ret = ti_cm_get_macid(dev, port_id - 1, slave_data->mac_addr);
            if (ret)
                goto err_node_put;
        }

        if (of_property_read_u32(port_np, "ti,dual-emac-pvid", &prop)) {
            dev_err(dev, "%pOF Missing dual_emac_res_vlan in DT.\n", port_np);
            slave_data->dual_emac_res_vlan = port_id;
            dev_err(dev, "%pOF Using %d as Reserved VLAN\n", port_np,
                    slave_data->dual_emac_res_vlan);
        } else {
            slave_data->dual_emac_res_vlan = prop;
        }
    }

    of_node_put(tmp_node);
    return 0;

err_node_put:
    of_node_put(port_np);
    of_node_put(tmp_node);
    return ret;
}

static void cpsw_remove_dt(struct cpsw_common* cpsw) {
    struct cpsw_platform_data* data = &cpsw->data;
    int i = 0;

    for (i = 0; i < cpsw->data.slaves; i++) {
        struct cpsw_slave_data* slave_data = &data->slave_data[i];
        struct device_node* port_np = slave_data->phy_node;

        if (port_np) {
            if (of_phy_is_fixed_link(port_np))
                of_phy_deregister_fixed_link(port_np);

            of_node_put(port_np);
        }
    }
}

int cpsw_create_ports(struct cpsw_common* cpsw) {
    struct cpsw_platform_data* data = &cpsw->data;
    struct net_device *ndev, *napi_ndev = NULL;
    struct device* dev = cpsw->dev;
    struct cpsw_priv* priv;
    int ret = 0, i = 0;
    for (i = 0; i < cpsw->data.slaves; i++) {
        struct cpsw_slave_data* slave_data = &data->slave_data[i];

        if (slave_data->disabled)
            continue;

        ndev = devm_alloc_etherdev_mqs(dev, sizeof(struct cpsw_priv),
                                       CPSW_MAX_QUEUES, CPSW_MAX_QUEUES);
        if (!ndev) {
            dev_err(dev, "error allocating net_device\n");
            return -ENOMEM;
        }

        priv = netdev_priv(ndev);
        priv->cpsw = cpsw;
        priv->ndev = ndev;
        priv->dev = dev;
        priv->msg_enable = netif_msg_init(0, CPSW_DEBUG); // disable debug output
        priv->emac_port = i + 1;
        priv->tx_packet_min = CPSW_MIN_PACKET_SIZE;

        if (is_valid_ether_addr(slave_data->mac_addr)) {
            ether_addr_copy(priv->mac_addr, slave_data->mac_addr);
            dev_info(cpsw->dev, "Detected MACID = %pM\n", priv->mac_addr);
        } else {
            eth_random_addr(slave_data->mac_addr);
            dev_info(cpsw->dev, "Random MACID = %pM\n", priv->mac_addr);
        }
        eth_hw_addr_set(ndev, slave_data->mac_addr);
        ether_addr_copy(priv->mac_addr, slave_data->mac_addr);

        cpsw->slaves[i].ndev = ndev;

    // ndev->features |= NETIF_F_HW_VLAN_CTAG_FILTER | NETIF_F_HW_VLAN_CTAG_RX |
    //                   NETIF_F_NETNS_LOCAL | NETIF_F_HW_TC;

        ndev->netdev_ops = &cpsw_netdev_ops;
        // ndev->ethtool_ops = &cpsw_ethtool_ops;
        SET_NETDEV_DEV(ndev, dev);

        if (!napi_ndev) {
            /* CPSW Host port CPDMA interface is shared between
             * ports and there is only one TX and one RX IRQs
             * available for all possible TX and RX channels
             * accordingly.
             */
            netif_napi_add(ndev, &cpsw->napi_rx,
                           cpsw->quirk_irq ? cpsw_rx_poll : cpsw_rx_mq_poll);
            netif_napi_add_tx(ndev, &cpsw->napi_tx,
                              cpsw->quirk_irq ? cpsw_tx_poll : cpsw_tx_mq_poll);
        }

        napi_ndev = ndev;
    }

    return ret;
}

static void cpsw_unregister_ports(struct cpsw_common* cpsw) {
    int i = 0;

    for (i = 0; i < cpsw->data.slaves; i++) {
        if (!cpsw->slaves[i].ndev)
            continue;

        unregister_netdev(cpsw->slaves[i].ndev);
    }
}

int cpsw_register_ports(struct cpsw_common* cpsw) {
    int ret = 0, i = 0;

    for (i = 0; i < cpsw->data.slaves; i++) {
        if (!cpsw->slaves[i].ndev)
            continue;

        /* register the network device */
        ret = register_netdev(cpsw->slaves[i].ndev);
        if (ret) {
            dev_err(cpsw->dev, "cpsw: err registering net device%d\n", i);
            cpsw->slaves[i].ndev = NULL;
            break;
        }
    }

    if (ret)
        cpsw_unregister_ports(cpsw);
    return ret;
}

bool cpsw_port_dev_check(const struct net_device* ndev) {
    if (ndev->netdev_ops == &cpsw_netdev_ops) {
        struct cpsw_common* cpsw = ndev_to_cpsw(ndev);

        return !cpsw->data.dual_emac;
    }

    return false;
}

// int cpsw_probe(struct cpsw_common* cpsw, struct resource* ss_res) {
//     struct device* dev = cpsw->dev;
//     int ret = 0;


//     /* This may be required here for child devices. */
//     pm_runtime_enable(dev);

//     /* Need to enable clocks with runtime PM api to access module
//      * registers
//      */
//     // ret = pm_runtime_resume_and_get(dev);
//     // if (ret < 0) {
//     //     pm_runtime_disable(dev);
//     //     return ret;
//     // }


//     /* if (!cpsw->cpts)
//         goto skip_cpts;

//     ret = devm_request_irq(dev, cpsw->misc_irq, cpsw_misc_interrupt, 0,
//                            dev_name(cpsw->dev), cpsw);
//     if (ret < 0) {
//         dev_err(dev, "error attaching misc irq (%d)\n", ret);
//         goto clean_unregister_netdev;
//     }
// */

// // skip_cpts:
//     // ret = cpsw_register_notifiers(cpsw);
//     // if (ret)
//     //     goto clean_unregister_netdev;

//     // ret = cpsw_register_devlink(cpsw);
//     // if (ret)
//     //     goto clean_unregister_notifiers;

//     dev_notice(
//         dev, "initialized (regs %pa, pool size %d) hw_ver:%08X %d.%d (%d)\n",
//         &ss_res->start, descs_pool_size, cpsw->version,
//         CPSW_MAJOR_VERSION(cpsw->version), CPSW_MINOR_VERSION(cpsw->version),
//         CPSW_RTL_VERSION(cpsw->version));

//     pm_runtime_put(dev);

//     return 0;

// // clean_unregister_netdev:
//     cpsw_unregister_ports(cpsw);
// // clean_cpts:
//     cpts_release(cpsw->cpts);
//     cpdma_ctlr_destroy(cpsw->dma);
// // clean_dt_ret:
//     cpsw_remove_dt(cpsw);
//     pm_runtime_put_sync(dev);
//     pm_runtime_disable(dev);
//     return ret;
// }

int cpsw_remove(struct cpsw_common* cpsw) {
    int ret;

    ret = pm_runtime_resume_and_get(cpsw->dev);
    if (ret < 0)
        return ret;

    cpsw_unregister_ports(cpsw);

    cpts_release(cpsw->cpts);
    cpdma_ctlr_destroy(cpsw->dma);
    cpsw_remove_dt(cpsw);
    pm_runtime_put_sync(cpsw->dev);
    pm_runtime_disable(cpsw->dev);
    return 0;
}
MODULE_LICENSE("GPL");
