#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

static int hello_open (struct inode *inode, struct file *file);
static int hello_release (struct inode *inode, struct file *file);
static long hello_ioctl (struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t hello_read (struct file *file, char __user * buf, size_t count, loff_t * offset);
static ssize_t hello_write (struct file *file, const char __user * buf, size_t count, loff_t * offset);

#define BUFFER_SIZE 1024
static char device_buffer[BUFFER_SIZE];
#define DEVICENAME "hello"

static const struct file_operations hello_fops = {
  .owner = THIS_MODULE,
  .open = hello_open,
  .release = hello_release,
  .unlocked_ioctl = hello_ioctl,
  .read = hello_read,
  .write = hello_write
};

struct mychar_device_data
{
  struct cdev cdev;
};

static int dev_major = 0;
static struct class *hello_class = NULL;
static struct mychar_device_data hello_dev;

static int hello_uevent (struct device *dev, struct kobj_uevent_env *env)
{
  add_uevent_var (env, "DEVMODE=%#o", 0666);
  return 0;
}

static int __init hello_init (void)
{
  int err;
  dev_t dev;

  err = alloc_chrdev_region (&dev, 0, 0, DEVICENAME);

  if (err < 0)
    {
      printk (KERN_ALERT " hello : can't get major number\n");
      return err;
    }

  dev_major = MAJOR (dev);
  printk (KERN_INFO "hello : major number: %d\n", dev_major);

  hello_class = class_create (THIS_MODULE, DEVICENAME);
  hello_class->dev_uevent = hello_uevent;

  cdev_init (&hello_dev.cdev, &hello_fops);
  hello_dev.cdev.owner = THIS_MODULE;

  err = cdev_add (&hello_dev.cdev, MKDEV (dev_major, 0), 1);

  if (err < 0)
    {
      printk (KERN_ALERT "hello : Fatal error\n");
      return err;
    }

  device_create (hello_class, NULL, MKDEV (dev_major, 0), NULL, DEVICENAME, 0);

  return 0;
}

static void __exit hello_exit (void)
{
  device_destroy (hello_class, MKDEV (dev_major, 0));
  class_unregister (hello_class);
  class_destroy (hello_class);

  unregister_chrdev_region (MKDEV (dev_major, 0), MINORMASK);
}

static int hello_open (struct inode *inode, struct file *file)
{
  printk ("hello: Device open\n");
  return 0;
}

static int hello_release (struct inode *inode, struct file *file)
{
  printk ("hello: Device close\n");
  return 0;
}

static long hello_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
  printk ("hello: Device ioctl\n");
  return 0;
}

static ssize_t hello_read (struct file *file, char __user * buf, size_t length, loff_t * offset)
{
  int bytes_to_read;
  int bytes_read;
  int maxbytes = BUFFER_SIZE - *offset;
  if (maxbytes > length)
    bytes_to_read = length;
  else
    bytes_to_read = maxbytes;
  if (bytes_to_read == 0)
    printk (KERN_INFO "hello : fine\n");

  bytes_read = bytes_to_read - copy_to_user (buf, device_buffer + *offset, bytes_to_read);
  printk (KERN_INFO "hello : letti %d\n", bytes_read);

  *offset += bytes_read;

  return bytes_read;
}

static ssize_t hello_write (struct file *file, const char __user * buf, size_t length, loff_t * offset)
{
  int bytes_to_write;
  int bytes_written;
  int maxbytes = BUFFER_SIZE - *offset;
  if (maxbytes > length)
    bytes_to_write = length;
  else
    bytes_to_write = maxbytes;

  bytes_written =
    bytes_to_write - copy_from_user (device_buffer + *offset, buf,
				     bytes_to_write);
  printk (KERN_INFO "hello : scritti %d\n", bytes_written);
  *offset += bytes_written;
  return bytes_written;
}

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("");

module_init (hello_init);
module_exit (hello_exit);
