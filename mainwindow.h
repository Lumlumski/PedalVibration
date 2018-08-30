#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialthread.h"
#include <QTimer>
#include "telemetryreader.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void refreshSerialPortList();

public Q_SLOTS:
    void onFrontLeftStatusUpdated(WheelSlipStatus status);
    void onFrontRightStatusUpdated(WheelSlipStatus status);
    void onRearLeftStatusUpdated(WheelSlipStatus status);
    void onRearRightStatusUpdated(WheelSlipStatus status);

private Q_SLOTS:
    void onError(const QString &error);
    void on_sendButton_clicked();
    void on_lineEdit_textChanged(const QString &text);
    void on_portsComboBox_currentIndexChanged(int index);

private:
    void setStatus(const QString &status);
    QStringList getAvailableSerialPorts();
    void setupSerialPortList();

    Ui::MainWindow *ui;
    SerialThread m_serialThread;
    qint32 m_selectedSerialPortIndex = -1;
    QList<QString> m_serialPorts;
    TelemetryReader m_telemetryReader;

};

#endif // MAINWINDOW_H
