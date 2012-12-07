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

        std::cout << "Forking to background..." << std::endl;
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
            log_->write(e.what(), LogLevel::ERROR);
        }
        retval = RETVAL_FAIL;
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
    log_ = new Log(LogParams("/var/log/psmoveinput.log", loglvl));
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
                                 2,   // TODO: make configurable
                                 *log_);

    // connect handler signals to device slots
    move_signal &moveSignal = handler_->getMoveSignal();
    key_signal &keySignal = handler_->getKeySignal();

    moveSignal.connect(boost::bind(&InputDevice::reportMove, device_, _1, _2));
    keySignal.connect(boost::bind(&InputDevice::reportKey, device_, _1, _2));
}

void PSMoveInput::startListener()
{
    listener_ = new PSMoveListener(*log_, config_.getOpMode());

    // connect listener signals to handler slots
    gyro_signal &gyroSignal = listener_->getGyroSignal();
    button_signal &buttonSignal = listener_->getButtonSignal();

    gyroSignal.connect(boost::bind(&PSMoveHandler::onGyroscope, handler_, _1, _2));
    buttonSignal.connect(boost::bind(&PSMoveHandler::onButtons, handler_, _1, _2));

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
    std::cout << PSMOVEINPUT_VERSION << std::endl;
}

// psmoveinput signal handler
void signal_handler(int sig)
{
    PSMoveInput &input = PSMoveInput::getRef();
    input.stop();
    PSMoveInput::releaseRef();
}

} // namespace psmoveinput
