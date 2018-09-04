#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialthread.h"
#include <QTimer>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include "telemetryreader.h"
#include "settings.h"

namespace Ui {
class MainWindow;
}

struct Port
{
    QString portName;
    QString description;

    Port(){}

    Port(const QString &name, const QString &desc)
    {
        portName = name;
        description = desc;
    }

    QString getDesignator()
    {
        return QString(description + " (" + portName + ")");
    }

    bool isEmpty()
    {
        return portName.isEmpty();
    }

    bool operator==(const Port &other)
    {
        return (portName == other.portName);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

Q_SIGNALS:
    void quit();

public Q_SLOTS:
    void onSetStatus(const AC_STATUS &status);
    void onSetBumpingState(bool bumping);
    void onFrontLeftStatusUpdated(WheelSlipStatus status);
    void onFrontRightStatusUpdated(WheelSlipStatus status);
    void onRearLeftStatusUpdated(WheelSlipStatus status);
    void onRearRightStatusUpdated(WheelSlipStatus status);
    void onSendData(const QByteArray &data);

private Q_SLOTS:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();

    void onError(const QString &error);
    void on_wheelSlipPortComboBox_currentIndexChanged(int index);
    void on_minimizeWindowCheckBox_clicked(bool checked);
    void on_upsSpinBox_valueChanged(int ups);
    void on_enableWheelSlipCheckBox_clicked(bool checked);
    void on_enableLedFlagCheckBox_clicked(bool checked);

private:
    void setupTelemetyReader();
    void setupTrayIcon();
    void setupSerialPortList();
    void readSettings();

    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    QList<Port> getAvailableSerialPorts();
    void refreshSerialPortList();
    void clearWheelSlipIndicators();

    void showWheelSlipPage(bool show);
    void showLedFlagPage(bool show);

    void createActions();
    void createTrayIcon();
    void showAppStartedMessage();

    Ui::MainWindow *ui;
    SerialThread m_serialThread;
    TelemetryReader m_telemetryReader;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;
    QAction *m_minimizeAction;
    QAction *m_maximizeAction;
    QAction *m_restoreAction;
    QAction *m_quitAction;

    bool m_initializing;
    qint32 m_selectedSerialPortIndex = -1;
    Port m_port;
    QList<Port> m_serialPorts;

};

#endif // MAINWINDOW_H
