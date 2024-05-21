#!/bin/sh /etc/rc.common
ln -sf /lib/ld-*.so /usr/lib/ld.so.1
insmod /lib/modules/4.1.8/linux-kernel-bde.ko
insmod /lib/modules/4.1.8/linux-user-bde.ko
mknod /dev/linux-user-bde c 126 0
/usr/sbin/robodiag &
exit
