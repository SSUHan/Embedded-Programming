// FPGA Text LCD Ioremap Control
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

#define IOM_FPGA_TEXT_LCD_MAJOR 263 // ioboard lcd device major number
#define IOM_FPGA_TEXT_LCD_NAME "fpga_text_lcd" // lcd device name
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x07000100 // physical address - 32byte (16*2)
#define IOM_FPGA_DEMO_ADDRESS 0x07000300
#define UON 0x00 // IOM
#define UOFF 0x01 // IOM

//Global Variables
static int fpga_text_lcd_port_usage  = 0; // 현재 이 lcd 포트가 사용되고 있는지 아닌지 확인하는 lock 역할을 함
static unsigned char * iom_fpga_text_lcd_addr; // text lcd 의 물리주소 -> 가상주소 로 받는 역할
static unsigned char * iom_fpga_demo_addr; // demo addr

// pre Defined Functions
ssize_t iom_fpga_text_lcd_write(struct file * inode, const char *gdata, size_t length, loff_t * off_what);
int iom_fpga_text_lcd_open(struct inode * minode, struct file * mfile);
int iom_fpga_text_lcd_release(struct inode * minode, struct file * mfile);

// define file_operations structure
struct file_operations iom_fpga_text_lcd_fops =
{
	.owner	=	THIS_MODULE,
	.open 	= 	iom_fpga_text_lcd_open,
	.write 	= 	iom_fpga_text_lcd_write,
	.release = 	iom_fpga_text_lcd_release, 
};

// when fpga_text_lcd device open, call this function
int iom_fpga_text_lcd_open(struct inode * minode, struct file * mfile){
	if(fpga_text_lcd_port_usage != 0){
		// 사용되고 있다면
		return -EBUSY;
	}
	// 아니라면 사용표시 해놓고,
	fpga_text_lcd_port_usage = 1;
	return 0;
}

int iom_fpga_text_lcd_release(struct inode * minode, struct file * mfile){
	// 이제 text lcd port 를 사용하지 않으므로, 0 으로 바꾼다
	fpga_text_lcd_port_usage = 0;
	return 0;
}

ssize_t iom_fpga_text_lcd_write(struct file * inode, const char * gdata, size_t length, loff_t * off_what){
	int i;
	unsigned char value[32];
	const char * tmp = gdata;

	if(copy_from_user(&value, tmp, length)) // gdata : user, value : kernel
		return -EFAULT;
	value[length] = 0; // NULL
	printk("Get Size : %d / String : %s\n", length, value); // Kernel Debugging

	for(i=0; i<length; i++){
		outb(value[i], (unsigned int)iom_fpga_text_lcd_addr + i); // 가상주소의 한칸씩 늘려가면서, value 값을 찍는다.
	}
	return length;
}

// module init function
int __init iom_fpga_text_lcd_init(){
	int result;
	// character device 를 Magor num 와 이름과 fops 구조체를 등록한다.
	result = register_chrdev(IOM_FPGA_TEXT_LCD_MAJOR, IOM_FPGA_TEXT_LCD_NAME, &iom_fpga_text_lcd_fops);
	if(result < 0){
		printk(KERN_WARNING "Can't get any major\n");
		return result;
	}

	// physical address -> virtual address mapping
	iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32); // [physical address, address size]
	iom_fpga_demo_addr = ioremap(IOM_FPGA_DEMO_ADDRESS, 0x01);

	// demo test
	outb(UON, (unsigned int)iom_fpga_demo_addr);
	printk("Init module : %s Major number : %d\n", IOM_FPGA_TEXT_LCD_NAME, IOM_FPGA_TEXT_LCD_MAJOR);
	return 0;

}

void __exit iom_fpga_text_lcd_exit(){
	// io un mapping routine
	iounmap(iom_fpga_text_lcd_addr);

	// unregister chr dev
	unregister_chrdev(IOM_FPGA_TEXT_LCD_MAJOR, IOM_FPGA_TEXT_LCD_NAME);
}

module_init(iom_fpga_text_lcd_init); // kernel 버전 3.0 이상부터 사용하는 init 함수 등록 매크로
module_exit(iom_fpga_text_lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BrianLee");
