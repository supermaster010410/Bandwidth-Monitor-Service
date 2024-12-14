#include <QCoreApplication>
#include "monitorservice.h"

int main(int argc, char *argv[])
{
    // run service
    MonitorService service(argc, argv, "BandwidthMonitorService");
    return service.exec();
}

