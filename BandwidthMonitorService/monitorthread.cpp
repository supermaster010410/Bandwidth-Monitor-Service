#include "config.h"
#include "monitorthread.h"
#include "pcap.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QSettings>

#define ADDR_STR_MAX 128


MonitorThread::MonitorThread(QObject *parent) : QThread(parent)
{

}


const char* iptos(struct sockaddr *sockaddr) // convert ip address to string
{
    static char address[ADDR_STR_MAX] = {0};
    int gni_error = 0;

    gni_error = getnameinfo(sockaddr, sizeof(struct sockaddr_storage), address, ADDR_STR_MAX, NULL, 0, NI_NUMERICHOST);
    if (gni_error != 0)
    {
        qDebug() << "getnameinfo: " << gni_error;
        return "ERROR!";
    }

    return address;
}


bool checkAdapter(pcap_if_t *d) // check local adapter or not
{
    pcap_addr_t *a;

    // check broadcast IP addresses
    for(a = d->addresses; a; a = a->next)
        if (a->addr->sa_family > 0 && a->broadaddr && strncmp(iptos(a->broadaddr), Config::m_broadcast.toLocal8Bit(), Config::m_broadcast.length()) == 0)
            return true;

    return false;
}

QDateTime getCurrentTime()
{
    QNetworkAccessManager manager;
    QNetworkRequest request;
    QNetworkReply *reply;
    QEventLoop eventloop;

    // get Tokyo time from internet
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventloop, SLOT(quit()));
    request.setUrl(QUrl("http://worldtimeapi.org/api/timezone/Asia/Tokyo"));
    reply = manager.get(request);
    eventloop.exec();

    // check send successfully or not
    if (reply->error() == QNetworkReply::NoError)
    {
        // get current time
        QJsonParseError error;
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll(), &error).object();
        if(error.error == QJsonParseError::NoError)
        {
            delete reply;
            return QDateTime::fromString(obj["datetime"].toString(), Qt::ISODate);
        }
    }
    delete reply;

    // get system time
    QDateTime systime = QDateTime::currentDateTimeUtc();
    systime = systime.addSecs(3600 * 9); // convert to Tokyo time
    return systime;
}


void sendInfo(QString ip, qlonglong usage, qlonglong runtime)
{
    // get current time
    QDateTime current = getCurrentTime();
    int i;

    // make a data send to the server
    QJsonObject obj, infos, info;
    obj["ip"] = ip;
    info["usage"] = usage;
    info["runtime"] = runtime;
    infos[current.toString("yyyy/MM/dd")] = info;
    if(!Config::m_sync.empty())
    {
        QStringList keys = Config::m_sync.keys();
        for(i = 0; i < Config::m_sync.length(); i++)
            infos[keys[i]] = Config::m_sync[keys[i]];
    }
    obj["infos"] = infos;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    // send data to server
    QEventLoop eventloop;
    QNetworkAccessManager manager;

    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventloop, SLOT(quit()));

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + Config::m_address + ":8000/network/add"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = manager.post(request, data);
    eventloop.exec();

    // check send successfully or not
    if (reply->error() == QNetworkReply::NoError) {
        //success
        qDebug() << "Success" <<reply->readAll();
        delete reply;

        // update config
        if (!Config::m_sync.empty())
        {
            QStringList keys = Config::m_sync.keys();
            for(i = Config::m_sync.length() - 1; i >= 0 ; i--)
                Config::m_sync.remove(keys[i]);
        }
        Config::save();
    }
    else {
        //failure
        qDebug() << "Failure" <<reply->errorString();
        delete reply;

        // update config
        if (Config::m_sync.contains(current.toString("yyyy/MM/dd")))
        {
            info = Config::m_sync[current.toString("yyyy/MM/dd")].toObject();
            info["usage"] = info["usage"].toInt() + usage;
            info["runtime"] = info["runtime"].toInt() + runtime;
            Config::m_sync[current.toString("yyyy/MM/dd")] = info;
        }
        else
        {
            info["usage"] = usage;
            info["runtime"] = runtime;
            Config::m_sync[current.toString("yyyy/MM/dd")] = info;
        }
        Config::save();
    }
}


