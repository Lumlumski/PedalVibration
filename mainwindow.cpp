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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    (void)connect(&m_telemetryReader, &TelemetryReader::error, this, &MainWindow::onError);
    (void)connect(&m_telemetryReader, &TelemetryReader::setStatus, this, &MainWindow::onSetStatus);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontLeftStatusUpdated, this, &MainWindow::onFrontLeftStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontRightStatusUpdated, this, &MainWindow::onFrontRightStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::rearLeftStatusUpdated, this, &MainWindow::onRearLeftStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::rearRightStatusUpdated, this, &MainWindow::onRearRightStatusUpdated);

    createActions();
    createTrayIcon();

    m_trayIcon->setIcon(QIcon("icon.png"));
    m_trayIcon->show();
    (void)connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
    (void)connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::showWindow);

    setupSerialPortList();

    m_telemetryReader.setUpdatesPerSecond(5);
    m_telemetryReader.run();

    showAppStartedMessage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::hideEvent(QHideEvent * event)
{
    m_minimizeAction->trigger();
    event->ignore();
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

QStringList MainWindow::getAvailableSerialPorts()
{
    QStringList serialPorts;
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    for (qint32 i = 0; i < portList.size(); ++i)
    {
        serialPorts.append(portList.at(i).description() + " (" + portList.at(i).portName() + ")");
    }

    return serialPorts;
}

void MainWindow::setupSerialPortList()
{
    QStringList serialPortList = getAvailableSerialPorts();
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
    ui->portsComboBox->addItems(m_serialPorts);

    m_selectedSerialPortIndex = 0;
    ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
}

void MainWindow::refreshSerialPortList()
{
    qDebug() << "Refreshing serial port list";
    QStringList newSerialPortList = getAvailableSerialPorts();
    qDebug() << newSerialPortList;

    m_port.clear();

    if (newSerialPortList.isEmpty())
    {
        m_selectedSerialPortIndex = -1;
        m_serialPorts.clear();
        ui->portsComboBox->clear();
        ui->portsComboBox->setEnabled(false);
        return;
    }

    ui->portsComboBox->setEnabled(true);

    if (m_selectedSerialPortIndex != -1)
    {
        m_port = m_serialPorts.at(m_selectedSerialPortIndex);
    }

    ui->portsComboBox->clear();

    if (!m_port.isEmpty())
    {
        qint32 newIndex = newSerialPortList.indexOf(m_port);
        if (newIndex == -1)
        {
            m_selectedSerialPortIndex = -1;
            ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
            m_port.clear();
            return;
        }

        m_selectedSerialPortIndex = newIndex;
    }

    m_serialPorts = newSerialPortList;

    ui->portsComboBox->addItems(m_serialPorts);
    ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
    m_port = m_serialPorts.at(m_selectedSerialPortIndex);

    // If no port was selected before, select the first one by default
    if (m_selectedSerialPortIndex == -1)
    {
        m_selectedSerialPortIndex = 0;
        ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
        m_port = m_serialPorts.at(m_selectedSerialPortIndex);
    }
}

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

    m_serialThread.transaction(m_port, data);
}

void MainWindow::on_portsComboBox_currentIndexChanged(int index)
{
    if (m_selectedSerialPortIndex != index)
    {
        m_selectedSerialPortIndex = index;
    }
}

void MainWindow::on_minimizeWindowCheckBox_clicked(bool checked)
{
    Q_UNUSED(checked)
}
