// FND device control driver
// Author : Brian Lee

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/version.h>

#define FND_MAJOR 261 
#define FND_NAME "fpga_fnd"
#define FND_ADDRESS 0x07000004 // physical address
#define DEMO_ADDRESS 0x07000300 // demo physical address
#define UON 0x00
#define UOFF 0x01

// Global Variables
static int fnd_port_usage = 0;
static unsigned char * fnd_addr; // fnd virtual address
static unsigned char * demo_addr;

// Functions
int __init fnd_init();
void __exit fnd_exit();
int fnd_open(struct inode * minode, struct file * mfile);
int fnd_release(struct inode * minode, struct file * mfile);
ssize_t fnd_write(struct file * inode, const char * gdata, size_t length, loff_t * off_what);
ssize_t fnd_read(struct file * inode, char * gdata, size_t length, loff_t * off_what);

// file_operations structure
struct file_operations fnd_fops =
{
	// .owner = THIS_MODULE,
	.open 	=	fnd_open,
	.release = 	fnd_release,
	.write 	= 	fnd_write,
	.read 	= 	fnd_read
};

int fnd_open(struct inode * minode, struct file * mfile){
	if(fnd_port_usage != 0)
		return -EBUSY;
	fnd_port_usage = 1;
	return 0;
}

int fnd_release(struct inode * minode, struct file * mfile){
	fnd_port_usage = 0;
	return 0;
}


int __init fnd_init(){
	int result = register_chrdev(FND_MAJOR, FND_NAME, &fnd_fops);
	if(result < 0){
		return result;
	}

	fnd_addr = ioremap(FND_ADDRESS, 0x04); // [physical address, addr size]
	demo_addr = ioremap(DEMO_ADDRESS, 0x01);

	outb(UON, (unsigned int)demo_addr);

	return 0;
}

ssize_t fnd_write(struct file * inode, const char * gdata, size_t length, loff_t * off_what){
	int i;
	unsigned char value[4];
	const char * temp = gdata;

	if(copy_from_user(&value, temp, length)){
		return -EFAULT;
	}
	for(i=0; i<length; i++){
		outb(value[i], (unsigned int)fnd_addr + i);
	}
	return length;
}

ssize_t fnd_read(struct file * inode, char * gdata, size_t length, loff_t * off_what){
	int i;
	unsigned char value[4];
	char * temp = gdata;

	for(i=0;i<length; i++){
		value[i] = inb((unsigned int)fnd_addr + i);
	}
	if(copy_to_user(temp, value, length))
		return -EFAULT;

	return length;
}

void __exit fnd_exit(){
	iounmap(fnd_addr)
	iounmap(demo_addr)
	unregister_chrdev(FND_MAJOR,FND_NAME);
}

module_init(fnd_init);
module_exit(fnd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BrainLee");




