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
    , m_wheelSlipConfig(new WheelSlipConfiguration(this))
    , m_windFanConfig(new WindFanConfiguration(this))
    , m_initializing(true)
{
    ui->setupUi(this);
    this->setFixedSize(450, 450);
    ui->bumpingLabel->setVisible(false);
    ui->tabWidget->setCurrentIndex(0);

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

    // UI
    (void)connect(&m_telemetryReader, &TelemetryReader::setStatus, this, &MainWindow::onSetStatus);
    (void)connect(&m_telemetryReader, &TelemetryReader::setBumpingState, this, &MainWindow::onSetBumpingState);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontLeftStatusUpdated, this, &MainWindow::onFrontLeftStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontRightStatusUpdated, this, &MainWindow::onFrontRightStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::rearLeftStatusUpdated, this, &MainWindow::onRearLeftStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::rearRightStatusUpdated, this, &MainWindow::onRearRightStatusUpdated);
    (void)connect(&m_telemetryReader, &TelemetryReader::speedUpdated, this, &MainWindow::onSpeedUpdated);

    // Serial
    (void)connect(&m_telemetryReader, &TelemetryReader::sendWheelSlipData, this, &MainWindow::onSendWheelSlipData);
    (void)connect(&m_telemetryReader, &TelemetryReader::sendLedFlagData, this, &MainWindow::onSendLedFlagData);
    (void)connect(&m_telemetryReader, &TelemetryReader::sendWindFanData, this, &MainWindow::onSendWindFanData);
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

    // Wheel slip detection enabled
    if (settings->getWheelSlipEnabled())
    {
        ui->enableWheelSlipCheckBox->setCheckState(Qt::CheckState::Checked);
        showWheelSlipPage(true);
    }
    else
    {
        ui->enableWheelSlipCheckBox->setCheckState(Qt::CheckState::Unchecked);
        showWheelSlipPage(false);
    }

    // LED Flag enabled
    if (settings->getLedFlagEnabled())
    {
        ui->enableLedFlagCheckBox->setCheckState(Qt::CheckState::Checked);
        showLedFlagPage(true);
    }
    else
    {
        ui->enableLedFlagCheckBox->setCheckState(Qt::CheckState::Unchecked);
        showLedFlagPage(false);
    }

    // Wind Fan enabled
    if (settings->getWindFanEnabled())
    {
        ui->enableWindFanCheckBox->setCheckState(Qt::CheckState::Checked);
        showWindFanPage(true);
    }
    else
    {
        ui->enableWindFanCheckBox->setCheckState(Qt::CheckState::Unchecked);
        showWindFanPage(false);
    }

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
    m_trayIcon->showMessage("Pedal vibration", "Pedal vibration started in tray", QSystemTrayIcon::Information, 100);
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
    qint32 wheelSlipPortSelectedIndex = -1;
    qint32 ledFlagPortSelectedIndex = -1;
    qint32 windFanPortSelectedIndex = -1;

    QList<Port> serialPortList = getAvailableSerialPorts();
    if (serialPortList.isEmpty())
    {
        m_serialPorts.clear();
        ui->wheelSlipPortComboBox->clear();
        ui->wheelSlipPortComboBox->setEnabled(false);
        ui->ledFlagPortComboBox->clear();
        ui->ledFlagPortComboBox->setEnabled(false);
        ui->windFanPortComboBox->clear();
        ui->windFanPortComboBox->setEnabled(false);
        return;
    }

    m_serialPorts = serialPortList;

    QString wheelSlipPort = Settings::getInstance()->getWheelSlipPort();
    QString ledFlagPort = Settings::getInstance()->getLedFlagPort();
    QString windFanPort = Settings::getInstance()->getWindFanPort();

    for (qint32 i = 0; i < m_serialPorts.size(); ++i)
    {
        QString portEntry = m_serialPorts[i].getDesignator();
        ui->wheelSlipPortComboBox->addItem(portEntry);
        ui->ledFlagPortComboBox->addItem(portEntry);
        ui->windFanPortComboBox->addItem(portEntry);

        if (m_serialPorts[i].portName == wheelSlipPort)
        {
            wheelSlipPortSelectedIndex = i;
            m_wheelSlipPort = m_serialPorts[i];
            ui->wheelSlipPortComboBox->setCurrentIndex(i);
        }

        if (m_serialPorts[i].portName == ledFlagPort)
        {
            ledFlagPortSelectedIndex = i;
            m_ledFlagPort = m_serialPorts[i];
            ui->ledFlagPortComboBox->setCurrentIndex(i);
        }

        if (m_serialPorts[i].portName == windFanPort)
        {
            windFanPortSelectedIndex = i;
            m_windFanPort = m_serialPorts[i];
            ui->windFanPortComboBox->setCurrentIndex(i);
        }
    }

    if (wheelSlipPortSelectedIndex == -1)
    {
        ui->wheelSlipPortComboBox->setCurrentIndex(0);
    }

    if (ledFlagPortSelectedIndex == -1)
    {
        ui->ledFlagPortComboBox->setCurrentIndex(0);
    }

    if (windFanPortSelectedIndex == -1)
    {
        ui->windFanPortComboBox->setCurrentIndex(0);
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
        clearIndicators();
        break;
    case AC_REPLAY:
        qDebug() << "Status changed: REPLAY";
        statusText = "Replay running";
        clearIndicators();
        break;
    case AC_LIVE:
        qDebug() << "Status changed: LIVE";
        statusText = "Live! Sending telemetry data";
        break;
    case AC_PAUSE:
        qDebug() << "Status changed: PAUSE";
        statusText = "Game is paused";
        clearIndicators();
        break;
    }

    ui->statusLabel->setText(statusText);
}

