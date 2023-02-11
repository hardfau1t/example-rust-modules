#include "ceth.h"
#include <asm-generic/rwonce.h>
#include <linux/clk.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>  // pm_runtime_enable
#include <net/cfg80211.h>      // struct mac_address
#include "davinci_cpdma.h"

#define TRACE (pr_info("%s\n", __FUNCTION__))

#define CPDMA_RXTHRESH 0x0c0
#define CPDMA_RXFREE 0x0e0
#define CPDMA_TXHDP 0x00
#define CPDMA_RXHDP 0x20
#define CPDMA_TXCP 0x40
#define CPDMA_RXCP 0x60
#define CPSW_MIN_PACKET_SIZE (ETH_ZLEN)
#define CPSW_CPDMA_DESCS_POOL_SIZE_DEFAULT 256
#define CPSW_MAX_QUEUES 8
#define CPSW_BD_RAM_SIZE 0x2000
static const struct of_device_id cpsw_of_mtable[] = {
    {.compatible = "ti,am335x-cpsw-switch"},
};

struct cpsw_priv {
    u32 bus_freq_mhz;
    int irq_tx;
    int irq_rx;
    int irq_misc;
    struct cpdma_chan* tx_chan;
    struct cpdma_chan* rx_chan;
    int tx_chan_num;
    int rx_chan_num;
    struct cpdma_ctlr* dma;
    struct device* dev;
    struct net_device* ndev;
    struct cpsw_ss_regs __iomem* regs;
    struct cpsw_host_regs __iomem* host_port_regs;
};
struct cpsw_ss_regs {
    u32 id_ver;
    u32 control;
    u32 soft_reset;
    u32 stat_port_en;
    u32 ptype;
    u32 soft_idle;
    u32 thru_rate;
    u32 gap_thresh;
    u32 tx_start_wds;
    u32 flow_control;
    u32 vlan_ltype;
    u32 ts_ltype;
    u32 dlr_ltype;
};
struct cpsw_host_regs {
    u32 max_blks;
    u32 blk_cnt;
    u32 tx_in_ctl;
    u32 port_vlan;
    u32 tx_pri_map;
    u32 cpdma_tx_pri_map;
    u32 cpdma_rx_chan_map;
};
struct netdev_priv {
    struct mac_address device_addr;
    struct cpsw_priv* cpsw;
};
void soft_reset(const char* module, void __iomem* reg) {
    unsigned long timeout = jiffies + HZ;

    writel_relaxed(1, reg);
    do {
        cpu_relax();
    } while ((readl_relaxed(reg) & 1) && time_after(timeout, jiffies));

    WARN(readl_relaxed(reg) & 1, "failed to soft-reset %s\n", module);
}
static void cpsw_init_host_port_switch(struct cpsw_priv* cpsw) {
    // int vlan = cpsw->data.default_vlan;

    writel(CPSW_FIFO_NORMAL_MODE, &cpsw->host_port_regs->tx_in_ctl);

    // writel(vlan, &cpsw->host_port_regs->port_vlan);

    // cpsw_ale_add_vlan(cpsw->ale, vlan, ALE_ALL_PORTS,
    // 		  ALE_ALL_PORTS, ALE_ALL_PORTS,
    // 		  ALE_PORT_1 | ALE_PORT_2);

    // cpsw_init_stp_ale_entry(cpsw);

    // cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_P0_UNI_FLOOD, 1);
    dev_dbg(cpsw->dev, "Set P0_UNI_FLOOD\n");
    // cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_PORT_NOLEARN, 0);
}
static void cpsw_init_host_port(struct cpsw_priv* cpsw) {
    u32 control_reg;

    /* soft reset the controller and initialize ale */
    soft_reset("cpsw", &cpsw->regs->soft_reset);
    // cpsw_ale_start(cpsw->ale);

    /* switch to vlan unaware mode */
    // cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM, ALE_VLAN_AWARE,
    // 		     CPSW_ALE_VLAN_AWARE);
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

    cpsw_init_host_port_switch(cpsw);

    // cpsw_ale_control_set(cpsw->ale, HOST_PORT_NUM,
    // 		     ALE_PORT_STATE, ALE_PORT_STATE_FORWARD);
}
static int ndev_open(struct net_device* ndev) {
    int ret;
    struct netdev_priv* net_priv = netdev_priv(ndev);
    struct cpsw_priv* cpsw = net_priv->cpsw;
    dev_info(cpsw->dev, "starting ndev. mode, dev %p\n", cpsw->dev);
    /* Initialize the device and allocate resources */
    // ret = pm_runtime_resume_and_get(cpsw->dev);
    // if (ret < 0){
    //        dev_err(cpsw->dev, "Failed to resume ndev\n");
    // 	return ret;
    //    }
    // dev_info(cpsw->dev, "resume passed\n");
    /* Notify the stack of the actual queue counts. */
    ret = netif_set_real_num_tx_queues(ndev, cpsw->tx_chan_num);
    if (ret) {
        dev_err(cpsw->dev,
                "cannot set real number of tx queues: %d\n cpsw->tx_chan_num: "
                "%d, dev->queue: %d \n",
                ret, cpsw->tx_chan_num, ndev->num_tx_queues);
        return ret;
    }

    ret = netif_set_real_num_rx_queues(ndev, cpsw->rx_chan_num);
    if (ret) {
        dev_err(cpsw->dev, "cannot set real number of rx queues: %d\n", ret);
        return ret;
    }
    cpsw_init_host_port(cpsw);
    netif_start_queue(ndev);
    return 0;
}

