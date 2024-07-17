#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "ioctl_mem"
#define IOCTL_ALLOCATE_MEMORY 100   // ioctl 申请内存空间命令
#define IOCTL_READ_MEMORY     101   // ioctl 读内存空间命令
#define IOCTL_WRITE_MEMORY    102   // ioctl 写内存空间命令

static dev_t dev_num;
static struct cdev cdev = {
	.owner = THIS_MODULE,
};

static char *kernel_memory = NULL;	// 申请的内存空间地址
static size_t memory_size = 0;			// 申请的内存空间大小

static long mem_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case IOCTL_ALLOCATE_MEMORY:
            if (kernel_memory)
                kfree(kernel_memory);

            memory_size = (size_t)arg;
            kernel_memory = kmalloc(memory_size, GFP_KERNEL); // kmalloc 申请memory_size大小的内存空间
            if (!kernel_memory)
                return -ENOMEM;

            memset(kernel_memory, 0, memory_size); // 初始化申请的内存空间
            printk(KERN_ERR "Memory allocated: %zu bytes\n", memory_size);
            break;

        case IOCTL_WRITE_MEMORY:
            if (!kernel_memory)
                return -ENOMEM;

						// 从用户态读内容到内核态
            if (copy_from_user(kernel_memory, (char __user*)arg, memory_size))
                return -EFAULT;

            printk(KERN_ERR "Memory written: %zu bytes\n", memory_size);
            break;

        case IOCTL_READ_MEMORY:
            if (!kernel_memory)
                return -ENOMEM;

						// 从内核态写内容到用户态
            if (copy_to_user((char __user*)arg, kernel_memory, memory_size))
                return -EFAULT;

            printk(KERN_ERR "Memory read: %zu bytes\n", memory_size);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static int mem_open(struct inode *inode, struct file *file)
{
    printk(KERN_ERR "Device opened\n");
    return 0;
}

static int mem_release(struct inode *inode, struct file *file)
{
    printk(KERN_ERR "Device closed\n");
    return 0;
}


static struct file_operations fops = {
    .unlocked_ioctl = mem_ioctl,
    .open = mem_open,
    .release = mem_release,
};

static int __init mem_init (void)
{
	int ret = 0;

	//动态申请设备号
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0){
		printk(KERN_ERR "register_chrdev_region failed!\n");
		return ret;
	}
	
	// 注册cdev
	//初始化
	cdev_init(&cdev, &fops);
	//将cdev添加到内核
	cdev_add(&cdev, dev_num, 1);

	printk(KERN_ERR "Device /dev/&s registered success \n", DEVICE_NAME);
	return 0;
}

static void __exit mem_exit (void)
{
	// 释放申请的内存
	if (kernel_memory)
		kfree(kernel_memory);

	// 注销设备
	unregister_chrdev_region(dev_num, 1);
	cdev_del(&cdev);
	printk(KERN_ERR "Device /dev/&s unregistered success \n", DEVICE_NAME);
	return ;
}

module_init (mem_init);
module_exit (mem_exit);
MODULE_LICENSE ("GPL");
