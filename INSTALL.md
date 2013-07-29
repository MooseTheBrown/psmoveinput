Pre-requisites
--------------
1. psmoveapi. See http://thp.io/2010/psmove/ for instructions on how to get the
   source, build and install it.
   psmoveapi provides psmovepair utility, which is required to connect PSMove controller
   to PC via Bluetooth. psmovepair is installed into <install_prefix>/bin directory,
   and should be present in PATH after psmoveapi is installed.

2. Boost libraries. At least boost-thread and boost-program-options have to be installed.
   You will also need boost development headers (usually tha package is called boost-devel).

3. Common development headers (glibc-devel package or something similar depending on
   distribution used).

4. cmake, make

5. gcc 4.6.x or 4.7.x

6. git

Getting the sources
-------------------
git clone https://github.com/MooseTheBrown/psmoveinput.git

Building the source
-------------------
Use the following sequence of commands to build psmoveinput:

1. cd psmoveinput
2. cmake
   Optionally specify installation prefix by running "cmake -D CMAKE_INSTALL_PREFIX=<prefix>"
3. make

You can build psmoveinput in a separate directory from where the sources are located.
To do this create new directory manually and invoke cmake from it:
    mkdir psmoveinput-build
    cd psmoveinput-build
    cmake ../psmoveinput

Installing
-------------------------------------------------------------------------------
Run "make install" as root from the directory, where psmoveinput was built.

Running unit tests
-------------------------------------------------------------------------------
If you are a developer and want to contribute to psmoveinput, you'll probably
want to run unit tests if you change existing functionality or add new tests
if you create new features.
The tests are executed by separate runner, which has to be built first. To generate
make target for building the runner run "cmake -D BUILD_UNIT_TESTS=ON". Then 
run "make psmoveinput-test". This will produce file "psmoveinput-test". Launch it
to run the tests.
psmoveinput utilizes Google Test Framework for unit tests. For information regarding
how to write the tests and add them to the psmoveinput test suite, refer to [Google
Test documentation](http://code.google.com/p/googletest/wiki/Documentation)

