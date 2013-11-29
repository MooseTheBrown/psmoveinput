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



cmake_minimum_required (VERSION 2.6)

project (psmoveinput)

# do we need to build unit tests?
option (BUILD_UNIT_TESTS "generate targets to build unit tests" OFF)

# install psmoveinput udev rule or not
option (INSTALL_UDEV_RULE "install psmoveinput udev rule" OFF)

set (COMMON_CXX_FLAGS "-std=c++0x")
set (CMAKE_BUILD_TYPE RelWithDebInfo)
set (CMAKE_LINKER_FLAGS -ldl)
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${COMMON_CXX_FLAGS} -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE ${COMMON_CXX_FLAGS})
set (COMMON_LINK_LIBS psmoveapi
                      boost_thread-mt
                      boost_program_options-mt
                      boost_system
                      rt)

include_directories (${psmoveinput_SOURCE_DIR})

set (PSMOVEINPUT_VERSION_MAJOR "0")
set (PSMOVEINPUT_VERSION_MINOR "3")
set (PSMOVEINPUT_VERSION_PATCH "0")
configure_file (${psmoveinput_SOURCE_DIR}/config.h.in
                ${psmoveinput_SOURCE_DIR}/config.h)

# main target configuration
set (PSMOVEINPUT_SRC_NOMAIN input_device.cpp
                            psmove_handler.cpp
                            config.cpp
                            conf_keymap_parser.cpp
                            log.cpp
                            file_log.cpp
                            psmove_listener.cpp
                            psmoveinput.cpp)
set (PSMOVEINPUT_SRC ${PSMOVEINPUT_SRC_NOMAIN} main.cpp)
add_executable (psmoveinput ${PSMOVEINPUT_SRC})
target_link_libraries (psmoveinput ${COMMON_LINK_LIBS})

# installation
set (PSMOVEINPUT_BINARY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                    WORLD_READ WORLD_EXECUTE)
install (TARGETS psmoveinput DESTINATION bin)
install (FILES config/psmoveinput.conf DESTINATION /etc)
install (FILES util/psmoveinput_disconnect.py DESTINATION bin PERMISSIONS ${PSMOVEINPUT_BINARY_PERMISSIONS})
if (INSTALL_UDEV_RULE)
    install (FILES config/99-psmoveinput.rules DESTINATION /etc/udev/rules.d)
endif (INSTALL_UDEV_RULE)

# unit test
if (BUILD_UNIT_TESTS)
    # Google test framework
    set (GTEST_NAME "gtest-1.6.0")
    add_subdirectory (${psmoveinput_SOURCE_DIR}/${GTEST_NAME} ${psmoveinput_BINARY_DIR}/${GTEST_NAME})
    include_directories (${psmoveinput_SOURCE_DIR}/${GTEST_NAME}/include)

    configure_file (${psmoveinput_SOURCE_DIR}/test/test_config.h.in
                    ${psmoveinput_SOURCE_DIR}/test/test_config.h)
        
    # the tests
    set (PSMOVEINPUT_UT_SRC ${PSMOVEINPUT_SRC_NOMAIN}
                            ${psmoveinput_SOURCE_DIR}/test/main.cpp
                            ${psmoveinput_SOURCE_DIR}/test/input_device_test.cpp
                            ${psmoveinput_SOURCE_DIR}/test/psmove_handler_test.cpp
                            ${psmoveinput_SOURCE_DIR}/test/psmove_handler_mt_test.cpp
                            ${psmoveinput_SOURCE_DIR}/test/config_test.cpp
                            ${psmoveinput_SOURCE_DIR}/test/log_test.cpp )
    add_executable (psmoveinput-test EXCLUDE_FROM_ALL ${PSMOVEINPUT_UT_SRC})
    target_link_libraries (psmoveinput-test ${COMMON_LINK_LIBS} gtest)
endif (BUILD_UNIT_TESTS)
