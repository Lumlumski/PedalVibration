#ifndef MAINWINDOW_98F9DED978F74F55B34B0B38F4F3CD95
#define MAINWINDOW_98F9DED978F74F55B34B0B38F4F3CD95

#include <QMainWindow>
#include "serialthread.h"
#include <QTimer>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include "telemetryreader.h"
#include "settings.h"
#include "wheelslipconfiguration.h"

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
    void onSpeedUpdated(qint32 speed);
    void onSendWheelSlipData(const QByteArray &data);
    void onSendLedFlagData(const QByteArray &data);
    void onSendWindFanData(const QByteArray &data);

private Q_SLOTS:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();

    void onError(const QString &error);
    void on_wheelSlipPortComboBox_currentIndexChanged(int index);
    void on_ledFlagPortComboBox_currentIndexChanged(int index);
    void on_windFanPortComboBox_currentIndexChanged(int index);
    void on_minimizeWindowCheckBox_clicked(bool checked);
    void on_upsSpinBox_valueChanged(int ups);
    void on_enableWheelSlipCheckBox_clicked(bool checked);
    void on_enableLedFlagCheckBox_clicked(bool checked);
    void on_enableWindFanCheckBox_clicked(bool checked);

    void on_pushButton_clicked();

private:
    void setupTelemetyReader();
    void setupTrayIcon();
    void setupSerialPortList();
    void readSettings();

    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    QList<Port> getAvailableSerialPorts();
    void refreshSerialPortList();
    void clearIndicators();

    void showWheelSlipPage(bool show);
    void showLedFlagPage(bool show);
    void showWindFanPage(bool show);
    void sendStopFan();

    void createActions();
    void createTrayIcon();
    void showAppStartedMessage();

    Ui::MainWindow *ui;
    SerialThread m_wheelSlipSerialThread;
    SerialThread m_ledFlagSerialThread;
    SerialThread m_windFanSerialThread;
    TelemetryReader m_telemetryReader;
    WheelSlipConfiguration* const m_wheelSlipConfig;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;
    QAction *m_minimizeAction;
    QAction *m_maximizeAction;
    QAction *m_restoreAction;
    QAction *m_quitAction;

    bool m_initializing;
    qint32 m_selectedSerialPortIndex = -1;
    Port m_wheelSlipPort;
    Port m_ledFlagPort;
    Port m_windFanPort;
    QList<Port> m_serialPorts;

};

#endif // MAINWINDOW_98F9DED978F74F55B34B0B38F4F3CD95
