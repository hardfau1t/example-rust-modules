#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h> // pm_runtime_enable

static const struct of_device_id cpsw_of_mtable[] = {
	{ .compatible = "ti,am335x-cpsw-switch"},
};

struct cpsw_data{
    u32 bus_freq_mhz;
    int irq_tx;
    int irq_rx;
    int irq_misc;

};
static int cpsw_probe (struct platform_device * pdev){
    struct cpsw_data* cpsw ;
	int irq, ret;
    pr_info("registered cpsw: %s, Id: %d\n", pdev->name, pdev->id);
	cpsw = devm_kzalloc(&pdev->dev, sizeof(struct cpsw_data), GFP_KERNEL);
	if (!cpsw)
		return -ENOMEM;
    // TODO: add slaves and initialize slaves
    // TODO: get clock frequency
    // TODO: get the registers
	irq = platform_get_irq_byname(pdev, "rx");
	if (irq < 0)
		return irq;
	cpsw->irq_rx = irq;

	irq = platform_get_irq_byname(pdev, "tx");
	if (irq < 0)
		return irq;
	cpsw->irq_tx = irq;

	irq = platform_get_irq_byname(pdev, "misc");
	if (irq <= 0)
		return irq;
	cpsw->irq_misc = irq;
    // sets the cpsw struct to pdev structure
	platform_set_drvdata(pdev, cpsw);
    // NOTE: try removing this, doesn't look its doing much
	pm_runtime_enable(&pdev->dev);
    // Not sure what is this
	ret = pm_runtime_resume_and_get(&pdev->dev);
	if (ret < 0) {
		pm_runtime_disable(&pdev->dev);
		return ret;
	}
    // TODO: `cpsw_probe_dt` will read the dtb file and sets the interface details
    return 0;
}
static int cpsw_remove(struct platform_device * device){
    pr_info("cpsw removed\n");
    return 0;
}


static struct platform_driver cpsw_platform = {
	.probe = cpsw_probe,
	.remove = cpsw_remove,
	.driver = {
		.name = "cpsw-switch",
        .of_match_table = cpsw_of_mtable,
	},
};




int cpsw_init(void)
{
    platform_driver_register(&cpsw_platform);
    return 0;
}

void cpsw_cleanup(void)
{
    platform_driver_unregister(&cpsw_platform);
}

MODULE_LICENSE("GPL");
module_init(cpsw_init);
module_exit(cpsw_cleanup);
