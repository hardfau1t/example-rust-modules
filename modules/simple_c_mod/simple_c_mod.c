#include <asm-generic/rwonce.h>
#include <linux/clk.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>  // pm_runtime_enable
#include <net/cfg80211.h>      // struct mac_address

#define TRACE (pr_info("%s\n", __FUNCTION__))

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>

struct net_device* cpsw_ndev;

int cpsw_open(struct net_device* dev) {
    printk("cpsw open called\n");
    return 0;
}

int cpsw_stop(struct net_device* dev) {
    printk("cpsw_stop called\n");
    netif_stop_queue(dev);
    return 0;
}

int cpsw_xmit(struct sk_buff* skb, struct net_device* dev) {
    printk("cpsw xmit function called...\n");
    dev_kfree_skb(skb);
    return 0;
}
int cpsw_init(struct net_device* dev) {
    printk("cpsw device initialized\n");
    return 0;
};

const struct net_device_ops my_netdev_ops = {
    .ndo_init = cpsw_init,
    .ndo_open = cpsw_open,
    .ndo_stop = cpsw_stop,
    .ndo_start_xmit = cpsw_xmit,
};


static void virtual_setup(struct net_device* dev) {
    // dev->netdev_ops = &my_netdev_ops;
}

static const struct of_device_id cpsw_of_mtable[] = {
    {.compatible = "ti,am335x-cpsw-switch"},
};
static int cpsw_probe(struct platform_device* pdev) {
    int result;
    cpsw_ndev = alloc_netdev(0, "cpsw%d", NET_NAME_ENUM, virtual_setup);
    cpsw_ndev->netdev_ops = &my_netdev_ops;
    if ((result = register_netdev(cpsw_ndev))) {
        printk("CpswNetDev: Error %d initalizing card ...", result);
        return result;
    }
    return 0;
}

static int cpsw_remove(struct platform_device* pdev) {
    printk("Cpsw Platform Up the Module\n");
    unregister_netdev(cpsw_ndev);
    return 0;
}

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
