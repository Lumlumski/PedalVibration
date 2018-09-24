#ifndef TELEMETRYREADER_122A5A0D4A0B4698AA1164390F74EBFE
#define TELEMETRYREADER_122A5A0D4A0B4698AA1164390F74EBFE

#include <QObject>
#include <QTimer>
#include "assettocorsadata.h"

struct WheelValueInt
{
    qint32 frontLeft;
    qint32 frontRight;
    qint32 rearLeft;
    qint32 rearRight;

    WheelValueInt()
    {
        frontLeft = 0;
        frontRight = 0;
        rearLeft = 0;
        rearRight = 0;
    }
};

struct WheelValueFloat
{
    float frontLeft;
    float frontRight;
    float rearLeft;
    float rearRight;

    WheelValueFloat()
    {
        frontLeft = 0.0f;
        frontRight = 0.0f;
        rearLeft = 0.0f;
        rearRight = 0.0f;
    }
};

enum WheelSlipStatus
{
    NotSlipping,
    SlippingFromBraking,
    SlippingFromGas
};

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
    void error(const QString &error);
    void sendWheelSlipData(const QByteArray &data);
    void sendLedFlagData(const QByteArray &data);
    void sendWindFanData(const QByteArray &data);

private Q_SLOTS:
    void readData();

private:
    WheelValueInt getWheelSlip();
    WheelValueFloat getCalculatedSpeed();
    float calculateSpeed(float tyreRadius, float wheelAngularSpeed);
    WheelSlipStatus getSlipStatus(float slipValue, float calculatedSpeed);
    bool dataChanged();
    void sendInitialValues();
    void sendWheelSlip();
    void sendLedFlag();
    void sendWindFan();
    void readSettings();

    QTimer m_readTimer;
    qint32 m_standbyInterval;
    qint32 m_liveInterval;
    AssettoCorsaData m_acData;
    AC_STATUS m_lastStatus;

    bool m_readStaticData;
    WheelValueFloat m_tyreRadius;
    float m_brakeIndex = 0.0f;
    float m_gasIndex = 0.0f;
    qint32 m_bumpingIndex = 0;
    qint32 m_speed;
    qint32 m_lastSpeed;
    bool m_lastBumping;
    //qint32 m_flagStatus;

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
