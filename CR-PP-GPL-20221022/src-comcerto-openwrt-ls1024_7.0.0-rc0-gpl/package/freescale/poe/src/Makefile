# ************************************************************************  
# 
#   Description: make file for PoE .
# 
# 
#*************************************************************************

TOP_DIR := $(shell pwd)
 
obj-m  += poedrv.o

all:
	$(MAKE) -C $(KERNEL_SOURCE) ARCH=arm SUBDIRS="$(TOP_DIR)" modules


clean:
	make ARCH=arm -C $(KERNEL_SOURCE) M=$(PWD) clean
