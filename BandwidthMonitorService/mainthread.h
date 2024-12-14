#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include <QThread>
#include <QtServiceBase>

class MainThread : public QThread
{
    Q_OBJECT
public:
    explicit MainThread(QtServiceBase *service, QObject *parent = 0);
    void run();

private:
    QtServiceBase *m_service;
};

#endif // MAINTHREAD_H
