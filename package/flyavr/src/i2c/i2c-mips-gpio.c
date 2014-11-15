/* ------------------------------------------------------------------------- */
/* i2c-mips-gpio.c i2c-hw access                                             */
/* ------------------------------------------------------------------------- */

/*  Improved version by Michael Schoeberl, 2008
      - fixed the I2C bus handling 
      - requires only 2 pins (SCL/SDA) 
      - both pins needs a 10 kOhm pullup
    This is based on previous versions done by John Newbigin, Simon G. Vogl 
    and Nekmech aka datenritter.
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */
 
 
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/stddef.h>

#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>



#define DEFAULT_GPIO_SCL 4
#define DEFAULT_GPIO_SDA 3


// this is just the default, those are module paramers
static int gpio_scl = DEFAULT_GPIO_SCL;
static int gpio_sda = DEFAULT_GPIO_SDA; 

#define GPIO_SCL (1 << gpio_scl)
#define GPIO_SDA (1 << gpio_sda)

#ifndef __exit
#define __exit __init
#endif
 
typedef unsigned int uint32;
static volatile uint32 *gpioaddr_input = (uint32 *)0xb8000060;
static volatile uint32 *gpioaddr_output = (uint32 *)0xb8000064;
static volatile uint32 *gpioaddr_enable = (uint32 *)0xb8000068;
static volatile uint32 *gpioaddr_control = (uint32 *)0xb800006c;
 


// for the line to go high we need to release it (there is a pullup!) 
static void bit_gpio_set(unsigned int mask) {
        unsigned int port_state;

	// signal quality tweak: we drive it high for a short
        // time and then release it - this improves rise time
        port_state = *gpioaddr_output;
        port_state |= mask;
        *gpioaddr_output = port_state;

        port_state = *gpioaddr_enable;
        port_state &= ~mask;
        *gpioaddr_enable = port_state;
}
 
static void bit_gpio_clear(unsigned int mask) {
        unsigned int port_state;

	// set pin to output and drive it to low
        port_state = *gpioaddr_enable;
        port_state |= mask;
        *gpioaddr_enable = port_state;

        port_state = *gpioaddr_output;
        port_state &= ~mask;
        *gpioaddr_output = port_state;
}
 
static int bit_gpio_get(int mask) {
        unsigned char port_state;
        port_state = *gpioaddr_input;
        return port_state & mask;
}
 
static void bit_gpio_setscl(void *data, int state) {
        if (state) {
                bit_gpio_set(GPIO_SCL);
        } else {
                bit_gpio_clear(GPIO_SCL);
        }
}
 
static void bit_gpio_setsda(void *data, int state) {
        if (state) {
                bit_gpio_set(GPIO_SDA);
        } else {
                bit_gpio_clear(GPIO_SDA);
        }
}
 
static int bit_gpio_getscl(void *data) {
        return bit_gpio_get(GPIO_SCL);
}
 
static int bit_gpio_getsda(void *data) {
        return bit_gpio_get(GPIO_SDA);
}
 
 
static int bit_gpio_reg(struct i2c_client *client) {
        return 0;
}
 
static int bit_gpio_unreg(struct i2c_client *client) {
        return 0;
}
 
static void bit_gpio_inc_use(struct i2c_adapter *adap) {
        MOD_INC_USE_COUNT;
}
 
static void bit_gpio_dec_use(struct i2c_adapter *adap) {
        MOD_DEC_USE_COUNT;
}
 
//  Encapsulate the above functions in the correct operations structure.
//  This is only done when more than one hardware adapter is supported.
static struct i2c_algo_bit_data bit_gpio_data = {
        NULL,
        bit_gpio_setsda,
        bit_gpio_setscl,
        bit_gpio_getsda,
        bit_gpio_getscl,
        80,      //  udelay, half-clock-cycle time in microsecs, i.e. clock is (500 / udelay) KHz
	80,      //  mdelay, in millisecs, unused 
	100,     //  timeout, in jiffies
};
 
static struct i2c_adapter bit_gpio_ops = {
        "I2C GPIO",
        I2C_HW_B_LP, // bit algorithm adapter "Phillips style"
        NULL,
        &bit_gpio_data,
        bit_gpio_inc_use,
        bit_gpio_dec_use,
        bit_gpio_reg,
        bit_gpio_unreg,
};
 
int __init i2c_bitgpio_init(void) {
        unsigned char gpio_outen;

        printk(KERN_INFO "i2c-mpis-gpio.o: I2C GPIO module, SCL %d, SDA %d\n", gpio_scl, gpio_sda);

	// I'm still not sure why we do this, probably this needs cleanup on exit(?) 
        gpio_outen = *gpioaddr_control;
        gpio_outen = gpio_outen & ~(GPIO_SCL | GPIO_SDA);
        *gpioaddr_control = gpio_outen;
	
        // set to high=input as default idle state
	bit_gpio_set(GPIO_SCL);
        bit_gpio_set(GPIO_SDA);
 
	// register bus
        if(i2c_bit_add_bus(&bit_gpio_ops) < 0)
                return -ENODEV;
 
        return 0;
}

void __exit i2c_bitgpio_exit(void) {
	// set to high=input as default idle state
	bit_gpio_set(GPIO_SCL);
	bit_gpio_set(GPIO_SDA);

        i2c_bit_del_bus(&bit_gpio_ops);
}
 
module_init(i2c_bitgpio_init);
module_exit(i2c_bitgpio_exit);
EXPORT_NO_SYMBOLS;

MODULE_PARM(gpio_scl,"i");
MODULE_PARM_DESC(gpio_scl, "Number of GPIO wire used for SCL (default=5).");
MODULE_PARM(gpio_sda,"i");
MODULE_PARM_DESC(gpio_sda, "Number of GPIO wire used for SDA (default=6).");
 
MODULE_AUTHOR("Michael Schoeberl <openwrt@mailtonne.de>");
MODULE_DESCRIPTION("I2C-Bus adapter routines for GPIOs in openwrt");
MODULE_LICENSE("GPL");
 
#ifdef MODULE
int init_module(void) {
        return i2c_bitgpio_init();
}
 
void cleanup_module(void) {
        i2c_bitgpio_exit();
}
#endif 

