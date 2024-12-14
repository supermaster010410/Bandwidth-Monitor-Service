#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H

#include <QThread>

class MonitorThread : public QThread
{
    Q_OBJECT
public:
    explicit MonitorThread(QObject *parent = 0);
    void run();
};

#endif // MONITORTHREAD_H
