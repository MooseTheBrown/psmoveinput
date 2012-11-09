/* 
 * Copyright (C) 2012 Mikhail Sapozhnikov
 *
 * This file is part of psmoveinput.
 *
 * psmoveinput is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * psmoveinput is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with psmoveinput.  If not, see <http://www.gnu.org/licenses/>.
 *
 */



#ifndef PSMOVEINPUT_PSMOVEINPUT_HPP
#define PSMOVEINPUT_PSMOVEINPUT_HPP

#include "config.hpp"
#include "log.hpp"
#include "input_device.hpp"
#include "psmove_handler.hpp"
#include "psmove_listener.hpp"
#include "except.hpp"

namespace psmoveinput
{

// return values
#define RETVAL_OK       0   // successfull termination
#define RETVAL_FAIL     1   // general error
#define RETVAL_CONFIG   2   // invalid config
#define RETVAL_ALREADY  3   // psmoveinput instance is already running
#define RETVAL_FORK     4   // forking to background failed
#define RETVAL_PID      5   // couldn't write to pidfile

// input device name
#define INPUT_DEVICE_NAME "psmoveinput"

class PSMoveInput
{
public:
    int run(int argc, char **argv);
    void stop();

    static PSMoveInput &getRef();
    static void releaseRef();

protected:
    Config config_;
    Log *log_;
    InputDevice *device_;
    PSMoveHandler *handler_;
    PSMoveListener *listener_;

    static PSMoveInput *instance_;
    static int refs_;

    PSMoveInput();
    virtual ~PSMoveInput();

    int handleException(Exception &e);
    void processConfig(int argc, char **argv);
    void setupLog();
    void checkPidFile();
    void daemonize();
    void writePidFile();
    void removePidFile();
    void initDevice();
    void initHandler();
    void startListener();
    void setupSignals();
    void print_version();

    PSMoveInput(const PSMoveInput &) = delete;
    PSMoveInput& operator = (const PSMoveInput &) = delete;
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_PSMOVEINPUT_HPP
