#ifndef TELEMETRYREADER_H
#define TELEMETRYREADER_H

#include <QObject>
#include <QTimer>
#include "AssettoCorsaData.h"

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
    void frontLeftStatusUpdated(WheelSlipStatus status);
    void frontRightStatusUpdated(WheelSlipStatus status);
    void rearLeftStatusUpdated(WheelSlipStatus status);
    void rearRightStatusUpdated(WheelSlipStatus status);

private Q_SLOTS:
    void readData();

private:
    WheelValueInt getWheelSlip();
    WheelValueFloat getCalculatedSpeed();
    float calculateSpeed(float tyreRadius, float wheelAngularSpeed);
    WheelSlipStatus getSlipStatus(float slipValue, float calculatedSpeed);

    QTimer m_readTimer;
    qint32 m_standbyInterval;
    qint32 m_liveInterval;
    AssettoCorsaData m_acData;
    AC_STATUS m_lastStatus;

    bool m_readStaticData;
    WheelValueFloat m_tyreRadius;
    const float BRAKE_INDEX = 0.98f;
    const float GAS_INDEX = 1.08f;
    float m_speed;

    qint32 m_lastSerialData[8];
    qint32 m_serialData[8];

};

#endif // TELEMETRYREADER_H