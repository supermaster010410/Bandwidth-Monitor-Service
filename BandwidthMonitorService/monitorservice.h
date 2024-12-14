#ifndef MONITORSERVICE_H
#define MONITORSERVICE_H

#include <QtService>
#include "mainthread.h"

/*
Service class that measure bandwidth usage and send it to server using thread
*/

class MonitorService : public QtService<QCoreApplication>
{
public:
    MonitorService(int argc, char **argv, QString servicename);

protected:
    void start(); // function run when service start

private:
    MainThread *m_thread;
};

#endif // MONITORSERVICE_H
