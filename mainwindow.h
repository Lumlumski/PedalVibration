#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialthread.h"
#include <QTimer>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include "telemetryreader.h"

namespace Ui {
class MainWindow;
}

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
    void onFrontLeftStatusUpdated(WheelSlipStatus status);
    void onFrontRightStatusUpdated(WheelSlipStatus status);
    void onRearLeftStatusUpdated(WheelSlipStatus status);
    void onRearRightStatusUpdated(WheelSlipStatus status);
    void onSendData(const QByteArray &data);

private Q_SLOTS:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();

    void onError(const QString &error);
    void on_portsComboBox_currentIndexChanged(int index);
    void on_minimizeWindowCheckBox_clicked(bool checked);

private:
    void hideEvent(QHideEvent *event) override;

    QStringList getAvailableSerialPorts();
    void setupSerialPortList();
    void refreshSerialPortList();
    void clearWheelSlipIndicators();

    void createActions();
    void createTrayIcon();
    void showAppStartedMessage();

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;

    QAction *m_minimizeAction;
    QAction *m_maximizeAction;
    QAction *m_restoreAction;
    QAction *m_quitAction;

    Ui::MainWindow *ui;
    SerialThread m_serialThread;
    qint32 m_selectedSerialPortIndex = -1;
    QString m_port;
    QList<QString> m_serialPorts;
    TelemetryReader m_telemetryReader;

};

#endif // MAINWINDOW_H
