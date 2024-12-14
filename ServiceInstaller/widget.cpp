#include "widget.h"
#include "ui_widget.h"

#include <QMessageBox>
#include <QProcess>

#define SERVICENAME "BandwidthMonitorService"
#define SERVICEFILE "BandwidthMonitorService.exe"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // create controller
    m_controller = new QtServiceController(SERVICENAME);

    // check service is installed or not
    if(m_controller->isInstalled())
    {
        ui->installButton->setDisabled(true);
        ui->uninstallButton->setEnabled(true);
    }
    else
    {
        ui->installButton->setEnabled(true);
        ui->uninstallButton->setDisabled(true);
    }
}

Widget::~Widget()
{
    delete ui;
    delete m_controller;
}

void Widget::on_installButton_clicked()
{
    // get service file path
    QString servicepath = QCoreApplication::applicationFilePath();
    servicepath.replace("/", "\\");
    servicepath = servicepath.left(servicepath.lastIndexOf("\\"));
    servicepath += "\\";
    servicepath += SERVICEFILE;

    // Install Service
    if(!m_controller->install(servicepath))
    {
        QMessageBox::warning(this, "Error", "Failed to install service");
        return;
    }

    // Run Service
    if(!m_controller->start())
    {
        QMessageBox::warning(this, "Error", "Failed to start service");
        return;
    }

    ui->installButton->setDisabled(true);
    ui->uninstallButton->setEnabled(true);

    QMessageBox::about(this, "ServiceInstaller", "Successfully install service");
}

void Widget::on_uninstallButton_clicked()
{
    if(m_controller->isRunning())
    {
        QProcess process;
        process.start(QString("taskkill -im ") + SERVICEFILE + " -f");
        process.waitForFinished();
    }

    if(!m_controller->uninstall())
    {
        QMessageBox::warning(this, "Error", "Failed to uninstall network monitor service.");
        return;
    }

    ui->installButton->setEnabled(true);
    ui->uninstallButton->setDisabled(true);

    QMessageBox::about(this, "ServiceInstaller", "Successfully uninstall service.");
}
