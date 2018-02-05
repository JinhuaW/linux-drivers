#include <linux/module.h>		/* Needed by all modules */
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/uio_driver.h>


static irqreturn_t test_irq_handler(int irq,  struct uio_info *uio)
{
	printk(KERN_INFO "test irq comes ...\n");
	return IRQ_HANDLED;
}

static int test_irq_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *of_node = dev->of_node;
	struct uio_info *uio;
	int ret,irq;

	/* Allocate test_irq_info data structure */
	uio = devm_kzalloc(dev, sizeof(*uio), GFP_KERNEL);
	if (!uio) {
		dev_err(dev, "cannot allocate test_irq resource\n");
		return -ENOMEM;
	}

	irq = of_irq_to_resource(of_node, 0, NULL);
	if (unlikely(irq == NO_IRQ)) {
		dev_warn(&pdev->dev,"couldn't obtain test interrupt information from device tree\n");
	}

	uio->irq = irq;
	uio->version = "0.0.1";
	uio->name = "test_irq";
	uio->handler = test_irq_handler;
	uio->irq_flags = IRQF_SHARED;
	uio->priv = pdev;

	platform_set_drvdata(pdev, uio);
	ret = uio_register_device(dev, uio);
	return ret;
}

static int test_irq_remove(struct platform_device *pdev)
{
	struct uio_info *info = platform_get_drvdata(pdev);
	uio_unregister_device(info);
	devm_kfree(&pdev->dev, info);
	return 0;
}

static const struct of_device_id test_irq_ids[] = {
	{
		.compatible = "spe,testirq",
	},
	{},
};

static struct platform_driver test_irq_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "test_irq",
		.of_match_table = test_irq_ids,
	},
	.probe = test_irq_probe,
	.remove = test_irq_remove,
};

static int __init test_irq_init(void)
{
	return platform_driver_register(&test_irq_driver);
}

static void __exit test_irq_exit(void)
{
	platform_driver_unregister(&test_irq_driver);
}

module_init(test_irq_init);
module_exit(test_irq_exit);

/******************************************************************************/
MODULE_AUTHOR("Jinhua Wu");
MODULE_DESCRIPTION("test irq driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
