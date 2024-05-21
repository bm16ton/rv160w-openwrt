#!/bin/sh

kill_led() {
  for p in `pgrep -f rv16x_26x_led.sh | xargs`
	do
    LED=`cat /proc/$p/cmdline | grep $1`
		if [ $LED == "$1" ]; then
#      echo -n "Killing $LED process with PID $p..."
      kill -9 $p > /dev/null
      echo "done."
	  else
      echo "NOT killing LED process PID $p"
    fi
  done
}

kill_led $1