static int ndev_stop(struct net_device* ndev) {
    TRACE;
    /* Release resources and shut down the device */
    netif_stop_queue(ndev);
    return 0;
}

static int ndev_xmit(struct sk_buff* skb, struct net_device* dev) {
	struct cpdma_chan *txch;
	int ret ; //,q_idx;
    struct cpsw_priv* cpsw = netdev_priv(dev);
    struct cpsw_priv* priv = netdev_priv(dev);
    if (skb_put_padto(skb, CPSW_MIN_PACKET_SIZE)) {
        dev_warn(priv->dev, "Failed to pad pad skbuff\n");
        return NET_XMIT_DROP;
    }

    // q_idx = skb_get_queue_mapping(skb);
    // if (q_idx >= cpsw->tx_chan_num)
    //     q_idx = q_idx % cpsw->tx_chan_num;

    txch = cpsw->tx_chan;
    // txq = netdev_get_tx_queue(dev, q_idx);
    skb_tx_timestamp(skb);
    ret = cpdma_chan_submit(txch, skb, skb->data, skb->len, 1); // 1 means first slave
    if (unlikely(ret != 0)) {
        dev_err(cpsw->dev, "desc submit failed: %d\n", ret);
        return NETDEV_TX_BUSY;
    }

    /* If there is no more tx desc left free then we need to
     * tell the kernel to stop sending us tx frames.
     */
    // if (unlikely(!cpdma_check_free_tx_desc(txch))) {
    //     netif_tx_stop_queue(txq);

    //     /* Barrier, so that stop_queue visible to other cpus */
    //     smp_mb__after_atomic();

    //     // if (cpdma_check_free_tx_desc(txch))
    //     //     netif_tx_wake_queue(txq);
    // }

    dev_info(cpsw->dev,"Success xmit\n");
    return 0;
}

static const struct net_device_ops cpsw_netops = {
    .ndo_open = ndev_open,
    .ndo_stop = ndev_stop,
    .ndo_start_xmit = ndev_xmit,
};
void cpsw_tx_handler(void* token, int len, int status) {
    pr_warn("inside chann create\n");
    TRACE;
}
static void cpsw_rx_handler(void* token, int len, int status) {
    TRACE;
}

void init_dma_registers(struct cpdma_params* dma_params) {
    memset(dma_params, 0, sizeof(*dma_params));
    dma_params->rxthresh = dma_params->dmaregs + CPDMA_RXTHRESH;
    dma_params->rxfree = dma_params->dmaregs + CPDMA_RXFREE;
    dma_params->rxhdp = dma_params->txhdp + CPDMA_RXHDP;
    dma_params->txcp = dma_params->txhdp + CPDMA_TXCP;
    dma_params->rxcp = dma_params->txhdp + CPDMA_RXCP;

    dma_params->num_chan = CPSW_MAX_QUEUES;
    dma_params->has_soft_reset = true;
    dma_params->min_packet_size = CPSW_MIN_PACKET_SIZE;
    dma_params->desc_mem_size = CPSW_BD_RAM_SIZE;
    dma_params->desc_align = 16;
    dma_params->has_ext_regs = true;
    dma_params->desc_hw_addr = dma_params->desc_mem_phys;
    dma_params->descs_pool_size = CPSW_CPDMA_DESCS_POOL_SIZE_DEFAULT;
}
void init_cpsw_struct(struct cpsw_priv* cpsw, void __iomem* ss_regs) {}

