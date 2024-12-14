#include "mainthread.h"
#include "monitorthread.h"

#include <QNetworkAccessManager>


MainThread::MainThread(QtServiceBase *service, QObject *parent) : QThread(parent)
{
    m_service = service;
}


void MainThread::run()
{
    MonitorThread *thread = NULL; // thread to measure and send bandwidth usage to server

    while(true)
    {
        QNetworkAccessManager manager; // networkaccessmanager to check network accessiblity
        if(manager.networkAccessible() == QNetworkAccessManager::Accessible && (thread == NULL || thread->isFinished())) // network is accessible and monitorthread is terminated
        {
            qDebug() << "start monitoring";
            m_service->logMessage("start monitoring");

            // start thread
            if(thread != NULL)
            {
                delete thread;
                thread = NULL;
            }
            thread = new MonitorThread;
            thread->start();
        }
        else if(manager.networkAccessible() == QNetworkAccessManager::NotAccessible && thread != NULL) // network is not accessible and monitorthread is running
        {
            qDebug() << "stop monitoring";
            m_service->logMessage("stop monitoring");
            // terminate thread
            thread->terminate();;
            delete thread;
            thread = NULL;
        }
        sleep(1);
    }
}
