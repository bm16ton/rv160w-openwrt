#!/bin/sh

# This script is required to do some task only 1 time after firmware upgrade.

POSTUPGRADE_FLAG="/usr/bin/postupgrade/flag"
postupgradeFlag=`cat $POSTUPGRADE_FLAG`

if [ "$postupgradeFlag" -eq 1 ];then
        #  Do all the things here, which need to be done after upgrade.
        #  If any file (binary or data) is required, put them in postupgrade folder. They can be accessed from "/usr/bin/postupgrade" folder.

        # Set the flag to 0
        echo "0" > $POSTUPGRADE_FLAG
fi