void MonitorThread::run()
{
    pcap_if_t *alldevs; // all network device connected to computer
    pcap_if_t *adapter; // adapter to capture packets
    int res; // capture result

    pcap_t *fp; // file pointer for adapter
    char errbuf[PCAP_ERRBUF_SIZE]; // error message
    u_int netmask; // network mask
    struct bpf_program fcode;

    struct pcap_pkthdr *header; // header of packet
    u_char *pkt_data; // packet data

    long old_tvc; // timestamp
    bool firstflag = true; // true: first loop, false: otherwise

    // Initialize socket
    WSADATA wsadata;
    int err = WSAStartup(MAKEWORD(2, 2), &wsadata);

    if (err != 0)
    {
        qDebug() << "WSAStartup failed " << err;
        return;
    }

    // Retrieve the device list from the local machine
    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        qDebug() << "Error in pcap findalldevs ex: " << errbuf;
        return;
    }

    if(alldevs == NULL)
    {
        qDebug() << "No interfaces found! Make sure WinPcap is installed.";
        return;
    }

    // find the local network adapter
    for (adapter = alldevs; adapter != NULL && checkAdapter(adapter) == false; adapter = adapter->next);

    qDebug() << "Network Adapter Name: " << adapter->name;

    // Open the output adapter
    if ( (fp= pcap_open(adapter->name, 100, PCAP_OPENFLAG_PROMISCUOUS, Config::m_delay, NULL, errbuf) ) == NULL)
    {
        qDebug() << "Unable to open adapter " << errbuf;
        return;
    }

    // Don't care about netmask, it won't be used for this filter
    netmask=0xffffff;

    // get host ip address
    char hostaddress[ADDR_STR_MAX];
    strcpy(hostaddress, iptos(adapter->addresses->addr));
    qDebug() << "Host IP Address: " << hostaddress;

    // make filter string
    char filter[200] = "(src host ";

    strcat(filter, hostaddress);
    strcat(filter, " and not dst net 192.168.80) or (not src net 192.168.80 and dst host ");
    strcat(filter, hostaddress);
    strcat(filter, ")");
    qDebug() << "filter: " << filter;

    //compile the filter
    if (pcap_compile(fp, &fcode, filter, 1, netmask) <0 )
    {
        qDebug() << "Unable to compile the packet filter. Check the syntax.";
        // Free the device list
        pcap_freealldevs(alldevs);
        return;
    }

    //set the filter
    if (pcap_setfilter(fp, &fcode)<0)
    {
        qDebug() << "Error setting the filter.";
        pcap_close(fp);
        // Free the device list
        pcap_freealldevs(alldevs);
        return;
    }

    // Put the interface in statstics mode
    if (pcap_setmode(fp, MODE_STAT)<0)
    {
        qDebug() << "Error setting the mode.";
        pcap_close(fp);
        // Free the device list
        pcap_freealldevs(alldevs);
        return;
    }

    qDebug() << "TCP traffic summary:";

    pcap_freealldevs(alldevs);

    // Start the main loop
    while((res = pcap_next_ex(fp, &header, (const u_char**)&pkt_data)) >= 0)
    {
        if(firstflag)
        {
            firstflag=false;
            old_tvc = header->ts.tv_sec - 10;
        }

        if(res == 0)
            continue;

        // Calculate the delay in microseconds from the last sample.
        // Get the number of Bytes per second
        qlonglong usage = (*(LONGLONG*)(pkt_data + 8));
        qlonglong runtime = header->ts.tv_sec - old_tvc;

        // Print the bandwidth usage and time
        qDebug() << "BPS=" << usage << "Time = " << runtime;
        old_tvc = header->ts.tv_sec;

        // snd infomation
        sendInfo(QString(hostaddress), usage, runtime);
    }

    pcap_close(fp);
}
