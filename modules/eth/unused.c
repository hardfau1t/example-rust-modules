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


static void cpsw_get_drvinfo(struct net_device* ndev,
                             struct ethtool_drvinfo* info) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);
    struct platform_device* pdev;

    pdev = to_platform_device(cpsw->dev);
    strscpy(info->driver, "cpsw-switch", sizeof(info->driver));
    strscpy(info->version, "2.0", sizeof(info->version));
    strscpy(info->bus_info, pdev->name, sizeof(info->bus_info));
}

static int cpsw_set_pauseparam(struct net_device* ndev,
                               struct ethtool_pauseparam* pause) {
    struct cpsw_common* cpsw = ndev_to_cpsw(ndev);
    struct cpsw_priv* priv = netdev_priv(ndev);
    int slave_no;

    slave_no = cpsw_slave_index(cpsw, priv);
    if (!cpsw->slaves[slave_no].phy)
        return -EINVAL;

    if (!phy_validate_pause(cpsw->slaves[slave_no].phy, pause))
        return -EINVAL;

    priv->rx_pause = pause->rx_pause ? true : false;
    priv->tx_pause = pause->tx_pause ? true : false;

    phy_set_asym_pause(cpsw->slaves[slave_no].phy, priv->rx_pause,
                       priv->tx_pause);

    return 0;
}

static int cpsw_set_channels(struct net_device* ndev,
                             struct ethtool_channels* chs) {
    return cpsw_set_channels_common(ndev, chs, cpsw_rx_handler);
}
static const struct ethtool_ops _cpsw_ethtool_ops = {
    .supported_coalesce_params = ETHTOOL_COALESCE_RX_USECS,
    .get_drvinfo = cpsw_get_drvinfo,
    .get_msglevel = cpsw_get_msglevel,
    .set_msglevel = cpsw_set_msglevel,
    .get_link = ethtool_op_get_link,
    .get_ts_info = cpsw_get_ts_info,
    .get_coalesce = cpsw_get_coalesce,
    .set_coalesce = cpsw_set_coalesce,
    .get_sset_count = cpsw_get_sset_count,
    .get_strings = cpsw_get_strings,
    .get_ethtool_stats = cpsw_get_ethtool_stats,
    .get_pauseparam = cpsw_get_pauseparam,
    .set_pauseparam = cpsw_set_pauseparam,
    .get_wol = cpsw_get_wol,
    .set_wol = cpsw_set_wol,
    .get_regs_len = cpsw_get_regs_len,
    .get_regs = cpsw_get_regs,
    .begin = cpsw_ethtool_op_begin,
    .complete = cpsw_ethtool_op_complete,
    .get_channels = cpsw_get_channels,
    .set_channels = cpsw_set_channels,
    .get_link_ksettings = cpsw_get_link_ksettings,
    .set_link_ksettings = cpsw_set_link_ksettings,
    .get_eee = cpsw_get_eee,
    .set_eee = cpsw_set_eee,
    .nway_reset = cpsw_nway_reset,
    .get_ringparam = cpsw_get_ringparam,
    .set_ringparam = cpsw_set_ringparam,
};

