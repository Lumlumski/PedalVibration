#ifndef TELEMETRYREADER_122A5A0D4A0B4698AA1164390F74EBFE
#define TELEMETRYREADER_122A5A0D4A0B4698AA1164390F74EBFE

#include <QObject>
#include <QTimer>
#include "assettocorsadata.h"
#include "globals.h"


class TelemetryReader : public QObject
{
    Q_OBJECT
public:
    explicit TelemetryReader(QObject *parent = nullptr);
    ~TelemetryReader();

    void run();
    void stop();

    void setUpdatesPerSecond(qint32 ups);

Q_SIGNALS:
    void setStatus(const AC_STATUS &status);
    void setBumpingState(bool bumping);
    void frontLeftStatusUpdated(WheelSlipStatus status);
    void frontRightStatusUpdated(WheelSlipStatus status);
    void rearLeftStatusUpdated(WheelSlipStatus status);
    void rearRightStatusUpdated(WheelSlipStatus status);
    void speedUpdated(qint32 speed);
    void flagStatusUpdated(AC_FLAG_TYPE flagStatus);
    void error(const QString &error);

    void sendInitialValues();
    void sendWheelSlipValues(quint8 gasValue, quint8 brakeValue);
    void sendWindFanValue(quint8 windFanValue);
    void sendLedFlagValue(quint8 ledFlagValue);

public Q_SLOTS:
    void onGasIndexChanged();
    void onBrakeIndexChanged();
    void onBumpingIndexChanged();
    void onWindFanIndexChanged();

private Q_SLOTS:
    void readData();

private:
    WheelValueInt getWheelSlip();
    WheelValueFloat getCalculatedSpeed();
    float calculateSpeed(float tyreRadius, float wheelAngularSpeed);
    WheelSlipStatus getSlipStatus(float slipValue, float calculatedSpeed);
    bool dataChanged();
    void readSettings();

    void calculateWheelSlip();
    void calculateLedFlagStatus();
    void calculateWindFanSpeed();

    QTimer m_readTimer;
    qint32 m_standbyInterval = 0;
    qint32 m_liveInterval = 0;
    AssettoCorsaData m_acData;
    AC_STATUS m_lastStatus;

    bool m_readStaticData = false;
    WheelValueFloat m_tyreRadius;
    float m_brakeIndex = 0.0f;
    float m_gasIndex = 0.0f;
    qint32 m_bumpingIndex = 0;
    qint32 m_windFanIndex = 0;
    qint32 m_speed = 0;
    qint32 m_lastSpeed = 0;
    quint8 m_lastWindFanValue = 0;
    bool m_lastBumping = false;
    AC_FLAG_TYPE m_lastFlagStatus = AC_NO_FLAG;

    WheelSlipStatus m_oldFrontLeftSlipStatus = NotSlipping;
    WheelSlipStatus m_oldFrontRightSlipStatus = NotSlipping;
    WheelSlipStatus m_oldRearLeftSlipStatus = NotSlipping;
    WheelSlipStatus m_oldRearRightSlipStatus = NotSlipping;

    qint32 m_maxBrakeValue = 0;
    qint32 m_lastMaxBrakeValue = 0;
    qint32 m_maxGasValue = 0;
    qint32 m_lastMaxGasValue = 0;

};

#endif // TELEMETRYREADER_122A5A0D4A0B4698AA1164390F74EBFE