irqreturn_t cpsw_rx_interrupt(int irq, void *dev_id){

	return IRQ_HANDLED;
}
irqreturn_t cpsw_tx_interrupt(int irq, void *dev_id)
{
	/* struct cpsw_common *cpsw = dev_id;

	writel(0, &cpsw->wr_regs->tx_en);
	cpdma_ctlr_eoi(cpsw->dma, CPDMA_EOI_TX);

	if (cpsw->quirk_irq) {
		disable_irq_nosync(cpsw->irqs_table[1]);
		cpsw->tx_irq_disabled = true;
	}

	napi_schedule(&cpsw->napi_tx); */
	return IRQ_HANDLED;
}
static int cpsw_probe(struct platform_device* pdev) {
    struct net_device* ndev;
    struct netdev_priv* ndev_priv;
    struct cpdma_params dma_params;
    int err, irq;
    struct cpsw_priv* cpsw =
        devm_kzalloc(&pdev->dev, sizeof(*cpsw), GFP_KERNEL);
    if (!cpsw)
        return -ENOMEM;
    cpsw->regs = devm_ioremap_resource(&pdev->dev, &pdev->resource[0]);
    cpsw->host_port_regs = (void __iomem*)cpsw->regs + CPSW2_HOST_PORT_OFFSET;
    if (IS_ERR(cpsw->regs)) {
        return PTR_ERR(cpsw->regs);
    }
    init_dma_registers(&dma_params);
    dma_params.dev = &pdev->dev;
    dma_params.bus_freq_mhz = cpsw->bus_freq_mhz;
    /* Allocate a new net_device structure */
    ndev = devm_alloc_etherdev_mqs(&pdev->dev, sizeof(struct cpsw_priv),
                                   CPSW_MAX_QUEUES, CPSW_MAX_QUEUES);
    if (!ndev)
        return -ENOMEM;
    ndev_priv = netdev_priv(ndev);
    ndev_priv->cpsw = cpsw;
    /* Set the device name and device operations */
    strcpy(ndev->name, "cpsw");
    ndev->netdev_ops = &cpsw_netops;

    /* Set the MAC address */
    eth_random_addr(ndev_priv->device_addr.addr);
    eth_hw_addr_set(ndev, ndev_priv->device_addr.addr);
    cpsw->ndev = ndev;
    cpsw->dev = &pdev->dev;
    cpsw->tx_chan_num = 1;
    cpsw->rx_chan_num = 1;
    SET_NETDEV_DEV(ndev, &pdev->dev);
    cpsw->dma = cpdma_ctlr_create(&dma_params);
    if (!cpsw->dma) {
        dev_err(&pdev->dev, "error initializing dma\n");
        return -ENOMEM;
    }
    cpsw->tx_chan = cpdma_chan_create(cpsw->dma, 7, cpsw_tx_handler, 0);
    if (IS_ERR(cpsw->tx_chan)) {
        dev_err(&pdev->dev, "error initializing tx dma channel\n");
        err = PTR_ERR(cpsw->tx_chan);
        goto err_cpdma;
    }

    cpsw->rx_chan = cpdma_chan_create(cpsw->dma, 0, cpsw_rx_handler, 1);
    if (IS_ERR(cpsw->rx_chan)) {
        dev_err(&pdev->dev, "error initializing rx dma channel\n");
        err = PTR_ERR(cpsw->rx_chan);
        goto err_cpdma;
    }
    /*skipped resource splitting */

    /* Register the device with the kernel */
    err = register_netdev(ndev);
    if (err < 0) {
        free_netdev(ndev);
        goto err_free_netdev;
    }
	irq = platform_get_irq_byname(pdev, "rx");
	if (irq < 0)
		return irq;
	cpsw->irq_rx= irq;

	irq = platform_get_irq_byname(pdev, "tx");
	if (irq < 0)
		return irq;
	cpsw->irq_tx= irq;
	err = devm_request_irq(&pdev->dev, cpsw->irq_rx, cpsw_rx_interrupt,
			       0, dev_name(&pdev->dev), cpsw);
	if (err < 0) {
		dev_err(&pdev->dev, "error attaching irq (%d)\n", err);
		goto err_free_netdev;
	}

	err = devm_request_irq(&pdev->dev, cpsw->irq_tx, cpsw_tx_interrupt,
			       0, dev_name(&pdev->dev), cpsw);
	if (err < 0) {
		dev_err(&pdev->dev, "error attaching irq (%d)\n", err);
		goto err_free_netdev;
	}
    /*skipped notifiers*/
    // skipped devlink

    platform_set_drvdata(pdev, ndev);
    pm_runtime_put(&pdev->dev);
    return 0;
err_free_netdev:
    free_netdev(ndev);
err_cpdma:
    cpdma_ctlr_destroy(cpsw->dma);
    pm_runtime_put_sync(&pdev->dev);
    pm_runtime_disable(&pdev->dev);
    return err;
}

static int cpsw_remove(struct platform_device* pdev) {
    struct net_device* dev = platform_get_drvdata(pdev);

    /* Unregister the device and free resources */
    unregister_netdev(dev);
    free_netdev(dev);
    return 0;
}

// MODULE_DEVICE_TABLE(of, cpsw_of_mtable);

static struct platform_driver cpsw_platform_driver = {
    .probe = cpsw_probe,
    .remove = cpsw_remove,
    .driver =
        {
            .name = "cpsw-switch",
            .of_match_table = cpsw_of_mtable,
        },
};
module_platform_driver(cpsw_platform_driver);

MODULE_AUTHOR("hardfault");
MODULE_DESCRIPTION("Example Ethernet Driver for BeagleBone Black");
MODULE_LICENSE("GPL");
