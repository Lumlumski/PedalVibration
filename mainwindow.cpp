#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    (void)connect(&m_serialThread, &SerialThread::error, this, &MainWindow::onError);
    (void)connect(&m_telemetryReader, &TelemetryReader::frontLeftStatusUpdated, this, &MainWindow::onFrontLeftStatusUpdated);

    ui->sendButton->setEnabled(false);
    setupSerialPortList();

    m_telemetryReader.setUpdatesPerSecond(5);
    m_telemetryReader.run();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setStatus(const QString &status)
{
    qDebug() << status;
    ui->statusLabel->setText(status);
}

void MainWindow::onError(const QString &error)
{
    setStatus("An error occured: " + error);
}

QStringList MainWindow::getAvailableSerialPorts()
{
    QStringList serialPorts;
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    for (qint32 i = 0; i < portList.size(); ++i)
    {
        serialPorts.append(portList.at(i).portName());
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

    if (newSerialPortList.isEmpty())
    {
        m_selectedSerialPortIndex = -1;
        m_serialPorts.clear();
        ui->portsComboBox->clear();
        ui->portsComboBox->setEnabled(false);
        return;
    }

    ui->portsComboBox->setEnabled(true);

    QString selectedPort;
    if (m_selectedSerialPortIndex != -1)
    {
        selectedPort = m_serialPorts.at(m_selectedSerialPortIndex);
        qDebug() << "selectedPort: " << selectedPort;
    }

    ui->portsComboBox->clear();

    if (!selectedPort.isEmpty())
    {
        qint32 newIndex = newSerialPortList.indexOf(selectedPort);
        qDebug() << "newIndex: " << newIndex;

        if (newIndex == -1)
        {
            m_selectedSerialPortIndex = -1;
            ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
            return;
        }

        qDebug() << "m_selectedSerialPortIndex = newIndex (" << newIndex << ")";
        m_selectedSerialPortIndex = newIndex;
    }

    m_serialPorts = newSerialPortList;

    ui->portsComboBox->addItems(m_serialPorts);
    qDebug() << "selected: " << m_selectedSerialPortIndex;
    ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);

    // If no port was selected before, select the first one by default
    if (m_selectedSerialPortIndex == -1)
    {
        m_selectedSerialPortIndex = 0;
        ui->portsComboBox->setCurrentIndex(m_selectedSerialPortIndex);
    }
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

void MainWindow::on_sendButton_clicked()
{
    QString text = ui->lineEdit->text();
    if (text.isEmpty() || (m_selectedSerialPortIndex < 0) || (m_selectedSerialPortIndex >= m_serialPorts.size()))
    {
        return;
    }

    QString port = m_serialPorts.at(m_selectedSerialPortIndex);
    if (port.isEmpty())
    {
        return;
    }

    m_serialThread.transaction(port, text.toUtf8());
}

void MainWindow::on_lineEdit_textChanged(const QString &text)
{
    bool textExists = !text.isEmpty();
    if (ui->sendButton->isEnabled() != textExists)
    {
        ui->sendButton->setEnabled(textExists);
    }
}

void MainWindow::on_portsComboBox_currentIndexChanged(int index)
{
    if (m_selectedSerialPortIndex != index)
    {
        qDebug() << "Selected index:" << index;
        m_selectedSerialPortIndex = index;
    }
}
