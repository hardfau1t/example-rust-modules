#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/sys_soc.h>

static const struct of_device_id cpsw_of_mtable[] = {
    {.compatible = "ti,cpsw-switch"},
};

static int cpsw_probe(struct platform_device* pdev) {
    pr_info("plat installed %d\n", pdev->id);
    return 0;
}

static int cpsw_remove(struct platform_device* pdev) {
    pr_info("plat removed\n");
    return 0;
}

static struct platform_driver cpsw_driver = {
    .driver =
        {
            .name = "plat",
            .of_match_table = cpsw_of_mtable,
        },
    .probe = cpsw_probe,
    .remove = cpsw_remove,
};

module_platform_driver(cpsw_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("blah blah");
