#include "config.h"
#include "monitorservice.h"

MonitorService::MonitorService(int argc, char **argv, QString servicename): QtService<QCoreApplication>(argc, argv, servicename)
{
    setServiceDescription("BandwidthMonitorService-1.0.0"); // set service description
    setServiceFlags(QtService::CannotBeStopped); // Make a servce can't be stopped and paused
    setStartupType(QtServiceController::AutoStartup); // Make a service run automatically when boot the computer
}


void MonitorService::start()
{
    logMessage("start"); // log message to windows event viewer

    // load config information
    if(!Config::load())
    {
        // stop service when fail to load config file
        logMessage("Failed to load config file");
        application()->quit();
        return;
    }

    // create main thread and run
    m_thread = new MainThread(this);
    m_thread->start();
}
