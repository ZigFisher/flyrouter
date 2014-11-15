#!/bin/sh

get() {
  wget http://172.28.200.7/$1 -O $1
}

get i2c-core.o
get i2c-dev.o
get i2c-algo-bit.o
get i2c-mips-gpio.o
get flyavr

insmod i2c-core.o
insmod i2c-algo-bit.o
insmod i2c-dev.o
insmod i2c-mips-gpio.o  gpio_scl=4 gpio_sda=3
