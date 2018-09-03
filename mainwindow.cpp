#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QIcon>
#include <QTimer>
#include "settings.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_initializing(true)
{
    ui->setupUi(this);
    setupTelemetyReader();
    setupTrayIcon();
    readSettings();
    setupSerialPortList();

    m_telemetryReader.run();
    showAppStartedMessage();
    m_initializing = false;
}

MainWindow::~MainWindow()
{
    disconnect(this);
    delete ui;
}

void MainWindow::setupTelemetyReader()
{
    (void)connect(&m_telemetryReader, &TelemetryReader::error, this, &MainWindow::onError);
    (void)connect(&m_telemetryReader, &TelemetryReader::setStatus, this, &MainWindow::onSetStatus);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontLeftStatusUpdated, this, &MainWindow::onFrontLeftStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontRightStatusUpdated, this, &MainWindow::onFrontRightStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::rearLeftStatusUpdated, this, &MainWindow::onRearLeftStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::rearRightStatusUpdated, this, &MainWindow::onRearRightStatusUpdated);
}

void MainWindow::setupTrayIcon()
{
    createActions();
    createTrayIcon();

    m_trayIcon->setIcon(QIcon("icon.png"));
    m_trayIcon->show();
    (void)connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
    (void)connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::showWindow);
}