void MainWindow::onSetBumpingState(bool bumping)
{
    ui->bumpingLabel->setVisible(bumping);
}

void MainWindow::clearIndicators()
{
    ui->frontLeftLineEdit->setText("--");
    ui->frontRightLineEdit->setText("--");
    ui->rearLeftLineEdit->setText("--");
    ui->rearRightLineEdit->setText("--");

    ui->speedLineEdit->setText("--");
}

void MainWindow::onFrontLeftStatusUpdated(WheelSlipStatus status)
{
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        return;
    }

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
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        return;
    }

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
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        return;
    }

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
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        return;
    }

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

void MainWindow::onSpeedUpdated(qint32 speed)
{
    ui->speedLineEdit->setText(QString::number(speed));
}

void MainWindow::onSendWheelSlipData(const QByteArray &data)
{
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        qDebug() << "sendWheelSlipData not enabled";
        return;
    }

    QString port = Settings::getInstance()->getWheelSlipPort();
    if (port.isEmpty())
    {
        qDebug() << "Port not found";
        return;
    }

    m_wheelSlipSerialThread.transaction(port, data);
}

void MainWindow::onSendLedFlagData(const QByteArray &data)
{
    if (!Settings::getInstance()->getLedFlagEnabled())
    {
        return;
    }

    QString port = Settings::getInstance()->getLedFlagPort();
    if (port.isEmpty())
    {
        return;
    }

    m_ledFlagSerialThread.transaction(port, data);
}

void MainWindow::onSendWindFanData(const QByteArray &data)
{
    if (!Settings::getInstance()->getWindFanEnabled())
    {
        return;
    }

    QString port = Settings::getInstance()->getWindFanPort();
    if (port.isEmpty())
    {
        return;
    }

    m_windFanSerialThread.transaction(port, data);
}

void MainWindow::on_wheelSlipPortComboBox_currentIndexChanged(int index)
{
    if (!m_initializing)
    {
        m_wheelSlipPort = m_serialPorts.at(index);
        qDebug() << "Selected port: " << m_wheelSlipPort.getDesignator();
        Settings::getInstance()->setWheelSlipPort(m_wheelSlipPort.portName);
    }
}

void MainWindow::on_ledFlagPortComboBox_currentIndexChanged(int index)
{
    if (!m_initializing)
    {
        m_ledFlagPort = m_serialPorts.at(index);
        qDebug() << "Selected port: " << m_ledFlagPort.getDesignator();
        Settings::getInstance()->setLedFlagPort(m_ledFlagPort.portName);
    }
}

void MainWindow::on_windFanPortComboBox_currentIndexChanged(int index)
{
    if (!m_initializing)
    {
        m_windFanPort = m_serialPorts.at(index);
        qDebug() << "Selected port: " << m_windFanPort.getDesignator();
        Settings::getInstance()->setWindFanPort(m_windFanPort.portName);
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

void MainWindow::on_enableWheelSlipCheckBox_clicked(bool checked)
{
    if (!m_initializing)
    {
        Settings::getInstance()->setWheelSlipEnabled(checked);
    }

    showWheelSlipPage(checked);
}

void MainWindow::on_enableLedFlagCheckBox_clicked(bool checked)
{
    if (!m_initializing)
    {
        Settings::getInstance()->setLedFlagEnabled(checked);
    }

    showLedFlagPage(checked);
}

void MainWindow::on_enableWindFanCheckBox_clicked(bool checked)
{
    if (!m_initializing)
    {
        Settings::getInstance()->setWindFanEnabled(checked);
    }

    showWindFanPage(checked);
    if (!checked)
    {
        sendStopFan();
    }
}

void MainWindow::sendStopFan()
{
    qint32 bytesWritten = 0;
    QByteArray data;

    data.append(0x02);
    ++bytesWritten;

    for (qint32 i = bytesWritten; i < 8; ++i)
    {
        data.append(QChar(0));
    }

    QString port = Settings::getInstance()->getWindFanPort();
    if (port.isEmpty())
    {
        return;
    }

    m_windFanSerialThread.transaction(port, data);
}

void MainWindow::showWheelSlipPage(bool show)
{
    ui->wheelSlipPortComboBox->setEnabled(show);
    ui->frontLeftLineEdit->setEnabled(show);
    ui->frontRightLineEdit->setEnabled(show);
    ui->rearLeftLineEdit->setEnabled(show);
    ui->rearRightLineEdit->setEnabled(show);

    if (!show)
    {
        ui->frontLeftLineEdit ->setText("--");
        ui->frontRightLineEdit->setText("--");
        ui->rearLeftLineEdit  ->setText("--");
        ui->rearRightLineEdit ->setText("--");
    }
}

void MainWindow::showLedFlagPage(bool show)
{
    ui->ledFlagPortComboBox->setEnabled(show);
}

void MainWindow::showWindFanPage(bool show)
{
    ui->windFanPortComboBox->setEnabled(show);
    ui->speedLineEdit->setEnabled(show);
    if (!show)
    {
        ui->speedLineEdit->setText("--");
    }
}

void MainWindow::on_configureWheelSlipButton_clicked()
{
    this->setEnabled(false);
    m_wheelSlipConfig->setEnabled(true);
    m_wheelSlipConfig->show();
}

void MainWindow::on_configureWindFanButton_clicked()
{
    this->setEnabled(false);
    m_windFanConfig->setEnabled(true);
    m_windFanConfig->show();
}
