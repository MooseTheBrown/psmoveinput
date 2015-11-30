/*
 * Copyright (C) 2012 - 2015 Mikhail Sapozhnikov
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



#include "psmoveinput.hpp"
#include "config.h"
#include "file_log.hpp"
#include <iostream>
#include <unistd.h>
#include <boost/format.hpp>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <exception>

namespace psmoveinput
{

extern "C"
{
    static void signal_handler(int sig);
}

PSMoveInput *PSMoveInput::instance_ = nullptr;
int PSMoveInput::refs_ = 0;

PSMoveInput &PSMoveInput::getRef()
{
    if (instance_ == nullptr)
    {
        instance_ = new PSMoveInput();
    }
    refs_++;
    return *instance_;
}

void PSMoveInput::releaseRef()
{
    refs_--;
    if (refs_ == 0)
    {
        delete instance_;
        instance_ = nullptr;
    }
}

PSMoveInput::PSMoveInput() :
    log_(nullptr),
    device_(nullptr),
    handler_(nullptr),
    listener_(nullptr)
{
}

PSMoveInput::~PSMoveInput()
{
    if (log_ != nullptr)
    {
        delete log_;
    }
    if (device_ != nullptr)
    {
        delete device_;
    }
    if (handler_ != nullptr)
    {
        delete handler_;
    }
    if (listener_ != nullptr)
    {
        delete listener_;
    }
}

int PSMoveInput::run(int argc, char **argv)
{
    int retval = RETVAL_OK;

    try
    {
        processConfig(argc, argv);

        std::cout << "Starting psmoveinput" << std::endl;

        setupLog();
        checkPidFile();

        daemonize();

        setupSignals();

        initDevice();
        initHandler();

        // launch psmove listener, it will automatically report events to the handler,
        // and the handler will forward them to the input device
        startListener();

        // time to quit
        removePidFile();
    }
    catch (Exception &e)
    {
        retval = handleException(e);
    }
    catch (std::exception &e)
    {
        if (log_)
        {
            log_->write(e.what(), LogLevel::FATAL);
        }
        retval = RETVAL_FAIL;
        removePidFile();
    }

    return retval;
}

int PSMoveInput::handleException(Exception &e)
{
    int retval = RETVAL_FAIL;

    switch (e.what())
    {
        case ex_type::INVALID_CONFIG:
        {
            std::cout << "Invalid configuration, exiting" << std::endl;
            retval = RETVAL_CONFIG;
            break;
        }
        case ex_type::ALREADY_RUNNING:
        {
            std::cout << "An instance of psmoveinput is already running. ";
            std::cout << "Pidfile: " << config_.getPidFileName() << std::endl;
            retval = RETVAL_ALREADY;
            break;
        }
        case ex_type::FORK_FAILED:
        {
            std::cout << "Forking failed" << std::endl;
            retval = RETVAL_FORK;
            break;
        }
        case ex_type::PARENT_QUIT:
        {
            // parent process is quitting
            retval = RETVAL_OK;
            break;
        }
        case ex_type::PIDFILE_CREAT_FAILED:
        {
            log_->write("Failed to write pid to pid file", LogLevel::FATAL);
            retval = RETVAL_PID;
            break;
        }
        case ex_type::HELP_RQ:
        {
            retval = RETVAL_OK;
            break;
        }
        case ex_type::VERSION_RQ:
        {
            print_version();
            retval = RETVAL_OK;
            break;
        }
        default:
            break;
    }

    return retval;
}

void PSMoveInput::processConfig(int argc, char **argv)
{
    config_.parse(argc, argv);
    if (!config_.isOK())
    {
        throw Exception(ex_type::INVALID_CONFIG);
    }

    if (config_.helpRequested())
    {
        throw Exception(ex_type::HELP_RQ);
    }

    if (config_.versionRequested())
    {
        throw Exception(ex_type::VERSION_RQ);
    }
}

void PSMoveInput::setupLog()
{
    LogLevel loglvl = config_.getLogLevel();
    log_ = new Log(LogParams(config_.getLogFileName(), loglvl));
    FileLog *logBackend = new FileLog();
    log_->addBackend(logBackend);
    log_->write("PSMoveInput logging started");
}

void PSMoveInput::checkPidFile()
{
    const char *pidfile = config_.getPidFileName();
    int fd = open(pidfile, O_RDONLY);
    if (fd >= 0)
    {
        // file already exists
        close(fd);
        throw Exception(ex_type::ALREADY_RUNNING);
    }
}

void PSMoveInput::daemonize()
{
    pid_t pid;

    if (config_.getForeground() == false) // don't fork in foreground mode
    {
        std::cout << "Forking to background..." << std::endl;
        pid = fork();
        if (pid < 0)
        {
            throw Exception(ex_type::FORK_FAILED);
        }

        if (pid > 0)
        {
            // exit from the parent
            throw Exception(ex_type::PARENT_QUIT);
        }

        umask(0);
        pid_t sid = setsid();
        if (sid < 0)
        {
            throw Exception(ex_type::FORK_FAILED);
        }

        if (chdir("/") < 0)
        {
            throw Exception(ex_type::FORK_FAILED);
        }

        std::freopen("/dev/null", "r", stdin);
        std::freopen("/dev/null", "w", stdout);
        std::freopen("/dev/null", "w", stderr);
    }

    writePidFile();
}

void PSMoveInput::writePidFile()
{
    const char *pidfile = config_.getPidFileName();
    // create file with 0644 permissions
    int fd = creat(pidfile, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1)
    {
        throw Exception(ex_type::PIDFILE_CREAT_FAILED);
    }

    char buf[32];
    pid_t pid = getpid();
    std::snprintf(buf, 32, "%d", pid);
    write(fd, static_cast<const void*>(buf), std::strlen(buf));

    close(fd);
}

void PSMoveInput::removePidFile()
{
    unlink(config_.getPidFileName());
}

void PSMoveInput::initDevice()
{
    key_array deviceKeys;

    log_->write("Initializing input device");
    log_->write("Reported keys:");

    for (KeyMapEntry entry : config_.getKeyMap(ControllerId::FIRST))
    {
        deviceKeys.push_back(entry.lincode);
        log_->write(boost::str(boost::format("pscode=%1%, lincode=%2%") % entry.pscode % entry.lincode).c_str());
    }
    for (KeyMapEntry entry : config_.getKeyMap(ControllerId::SECOND))
    {
        deviceKeys.push_back(entry.lincode);
        log_->write(boost::str(boost::format("pscode=%1%, lincode=%2%") % entry.pscode % entry.lincode).c_str());
    }

    device_ = new InputDevice(INPUT_DEVICE_NAME, deviceKeys, *log_);
}

void PSMoveInput::initHandler()
{
    log_->write("Initializing PSMoveHandler");

    handler_ = new PSMoveHandler(config_.getKeyMap(ControllerId::FIRST),
                                 config_.getKeyMap(ControllerId::SECOND),
                                 config_.getMoveCoeffs(),
                                 config_.getMoveThreshold(),
                                 config_.getGestureThreshold(),
                                 *log_);

    // connect handler signals to device slots
    move_signal &moveSignal = handler_->getMoveSignal();
    key_signal &keySignal = handler_->getKeySignal();
    mwheel_signal &mwheelSignal = handler_->getMWheelSignal();

    moveSignal.connect(boost::bind(&InputDevice::reportMove, device_, _1, _2));
    keySignal.connect(boost::bind(&InputDevice::reportKey, device_, _1, _2));
    mwheelSignal.connect(boost::bind(&InputDevice::reportMWheel, device_, _1));
}

void PSMoveInput::startListener()
{
    listener_ = new PSMoveListener(*log_,
                                   config_.getOpMode(),
                                   config_.getPollTimeout(),
                                   config_.getConnTimeout(),
                                   config_.getDisconnectTimeout(),
                                   config_.getLedTimeout(),
                                   config_.getGestureTimeout());

    // connect listener signals to handler slots
    gyro_signal &gyroSignal = listener_->getGyroSignal();
    gyro_signal &gestureSignal = listener_->getGestureSignal();
    button_signal &buttonSignal = listener_->getButtonSignal();
    disconnect_complete_signal &disconnectCompleteSignal = listener_->getDisconnectCompleteSignal();

    gyroSignal.connect(boost::bind(&PSMoveHandler::onGyroscope, handler_, _1, _2));
    gestureSignal.connect(boost::bind(&PSMoveHandler::onGesture, handler_, _1, _2));
    buttonSignal.connect(boost::bind(&PSMoveHandler::onButtons, handler_, _1, _2));
    disconnectCompleteSignal.connect(boost::bind(&PSMoveHandler::reset, handler_));

    // connect handler's disconnect signal to listener's slot
    disconnect_signal &disconnectSignal = handler_->getDisconnectSignal();
    disconnectSignal.connect(boost::bind(&PSMoveListener::onDisconnectKey, listener_, _1));

    /* NOTE: There are two disconnect signals. One belongs to PSMoveHandler and is used to
       notify PSMoveListener on disconnect button being pressed on one of the controllers.
       Another is PSMoveListener's "disconnect complete" signal. It is raised when all
       controller threads are disconnected (as a result of either disconnect key press or
       expiring disconnect timeout), and PSMoveHandler has to reset its internal state. */

    listener_->run();
}

void PSMoveInput::stop()
{
    if (listener_ != nullptr)
    {
        listener_->stop();
    }
}

void PSMoveInput::setupSignals()
{
    struct sigaction act;
    sigset_t sigset;

    // set of signals to be blocked during signal handling
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGQUIT);
    sigaddset(&sigset, SIGTERM);

    act.sa_handler = signal_handler;
    act.sa_mask = sigset;
    act.sa_flags = 0;

    sigaction(SIGINT, &act, nullptr);
    sigaction(SIGQUIT, &act, nullptr);
    sigaction(SIGTERM, &act, nullptr);
}

void PSMoveInput::print_version()
{
    std::cout << "psmoveinput version " << PSMOVEINPUT_VERSION << "\n";
    std::cout << "psmoveinput is a Linux input driver turning PSMove controller\n\
into mouse/keyboard input device\n\n";
    std::cout << "Copyright (C) 2012 - 2015  Mikhail Sapozhnikov (masapozhnikov@yandex.ru)\n";
    std::cout << "This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>." << std::endl;
}

// psmoveinput signal handler
void signal_handler(int sig)
{
    PSMoveInput &input = PSMoveInput::getRef();
    input.stop();
    PSMoveInput::releaseRef();
}

} // namespace psmoveinput
