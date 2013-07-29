Description
-----------
psmoveinput is a userspace Linux input driver that uses data from PSMove motion
controller to inject events to kernel's input subsystem thus making it possible
to use PSMove as a combined mouse/keyboard input device. It gives users ability
to control mouse pointer movements by moving PSMove controller and map PSMove
keys to common keyboard keys.

Dependencies
------------
psmoveinput requires the following software to be installed:

- [psmoveapi](http://thp.io/2010/psmove/)
- uinput module (modprobe uinput as root)
- bluez
- boost-thread
- boost-program-options
- python (recent versions of both branches 2 and 3 should work fine)

bluez and boost libraries should be present in most distributions' repos.
psmoveapi should be compiled and installed from source.

Installation
------------
See [INSTALL.md](./INSTALL.md) for instructions on how to build and install psmoveinput.

Configuring and running
-----------------------
psmoveinput needs to be run by root user in order to interact with uinput module.
psmoveinput recognizes several command-line options and reads its configuration
from a file. The location of configuration file is specified with -c or --config
command line options. During psmoveinput installation default configuration
file is installed to /etc/psmoveinput.conf. This file contains description of
all configurable options.
To get the list of all recognized command line options run psmoveinput with
-h or --help argument.

Connecting PSMove controller
----------------------------
1. Connect PSMove to PC via USB.
2. Run "psmovepair" as root. Notice controller's Bluetooth address. psmovepair
   is part of psmoveapi (see [INSTALL.md](./INSTALL.md) for details).
3. Disconnect the controller.
4. Press the PS button on the PSMove controller. You may see popup indicating
   Bluetooth connection attempt from the controller (depends on desktop environment
    used). Authorize it and wait until the PSMove is connected.
5. Run "psmoveinput" as root. It will automatically connect to the controller and
   start injecting input events.

This procedure is necessary only when connecting controller to the system for
the first time. After disconnecting controller it can be reconnected by pressing
PS button. However, if the same controller is then paired to another system
(including Sony PlayStation itself), the procedure has to be repeated in order
to reconnect controller to the current system.

psmoveinput supports up to two simultaneously connected controllers. Controller,
which is connected first, is used to control the mouse pointer as well as to
report key events, while the second one is used only for keys and gestures.
There are four gestures supported: up, down, left and right. Each of them can be
mapped to any key supported by psmoveinput (see the default config file provided
with psmoveinput for details).

License
-------
GNU GPLv3 or any later version (see [COPYING](./COPYING)).
Google Test Framework provided with psmoveinput is covered by [different license](./gtest-1.6.0/COPYING).

Credits
-------
Thanks to Thomas Perl and all who contributed to psmoveapi library.
