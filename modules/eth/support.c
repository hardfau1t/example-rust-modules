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

#define CPTS_N_ETX_TS 4
#define CTRL_MAC_LO_REG(offset, id) ((offset) + 0x8 * (id))
#define CTRL_MAC_HI_REG(offset, id) ((offset) + 0x8 * (id) + 0x4)


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

/* restore resources after port reset */
static void cpsw_restore(struct cpsw_priv* priv) {
    struct cpsw_common* cpsw = priv->cpsw;


    /* restore MQPRIO offload */
    cpsw_mqprio_resume(&cpsw->slaves[priv->emac_port - 1], priv);

    /* restore CBS offload */
    cpsw_cbs_resume(&cpsw->slaves[priv->emac_port - 1], priv);

    cpsw_qos_clsflower_resume(priv);
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
             !cpsw->data.dual_emac ? "switch" : "dual_mac");
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
    struct netdev_queue* txq;
    struct cpdma_chan* txch;
    int ret, q_idx;

    if (skb_put_padto(skb, READ_ONCE(priv->tx_packet_min))) {
        cpsw_err(priv, tx_err, "packet pad failed\n");
        ndev->stats.tx_dropped++;
        return NET_XMIT_DROP;
    }


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

static const struct net_device_ops cpsw_netdev_ops = {
    .ndo_open = cpsw_ndo_open,
    .ndo_stop = cpsw_ndo_stop,
    .ndo_start_xmit = cpsw_ndo_start_xmit,
    // .ndo_set_mac_address = cpsw_ndo_set_mac_address,
};
irqreturn_t cpsw_tx_interrupt(int irq, void* dev_id) {
    
    struct cpsw_common* cpsw = dev_id;

    writel(0, &cpsw->wr_regs->tx_en);
    cpdma_ctlr_eoi(cpsw->dma, CPDMA_EOI_TX);

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
                           cpsw_rx_mq_poll);
            netif_napi_add_tx(ndev, &cpsw->napi_tx,
                               cpsw_tx_mq_poll);
        }

        napi_ndev = ndev;
    }

    return ret;
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
    if (status & CPDMA_RX_VLAN_ENCAP){
        cpsw_rx_vlan_encap(skb);
    }
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

void cpsw_intr_disable(struct cpsw_common* cpsw) {
    writel_relaxed(0, &cpsw->wr_regs->tx_en);
    writel_relaxed(0, &cpsw->wr_regs->rx_en);

    cpdma_ctlr_int_ctrl(cpsw->dma, false);
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


/* struct cpsw_common is not needed, kept here for compatibility
 * reasons witrh the old driver
 */
int cpsw_slave_index (struct cpsw_common* cpsw,
                                 struct cpsw_priv* priv) {
    if (priv->emac_port == HOST_PORT_NUM)
        return -1;

    return priv->emac_port - 1;
}

void soft_reset(const char* module, void __iomem* reg) {
    unsigned long timeout = jiffies + HZ;

    writel_relaxed(1, reg);
    do {
        cpu_relax();
    } while ((readl_relaxed(reg) & 1) && time_after(timeout, jiffies));

    WARN(readl_relaxed(reg) & 1, "failed to soft-reset %s\n", module);
}


void cpsw_set_slave_mac(struct cpsw_slave* slave, struct cpsw_priv* priv) {
    slave_write(slave, mac_hi(priv->mac_addr), SA_HI);
    slave_write(slave, mac_lo(priv->mac_addr), SA_LO);
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


void cpsw_cbs_resume(struct cpsw_slave* slave, struct cpsw_priv* priv) {
    int fifo, bw;

    for (fifo = CPSW_FIFO_SHAPERS_NUM; fifo > 0; fifo--) {
        bw = priv->fifo_bw[fifo];
        if (!bw)
            continue;

        cpsw_set_fifo_rlimit(priv, fifo, bw);
    }
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

void cpsw_intr_enable(struct cpsw_common* cpsw) {
    writel_relaxed(0xFF, &cpsw->wr_regs->tx_en);
    writel_relaxed(0xFF, &cpsw->wr_regs->rx_en);

    cpdma_ctlr_int_ctrl(cpsw->dma, true);
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
int ti_cm_get_macid(struct device *dev, int slave, u8 *mac_addr)
{
    u16 offset = 0x630;
	u32 macid_lo;
	u32 macid_hi;
	struct regmap *syscon;

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
