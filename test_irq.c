#include <linux/module.h>		/* Needed by all modules */
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/printk.h>

struct test_irq_info {
	int irq;
	struct platform_device *pdev;
};


static irqreturn_t test_irq_handler(int irq, void *arg)
{
	struct platform_device *pdev = (struct platform_device *) arg;
	printk("test irq comes ..., pdev = 0x%p\n", pdev);
	return IRQ_HANDLED;
}

static int test_irq_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *of_node = dev->of_node;
	struct test_irq_info *test_irq;
	int ret,irq;

	/* Allocate test_irq_info data structure */
	test_irq = devm_kzalloc(dev, sizeof(*test_irq), GFP_KERNEL);
	if (!test_irq) {
		dev_err(dev, "cannot allocate test_irq resource\n");
		return -ENOMEM;
	}

	irq = of_irq_to_resource(of_node, 0, NULL);
	if (unlikely(irq == NO_IRQ)) {
		dev_warn(&pdev->dev,"couldn't obtain test interrupt information from device tree\n");
	}
	test_irq->irq = irq;

	dev_err(dev, "test_irq = %d\n", irq);
	ret = devm_request_irq(dev, irq, test_irq_handler, IRQF_SHARED, "test_irq", pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to request test_irq %d\n", test_irq->irq);
		test_irq->irq = -1;
		return -EINVAL;
	} 
	dev_dbg(dev, "registering interrupt %d, pdev = 0x%p\n", irq, pdev);

	platform_set_drvdata(pdev, test_irq);
	test_irq->pdev = pdev;
	return 0;
}

static int test_irq_remove(struct platform_device *pdev)
{
	struct test_irq_info *info = platform_get_drvdata(pdev);
	free_irq(info->irq, pdev);
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
