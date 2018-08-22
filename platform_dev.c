#include <linux/module.h> 
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <asm/page.h>
#include <linux/mm.h>

MODULE_LICENSE("Dual BSD/GPL");

static void *dev_buffer = NULL;
struct resource test_res0[] = {
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};
struct resource test_res1[] = {
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};
struct resource test_res2[] = {
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};
struct resource test_res3[] = {
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};
static void device_release(struct device *dev)
{
	printk("platform: device release\n");
}

struct platform_device test_device[4] = {
	{
		.id = 0,
		.name = "dummy_pci",
		.dev.release = device_release,
		.num_resources =ARRAY_SIZE(test_res0),
		.resource = test_res0,
	},
	{
		.id = 1,
		.name = "dummy_pci",
		.dev.release = device_release,
		.num_resources =ARRAY_SIZE(test_res1),
		.resource = test_res1,
	},
	{
		.id = 2,
		.name = "dummy_pci",
		.dev.release = device_release,
		.num_resources =ARRAY_SIZE(test_res2),
		.resource = test_res2,
	},
	{
		.id = 3,
		.name = "dummy_pci",
		.dev.release = device_release,
		.num_resources =ARRAY_SIZE(test_res3),
		.resource = test_res3,
	}
};

static int __init pci_test_platform_init(void)
{
	int i = 0;
	unsigned long phys;
	dev_buffer = kzalloc(PAGE_SIZE * 4, GFP_KERNEL);
	printk("dev init\n");
	if (!dev_buffer) {
		printk("alloc mem failed\n");
		return -1;
	}
	phys = virt_to_phys(dev_buffer);
	for (i = 0; i < 4; i++) {
		test_device[i].resource[0].start = phys + PAGE_SIZE * i;
		test_device[i].resource[0].end = phys + PAGE_SIZE * (i + 1) - 1;
		printk("dev%d: resource start at 0x%x, end at 0x%x\n", i, test_device[i].resource[0].start, test_device[i].resource[0].end); 
		platform_device_register(&test_device[i]);
	}
	return 0;
}

static void __exit pci_test_platform_exit(void)
{
	int i;
	for (i = 0; i < 4; i++) {
		platform_device_unregister(&test_device[i]);
	}
	if (dev_buffer)
		kfree(dev_buffer);
}

module_init(pci_test_platform_init);
module_exit(pci_test_platform_exit);
