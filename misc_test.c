#define DRIVER_VERSION		"0.0.1"

#include <linux/module.h>		/* For module specific items */
#include <linux/moduleparam.h>	/* For new moduleparam's */
#include <linux/types.h>		/* For standard types (like size_t) */
#include <linux/compiler.h>
#include <linux/errno.h>		/* For the -ENODEV/... values */
#include <linux/kernel.h>		/* For printk/panic/... */
#include <linux/slab.h>
#include <linux/miscdevice.h>	/* For MODULE_ALIAS_MISCDEV(MISC_DYNAMIC_MINOR) */
#include <linux/platform_device.h>
#include <linux/fs.h>			/* For file operations */
#include <linux/ioport.h>		/* For io-port access */
#include <linux/init.h>			/* For __init/__exit/... */
#include <linux/uaccess.h>		/* For copy_to_user/put_user/... */
#include <asm/io.h>
#include <linux/ioctl.h>
#include "export/misc_test_ioctl.h"
#include <linux/mm.h>

#define BAR_NUM 6

typedef struct pci_mapping pci_mapping_t;

typedef struct pcie_address_space {
	void *bar_addr;
	unsigned long long bar_size;
	void *bar_v_addr;
} pcie_address_space_t;

typedef struct misc_priv {
	switch_port_t port;
	int init_done;
	struct mutex mtx;
	int bar;
	pci_mapping_t *pci_mapping_table;
} misc_priv_t;

typedef struct pcie_priv {
	pcie_address_space_t pci_space[BAR_NUM]; //64位如何处理
	void *pdev;
	pci_mapping_t *pci_mapping_table;
} pcie_priv_t;

struct pci_mapping {
	struct mutex mtx;
	int in_use;
	int sec_bus_no;
	int sub_bus_no;
	int init_done;
	pcie_priv_t *priv;
};

static pci_mapping_t *g_pci_mapping_table = NULL;
static int pci_mapping_done = 0;
static struct mutex pci_mapping_mtx;

int dummy_pci_remove(struct platform_device *pdev)
{
	int i = 0;
	pcie_priv_t *priv = platform_get_drvdata(pdev);
	pci_mapping_t *mapping_table = priv->pci_mapping_table;
	mutex_lock(&mapping_table->mtx);
	mapping_table->in_use = 0;
	mapping_table->priv = NULL;
	mutex_unlock(&mapping_table->mtx);
	for (i = 0; i < BAR_NUM; i++) {
		//if (priv->pci_space[i].bar_v_addr)
		//iounmap(priv->pci_space[i].bar_v_addr);
	}
	kfree(priv);
	return 0;
}

int dummy_pci_probe(struct platform_device *pdev)
{
	int i, bus_no, ret = -1;
	pcie_priv_t *priv;
	struct resource *mem;
	bus_no = pdev->id;
	for (i = 0; i < MISC_MAX_PORT; i++) {
		mutex_lock(&(g_pci_mapping_table[i].mtx));
		if (g_pci_mapping_table[i].init_done && \
			g_pci_mapping_table[i].in_use == 0 && \
			g_pci_mapping_table[i].sec_bus_no <= bus_no && \
			g_pci_mapping_table[i].sub_bus_no >= bus_no) {
			priv = kzalloc(sizeof(pcie_priv_t), GFP_KERNEL);
			if (!priv) {
				mutex_unlock(&g_pci_mapping_table[i].mtx);
				return -ENOMEM;
			}
			priv->pdev = pdev;
			mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
			if (!mem) {
				mutex_unlock(&g_pci_mapping_table[i].mtx);
				return -EINVAL;
			}
			priv->pci_space[0].bar_size = mem->end - mem->start;
			priv->pci_space[0].bar_addr = mem->start;
			priv->pci_space[0].bar_v_addr = phys_to_virt(mem->start);
			priv->pci_mapping_table = &g_pci_mapping_table[i];
			g_pci_mapping_table[i].priv = priv;
			platform_set_drvdata(pdev, priv);
			g_pci_mapping_table[i].in_use = 1;
			ret = 0;
			mutex_unlock(&(g_pci_mapping_table[i].mtx));
			break;
		}
		mutex_unlock(&(g_pci_mapping_table[i].mtx));
	}		
	return ret;
}

static const struct of_device_id dummy_pci_ids[] = {
	{
		.compatible = "dummy_pci",
	},
	{},
};

static struct platform_driver dummy_pci_driver = {
	.driver = {
			   .owner = THIS_MODULE,
			   .name = "dummy_pci",
			   .of_match_table = dummy_pci_ids,
			   },
	.probe = dummy_pci_probe,
	.remove = dummy_pci_remove,
};

misc_priv_t *misc_priv_alloc(void)
{
	misc_priv_t *new_misc = (misc_priv_t *)kzalloc(sizeof(misc_priv_t), GFP_KERNEL);
	if (!new_misc)
		return NULL;
	mutex_init(&(new_misc->mtx));
	return new_misc;
}

int switch_port_mapping(pci_mapping_t *table, pci_port_t *ents, unsigned long num)
{
	unsigned long i = 0;
	pci_mapping_t *temp_table;
	for (i =0 ;i < num; i++) {
		if (ents[i].slot >= MISC_MAX_SLOT || ents[i].bay >= MISC_MAX_BAY)
			return -1;
		temp_table = &table[ents[i].slot * MISC_MAX_BAY + ents[i].bay];
		mutex_lock(&temp_table->mtx);
		temp_table->sec_bus_no = ents[i].sec_bus;
		temp_table->sub_bus_no = ents[i].sub_bus;
		temp_table->init_done = 1;
		mutex_unlock(&temp_table->mtx);
	}
	return 0;
}

