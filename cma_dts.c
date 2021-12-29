#include <linux/module.h>		/* Needed by all modules */
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/printk.h>
#include <linux/of_reserved_mem.h>
typedef struct cma_addr {
	void *v_addr;
	dma_addr_t p_addr;
} cma_addr_t;

uint32_t size = 0x100000;

module_param(size, uint, S_IRUGO);

static int cma_test_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	cma_addr_t *cma_info = devm_kzalloc(dev, sizeof(cma_addr_t), GFP_KERNEL);
	if (!cma_info) {
		dev_err(dev, "cannot allocate memory\n");
		return -ENOMEM;
	}

	ret = of_reserved_mem_device_init(dev);
	if(ret) {
		dev_err(dev, "cannot get resereved memory\n");
		return -ENOMEM;
	}

	cma_info->v_addr = dma_alloc_coherent(dev, size, &(cma_info->p_addr), GFP_KERNEL | __GFP_COMP);
	if (NULL == cma_info->v_addr) {
		printk(KERN_INFO "dma_alloc failed\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, cma_info);
	printk(KERN_INFO "cma_test physical addr to 0x%p!\n", (void *)cma_info->p_addr);
	return 0;
}

static int cma_test_remove(struct platform_device *pdev)
{
	cma_addr_t *cma_info = platform_get_drvdata(pdev);
	dma_free_coherent(&pdev->dev, size, cma_info->v_addr, cma_info->p_addr);
	return 0;
}

static const struct of_device_id cma_test_ids[] = {
	{
		.compatible = "spe,cma_test",
	},
	{},
};

static struct platform_driver cma_test_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "cma_test",
		.of_match_table = cma_test_ids,
	},
	.probe = cma_test_probe,
	.remove = cma_test_remove,
};

static int __init cma_test_init(void)
{
	return platform_driver_register(&cma_test_driver);
}

static void __exit cma_test_exit(void)
{
	platform_driver_unregister(&cma_test_driver);
}

module_init(cma_test_init);
module_exit(cma_test_exit);

/******************************************************************************/
MODULE_AUTHOR("Jinhua Wu");
MODULE_DESCRIPTION("CMA test driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
