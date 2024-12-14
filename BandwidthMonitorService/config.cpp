#include "config.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>

#define CONFIG_FILE "config.ini"
#define BACKUP_FILE "config.bak"

QString Config::m_address = "";
QString Config::m_broadcast = "";
int Config::m_delay = 0;
QJsonObject Config::m_sync;


bool Config::load()
{
    // get config file path
    QString configpath = QCoreApplication::applicationDirPath();
    configpath += "/";
    configpath += CONFIG_FILE;

    // read data from config file
    QFile file(configpath);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        file.setFileName(QCoreApplication::applicationDirPath() + "/" + BACKUP_FILE);
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return false;
    }
    QByteArray data = file.readAll();
    file.close();

    // parse json format and load config information
    QJsonObject config_data = QJsonDocument::fromJson(data).object();
    if(config_data.isEmpty())
    {
        file.setFileName(QCoreApplication::applicationDirPath() + "/" + BACKUP_FILE);
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return false;
        data = file.readAll();
        file.close();
        config_data = QJsonDocument::fromJson(data).object();
    }
    m_address = config_data["serveraddress"].toString();
    m_delay = config_data["delay"].toInt();
    m_sync = config_data["sync"].toObject();
    m_broadcast = config_data["broadcast"].toString();

    qDebug() << "server address: " << m_address;
    qDebug() << "dealy: " << m_delay;
    qDebug() << "sync: " << m_sync;
    qDebug() << "broadcast: " << m_broadcast;

    return true;
}


void Config::save()
{
    // get config file path
    QString configpath = QCoreApplication::applicationDirPath();
    configpath += "/";
    configpath += CONFIG_FILE;

    // make json object with config information
    QJsonObject data;
    data["serveraddress"] = m_address;
    data["delay"] = m_delay;
    data["sync"] = m_sync;
    data["broadcast"] = m_broadcast;
    QJsonDocument config_data(data);

    // save to config file
    QFile file(configpath);
    file.open(QFile::WriteOnly | QFile::Text);
    file.write(config_data.toJson());
    file.close();
}
