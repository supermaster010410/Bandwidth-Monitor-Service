#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QJsonObject>

class Config
{
public:
    static int m_delay; // delay time to send and measure information
    static QString m_address; // server address
    static QString m_broadcast; // broadcast address
    static QJsonObject m_sync; // sync data


public:
    Config();
    static bool load(); // load config data from setting file
    static void save(); // save config data to setting file
};

#endif // CONFIG_H
