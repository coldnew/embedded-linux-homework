#include <linux/init.h> /*包含用于声明装载模块初始化和清除函数的宏*/
#include <linux/kernel.h> /*printk()*/
#include <linux/module.h> /*明确指定是模块*/
#include <linux/fs.h> /*申请设备号,file_operation*/
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("ggarlic");
MODULE_LICENSE("GPL");

struct char_dev *char_device;
int dev_major=0;
int dev_minor=0;

char procfs_name[32] = "mychardev";

char str[32];
int integer;
struct proc_dir_entry *Our_Proc_File;

static int char_open(struct inode *inode,struct file *filep)
{   
    try_module_get(THIS_MODULE);
    return 0;
}


static ssize_t char_read(struct file * filep, char __user *buf, size_t count, loff_t *offset)
{
    if (count != sizeof(str) / sizeof(*str)) {
        printk(KERN_ALERT "String length is not 32?\n");
        return (-EINVAL);
    }

    copy_to_user(buf, str, count);

    return count ;
}


static ssize_t char_write(struct file * filep, const char __user *buf, size_t count, loff_t *offset)
{
    if (count != sizeof(str) / sizeof(*str)) {
        printk(KERN_ALERT "String length is not 32?\n");
        return (-EINVAL);
    }

    copy_from_user(str, buf, count);

    return count ;
} 

static long char_ioctl(struct file * filep, unsigned int cmd, unsigned long arg)
{
    integer = cmd;

    return 0;
}

static int char_procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
    char content[64];
    int length;
    int ret;

    length = sprintf(content, "%s\n%i\n", str, integer);

    if (offset >= length)
    {
        *eof = 1;
        ret = 0;
    }
    else
        ret = snprintf(buffer, buffer_length, "%s", content + offset);

    return ret;
}

static struct file_operations char_ops={
    .owner = THIS_MODULE,
    .open = char_open,
    .read = char_read,
    .write = char_write,
//   .release = char_release,
    .unlocked_ioctl = &char_ioctl //since 2.6.35 the ioctl is removed, use unlocked_ioctl instead
}; 

//设备初始化
static int dev_init(void)
{
    int result = 0;

    integer = 0;
    memset(str, 0, sizeof(str));

    result=register_chrdev(dev_major, "char_dev", &char_ops);

    if(result < 0)
    {
        printk(KERN_ALERT "can't get major %d\n",dev_major);
        return result;
    }
    //返回主设备号
    printk("the dev_major %d\n",dev_major);

    Our_Proc_File = create_proc_entry(procfs_name, 0644, NULL);
    if (Our_Proc_File == NULL) 
    {
        remove_proc_entry(procfs_name, NULL);
        unregister_chrdev(dev_major, "char_dev");
        printk(KERN_ALERT "error initializing /proc/%s\n", procfs_name);
        return (-ENOMEM);
    }

    Our_Proc_File->read_proc = char_procfile_read;
 //   Our_Proc_File->owner = THIS_MODULE;
    Our_Proc_File->mode = S_IFREG | S_IRUGO;
    Our_Proc_File->uid = 0;
    Our_Proc_File->gid = 0;
    Our_Proc_File->size = 64;

    return 0;
}

//设备被移出时调用
static void dev_exit(void)
{
    dev_t devno=0;

    devno = MKDEV(dev_major,dev_minor);
    unregister_chrdev(dev_major, "char_dev");

    remove_proc_entry(procfs_name, NULL);
}

//注册模块初始化和卸载函数
module_init(dev_init);
module_exit(dev_exit);

