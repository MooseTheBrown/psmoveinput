#!/usr/bin/python
# -*- coding: utf-8 -*-
# 
# Copyright (C) 2012 Mikhail Sapozhnikov
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

import dbus
import sys

if len(sys.argv) != 2:
    print >>sys.stderr, """
    Usage: %s <btaddr>

    Disconnect Bluetooth device with given address.
    """ % sys.argv[0]
    sys.exit(1)

address = sys.argv[1]

system_bus = dbus.SystemBus()

manager_object = system_bus.get_object('org.bluez', '/')
manager = dbus.Interface(manager_object, 'org.bluez.Manager')
adapter_path = manager.DefaultAdapter()
adapter_object = system_bus.get_object('org.bluez', adapter_path)
adapter = dbus.Interface(adapter_object, 'org.bluez.Adapter')
device_name = adapter.FindDevice(address)
device_object = system_bus.get_object('org.bluez', device_name)
device = dbus.Interface(device_object, 'org.bluez.Device')
device.Disconnect()
