# 
# Copyright (C) 2012, 2013 Mikhail Sapozhnikov
#
# This file is part of psmoveinput.
#
# psmoveinput is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# psmoveinput is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with psmoveinput.  If not, see <http://www.gnu.org/licenses/>.
#



#!/bin/sh

if [ -z "$1" ]; then
    exit 1
fi

result="Unknown"

for input_dev in `ls /sys/class/input`; do
    cd "/sys/class/input/$input_dev/device"
    dev_name=`cat name`
    if [ "$dev_name" = "$1" ]; then
        cd ../
        result=`cat uevent | awk -F '=' '/DEVNAME/ {print $2}'`
        break
    fi
done

if [ "$result" = "Unknown" ]; then
    exit 1
else
    echo "$result"
    exit 0
fi