void MainWindow::readSettings()
{
    Settings* settings = Settings::getInstance();

    // UPS
    qint32 ups = qBound(0, settings->getUps(), 120);
    qDebug() << "Setting update rate:" << ups << "ups";
    ui->upsSpinBox->setValue(ups);
    m_telemetryReader.setUpdatesPerSecond(ups);

    // Minimize with X
    if (settings->getMinimizeWithX())
    {
        ui->minimizeWindowCheckBox->setCheckState(Qt::CheckState::Checked);
    }
    else
    {
        ui->minimizeWindowCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

void MainWindow::hideEvent(QHideEvent * event)
{
    m_minimizeAction->trigger();
    event->ignore();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (Settings::getInstance()->getMinimizeWithX())
    {
        m_minimizeAction->trigger();
        event->ignore();
    }
}

void MainWindow::createActions()
{
    m_restoreAction = new QAction("Show window", this);
    (void)connect(m_restoreAction, &QAction::triggered, this, &MainWindow::showNormal);

    m_minimizeAction = new QAction("Minimize window", this);
    (void)connect(m_minimizeAction, &QAction::triggered, this, &MainWindow::hide);

    m_quitAction = new QAction("Quit", this);
    (void)connect(m_quitAction, &QAction::triggered, this, &MainWindow::quit);
}

void MainWindow::createTrayIcon()
{
    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addAction(m_minimizeAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setToolTip("Pedal vibration");
}

void MainWindow::showAppStartedMessage()
{
    m_trayIcon->showMessage("Pedal vibration", "Pedal vibration started in tray", m_trayIcon->icon(), 100);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            this->showWindow();
            break;
        default:
            break;
    }
}

void MainWindow::showWindow()
{
    m_restoreAction->trigger();
    this->raise();
    this->activateWindow();
}

void MainWindow::onError(const QString &error)
{
    ui->statusLabel->setText("An error occured: " + error);
}

QList<Port> MainWindow::getAvailableSerialPorts()
{
    QList<Port> serialPorts;
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    for (qint32 i = 0; i < portList.size(); ++i)
    {   
        serialPorts.append(Port(portList.at(i).portName(), portList.at(i).description()));
    }

    return serialPorts;
}

void MainWindow::setupSerialPortList()
{
    QList<Port> serialPortList = getAvailableSerialPorts();
    if (serialPortList.isEmpty())
    {
        m_selectedSerialPortIndex = -1;
        m_serialPorts.clear();
        ui->portsComboBox->clear();
        ui->portsComboBox->setEnabled(false);
        return;
    }

    ui->portsComboBox->setEnabled(true);
    m_serialPorts = serialPortList;
    QString port = Settings::getInstance()->getPort();
    for (qint32 i = 0; i < m_serialPorts.size(); ++i)
    {
        ui->portsComboBox->addItem(m_serialPorts[i].getDesignator());
        if (m_serialPorts[i].portName == port)
        {
            m_selectedSerialPortIndex = i;
        }
    }

    if (m_selectedSerialPortIndex == -1)
    {
        m_selectedSerialPortIndex = 0;
    }

    ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
}

//void MainWindow::refreshSerialPortList()
//{
//    qDebug() << "Refreshing serial port list";
//    QList<Port> newSerialPortList = getAvailableSerialPorts();
//    qDebug() << newSerialPortList;

//    m_port.clear();

//    if (newSerialPortList.isEmpty())
//    {
//        m_selectedSerialPortIndex = -1;
//        m_serialPorts.clear();
//        ui->portsComboBox->clear();
//        ui->portsComboBox->setEnabled(false);
//        return;
//    }

//    ui->portsComboBox->setEnabled(true);

//    if (m_selectedSerialPortIndex != -1)
//    {
//        m_port = m_serialPorts.at(m_selectedSerialPortIndex);
//    }

//    ui->portsComboBox->clear();

//    if (!m_port.isEmpty())
//    {
//        qint32 newIndex = newSerialPortList.indexOf(m_port);
//        if (newIndex == -1)
//        {
//            m_selectedSerialPortIndex = -1;
//            ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
//            m_port.clear();
//            return;
//        }

//        m_selectedSerialPortIndex = newIndex;
//    }

//    m_serialPorts = newSerialPortList;

//    ui->portsComboBox->addItems(m_serialPorts);
//    ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
//    m_port = m_serialPorts.at(m_selectedSerialPortIndex);

//    // If no port was selected before, select the first one by default
//    if (m_selectedSerialPortIndex == -1)
//    {
//        m_selectedSerialPortIndex = 0;
//        ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
//        m_port = m_serialPorts.at(m_selectedSerialPortIndex);
//    }
//}

void MainWindow::onSetStatus(const AC_STATUS &status)
{
    QString statusText;
    switch (status)
    {
    case AC_OFF:
        qDebug() << "Status changed: OFF";
        statusText = "acs.exe is not running";
        clearWheelSlipIndicators();
        break;
    case AC_REPLAY:
        qDebug() << "Status changed: REPLAY";
        statusText = "Replay running";
        clearWheelSlipIndicators();
        break;
    case AC_LIVE:
        qDebug() << "Status changed: LIVE";
        statusText = "Live! Sending telemetry data";
        break;
    case AC_PAUSE:
        qDebug() << "Status changed: PAUSE";
        statusText = "Game is paused";
        clearWheelSlipIndicators();
        break;
    }

    ui->statusLabel->setText(statusText);
}

void MainWindow::clearWheelSlipIndicators()
{
    ui->frontLeftLineEdit->setText("--");
    ui->frontRightLineEdit->setText("--");
    ui->rearLeftLineEdit->setText("--");
    ui->rearRightLineEdit->setText("--");
}

void MainWindow::onFrontLeftStatusUpdated(WheelSlipStatus status)
{
    QString newStatus;
    switch (status)
    {
    case WheelSlipStatus::NotSlipping:
        newStatus = "Not slipping";
        break;

    case WheelSlipStatus::SlippingFromBraking:
        newStatus = "Slipping from braking";
        break;

    case WheelSlipStatus::SlippingFromGas:
        newStatus = "Slipping from gas";
        break;
    }

    if (ui->frontLeftLineEdit->text() != newStatus)
    {
        ui->frontLeftLineEdit->setText(newStatus);
    }
}

void MainWindow::onFrontRightStatusUpdated(WheelSlipStatus status)
{
    QString newStatus;
    switch (status)
    {
    case WheelSlipStatus::NotSlipping:
        newStatus = "Not slipping";
        break;

    case WheelSlipStatus::SlippingFromBraking:
        newStatus = "Slipping from braking";
        break;

    case WheelSlipStatus::SlippingFromGas:
        newStatus = "Slipping from gas";
        break;
    }

    if (ui->frontRightLineEdit->text() != newStatus)
    {
        ui->frontRightLineEdit->setText(newStatus);
    }
}

void MainWindow::onRearLeftStatusUpdated(WheelSlipStatus status)
{
    QString newStatus;
    switch (status)
    {
    case WheelSlipStatus::NotSlipping:
        newStatus = "Not slipping";
        break;

    case WheelSlipStatus::SlippingFromBraking:
        newStatus = "Slipping from braking";
        break;

    case WheelSlipStatus::SlippingFromGas:
        newStatus = "Slipping from gas";
        break;
    }

    if (ui->rearLeftLineEdit->text() != newStatus)
    {
        ui->rearLeftLineEdit->setText(newStatus);
    }
}

void MainWindow::onRearRightStatusUpdated(WheelSlipStatus status)
{
    QString newStatus;
    switch (status)
    {
    case WheelSlipStatus::NotSlipping:
        newStatus = "Not slipping";
        break;

    case WheelSlipStatus::SlippingFromBraking:
        newStatus = "Slipping from braking";
        break;

    case WheelSlipStatus::SlippingFromGas:
        newStatus = "Slipping from gas";
        break;
    }

    if (ui->rearRightLineEdit->text() != newStatus)
    {
        ui->rearRightLineEdit->setText(newStatus);
    }
}

void MainWindow::onSendData(const QByteArray &data)
{
    if (m_port.isEmpty())
    {
        return;
    }

    m_serialThread.transaction(m_port.portName, data);
}

void MainWindow::on_portsComboBox_currentIndexChanged(int index)
{
    if ((m_selectedSerialPortIndex != index) && (!m_initializing))
    {
        m_selectedSerialPortIndex = index;
        Port port = m_serialPorts.at(m_selectedSerialPortIndex);
        qDebug() << "Selected port: " << port.getDesignator();
        Settings::getInstance()->setPort(port.portName);
    }
}

void MainWindow::on_minimizeWindowCheckBox_clicked(bool checked)
{
    qDebug() << "Minimize with X:" << checked;
    Settings::getInstance()->setMinimizeWithX(checked);
}

void MainWindow::on_upsSpinBox_valueChanged(int ups)
{
    qint32 upsInBounds = qBound(0, ups, 120);
    Settings::getInstance()->setUps(upsInBounds);
    if (ups != upsInBounds)
    {
        ui->upsSpinBox->setValue(upsInBounds);
    }
}