static long misc_test_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int i;
	unsigned long num = 0;
	void *map_ents = NULL;
	misc_priv_t *priv = filp->private_data;
	pci_array_t pci_array;
	switch_port_t pos;
	if (_IOC_TYPE(cmd) != MISC_TEST_IOC_MAGIC)
		return -EINVAL;
	mutex_lock(&pci_mapping_mtx);
	if (MISC_TEST_SET_PCIE_MAPPING != cmd && !pci_mapping_done) {
		mutex_unlock(&pci_mapping_mtx);
		return -EPERM;
	}	
	mutex_unlock(&pci_mapping_mtx);
	switch (cmd) {
		case MISC_TEST_SET_PCIE_MAPPING:
			mutex_lock(&pci_mapping_mtx);
			if (pci_mapping_done) {
				mutex_unlock(&pci_mapping_mtx);
				return -EPERM;
			}
			g_pci_mapping_table = kzalloc(sizeof(pci_mapping_t) * MISC_MAX_PORT, GFP_KERNEL);
			if (!g_pci_mapping_table) {
				mutex_unlock(&pci_mapping_mtx);
				return -ENOMEM;
			}
			for ( i = 0; i < MISC_MAX_PORT; i++) {
				mutex_init(&(g_pci_mapping_table[i].mtx));
			}
			if (copy_from_user(&pci_array, (void *)arg, sizeof(pci_array_t))) {
				mutex_unlock(&pci_mapping_mtx);
				return -EFAULT;
			}
			num = pci_array.num;
			if (num > MISC_MAX_PORT)
				num = MISC_MAX_PORT;
			printk("set pcie mapping num = %lu\n", num);
			map_ents = kzalloc(sizeof(pci_port_t) * num, GFP_KERNEL);
			if (!map_ents) {
				mutex_unlock(&pci_mapping_mtx);
				return -ENOMEM;
			}
			if (copy_from_user(map_ents, (void *) pci_array.ents, sizeof(pci_port_t) * num)) {
				mutex_unlock(&pci_mapping_mtx);	
				return -EFAULT;
			}
			if (switch_port_mapping(g_pci_mapping_table, map_ents, num)) {
				mutex_unlock(&pci_mapping_mtx);
				return -EINVAL;
			}
			platform_driver_register(&dummy_pci_driver);
			pci_mapping_done = 1;
			mutex_unlock(&pci_mapping_mtx);
			printk("set pcie mapping done\n");
			break;
		case MISC_TEST_SET_SLOT:
			mutex_lock(&priv->mtx);
			if (priv->init_done) {
				mutex_unlock(&priv->mtx);
				return -EPERM;
			}	
			if (copy_from_user(&pos, (void *)arg, sizeof(switch_port_t))) {
				mutex_unlock(&(priv->mtx));
				return -EFAULT;
			}
			if (pos.slot >= MISC_MAX_SLOT || pos.bay >= MISC_MAX_BAY) {
				return -EFAULT;
			}
			priv->pci_mapping_table = &g_pci_mapping_table[pos.slot * MISC_MAX_BAY + pos.bay];
			priv->init_done = 1;
			mutex_unlock(&priv->mtx);
			break;
		default:
			break;
	}
	return 0;
}

static int misc_test_open(struct inode *inode, struct file *filp)
{
	misc_priv_t *priv = misc_priv_alloc();
	if (!priv)
		return -ENOMEM;
	filp->private_data = priv;
	return 0;
}

static int misc_test_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static int misc_test_mmap(struct file * filp, struct vm_area_struct *vma)
{
	unsigned long pfn, size, paddr;
	misc_priv_t *priv= filp->private_data;
	if (!priv->init_done) {
		return -EPERM;
	}
	if (!priv->pci_mapping_table->in_use) {//lock needed?
		return -ENODEV;
	}
	paddr = (unsigned long) priv->pci_mapping_table->priv->pci_space[priv->bar].bar_addr;
	size  = priv->pci_mapping_table->priv->pci_space[priv->bar].bar_size;

	pfn = paddr >> PAGE_SHIFT;
	if(size > vma->vm_end - vma->vm_start) {
		size = vma->vm_end - vma->vm_start;
	}

	vma->vm_flags |= VM_IO;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* remap kernel memory to userspace */
	if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
		return (-EAGAIN);
	}

	return 0;
}
static const struct file_operations misc_test_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = misc_test_ioctl,
	.open = misc_test_open,
	.mmap = misc_test_mmap,
	.release = misc_test_close,
};

static struct miscdevice misc_test_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "misc_test",
	.fops = &misc_test_fops,
};


static int __init misc_test_init(void)
{
	int ret;
	mutex_init(&pci_mapping_mtx);
	ret = misc_register(&misc_test_miscdev);
	return ret;
}

static void __exit misc_test_exit(void)
{
	misc_deregister(&misc_test_miscdev);
}

module_init(misc_test_init);
module_exit(misc_test_exit);

MODULE_AUTHOR("Jinhua Wu");
MODULE_DESCRIPTION("Misc test driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
