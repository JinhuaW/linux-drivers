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
#include <linux/mm.h>
#include <asm/atomic.h>
#include <linux/dma-mapping.h>

static dma_addr_t dma_handle;
static void *buffer = NULL;
static int size = 0x900000;
static int cma_test_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int cma_test_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static int cma_test_mmap(struct file * filp, struct vm_area_struct *vma)
{
	return 0;
}
static long cma_test_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return 0;
}
static const struct file_operations cma_test_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = cma_test_ioctl,
	.open = cma_test_open,
	.mmap = cma_test_mmap,
	.release = cma_test_close,
};

static struct miscdevice cma_test_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "cma_test",
	.fops = &cma_test_fops,
};
int __init cma_test_init(void)
{
	int ret = 0;
	printk(KERN_INFO "cma_test_init!\n");
	ret = misc_register(&cma_test_dev);
	if (ret) {
		printk(KERN_INFO "misc_resiter failed, ret = %d\n", ret);
		return ret;
	}
	buffer = dma_alloc_coherent(NULL, size, &dma_handle, GFP_KERNEL | __GFP_COMP);
	if (NULL == buffer) {
		misc_deregister(&cma_test_dev);
		return -ENOMEM;
	}
	return ret;
}
void __exit cma_test_exit(void)
{
	misc_deregister(&cma_test_dev);
	if (buffer != NULL)
		dma_free_coherent(NULL, size, buffer, dma_handle);
	printk(KERN_INFO "cma_test_exit!\n");
}
module_param(size, uint , S_IRUGO);

module_init(cma_test_init);
module_exit(cma_test_exit);
