#include "telemetryreader.h"
#include <QDebug>
#include <QtMath>
#include <QString>
#include <QChar>
#include <QProcess>

TelemetryReader::TelemetryReader(QObject *parent)
    : QObject(parent)
    , m_standbyInterval(1000)
    , m_liveInterval(0)
    , m_lastStatus(AC_OFF)
    , m_readStaticData(false)
    , m_speed(0.0f)
{
    (void)connect(&m_readTimer, &QTimer::timeout, this, &TelemetryReader::readData);
    m_readTimer.setInterval(m_standbyInterval);
}

TelemetryReader::~TelemetryReader()
{
    m_readTimer.stop();
}

void TelemetryReader::run()
{
    if (m_liveInterval == 0)
    {
        qDebug() << "Cannot start read timer. No update rate set";
        return;
    }

    m_readTimer.start();
    qDebug() << "Started read timer";
}

void TelemetryReader::stop()
{
    m_readTimer.stop();
    qDebug() << "Stopped read timer";
}

void TelemetryReader::setUpdatesPerSecond(qint32 ups)
{
    double ms = 1000.0 / static_cast<double>(ups);
    m_liveInterval = qRound(ms + 0.5); // Round up
    qDebug() << "Setting" << ups << "UPS =" << m_liveInterval << "ms";
}

void TelemetryReader::readData()
{
    m_acData.update();

    AC_STATUS status = m_acData.getStatus();
    if (status != m_lastStatus)
    {
        Q_EMIT setStatus(status);

        if (m_lastStatus == AC_LIVE)
        {
            m_readTimer.setInterval(m_standbyInterval);
            m_lastStatus = status;
            return;
        }
        else if (status == AC_LIVE)
        {
            if (m_liveInterval > 0)
            {
                m_readTimer.setInterval(m_liveInterval);
            }

            Q_EMIT frontLeftStatusUpdated(WheelSlipStatus::NotSlipping);
            Q_EMIT frontRightStatusUpdated(WheelSlipStatus::NotSlipping);
            Q_EMIT rearLeftStatusUpdated(WheelSlipStatus::NotSlipping);
            Q_EMIT rearRightStatusUpdated(WheelSlipStatus::NotSlipping);
        }

        m_lastStatus = status;
    }
    else if (status != AC_LIVE)
    {
        // Status did not change and is not AC_LIVE
        // Do nothing
        return;
    }

    //  Check if tyre radius is not set yet
    if (!m_readStaticData)
    {
        m_tyreRadius.frontLeft = m_acData.getTyreRadius((Wheel::FrontLeft));
        m_tyreRadius.frontRight = m_acData.getTyreRadius(Wheel::FrontRight);
        m_tyreRadius.rearLeft = m_acData.getTyreRadius(Wheel::RearLeft);
        m_tyreRadius.rearRight = m_acData.getTyreRadius(Wheel::RearRight);
        m_readStaticData = true;
    }

    m_speed = m_acData.getSpeedKmh();
    WheelValueInt slip = getWheelSlip();
    WheelValueFloat calculatedSpeed = getCalculatedSpeed();

    qDebug() << "Front left wheel:";
    WheelSlipStatus frontLeftSlipStatus = getSlipStatus(slip.frontLeft, calculatedSpeed.frontLeft);

    qDebug() << "Front right wheel:";
    WheelSlipStatus frontRightSlipStatus = getSlipStatus(slip.frontRight, calculatedSpeed.frontRight);

    qDebug() << "Rear left wheel:";
    WheelSlipStatus rearLeftSlipStatus = getSlipStatus(slip.rearLeft, calculatedSpeed.rearLeft);

    qDebug() << "Rear right wheel:";
    WheelSlipStatus rearRightSlipStatus = getSlipStatus(slip.rearRight, calculatedSpeed.rearRight);

    // Check for bumping effect
    bool bumping = false;
    bumping |= (m_acData.getWheelLoad(Wheel::FrontLeft) == 0.0f);
    bumping |= (m_acData.getWheelLoad(Wheel::FrontRight) == 0.0f);
    bumping |= (m_acData.getWheelLoad(Wheel::RearLeft) == 0.0f);
    bumping |= (m_acData.getWheelLoad(Wheel::RearRight) == 0.0f);

    switch (frontLeftSlipStatus)
    {
    case SlippingFromBraking:
        m_serialData[0] = slip.frontLeft;
        m_serialData[1] = 0;
        break;

    case SlippingFromGas:
        m_serialData[0] = 0;
        m_serialData[1] = slip.frontLeft;
        break;

    case NotSlipping:
        m_serialData[0] = 0;
        m_serialData[1] = 0;
        break;
    }

    if (m_oldFrontLeftSlipStatus != frontLeftSlipStatus)
    {
        Q_EMIT frontLeftStatusUpdated(frontLeftSlipStatus);
        m_oldFrontLeftSlipStatus = frontLeftSlipStatus;
    }

    switch (frontRightSlipStatus)
    {
    case SlippingFromBraking:
        m_serialData[2] = slip.frontRight;
        m_serialData[3] = 0;
        break;

    case SlippingFromGas:
        m_serialData[2] = 0;
        m_serialData[3] = slip.frontRight;
        break;

    case NotSlipping:
        m_serialData[2] = 0;
        m_serialData[3] = 0;
        break;
    }

    if (m_oldFrontRightSlipStatus != frontRightSlipStatus)
    {
        Q_EMIT frontRightStatusUpdated(frontRightSlipStatus);
        m_oldFrontRightSlipStatus = frontRightSlipStatus;
    }

    switch (rearLeftSlipStatus)
    {
    case SlippingFromBraking:
        m_serialData[4] = slip.rearLeft;
        m_serialData[5] = 0;
        break;

    case SlippingFromGas:
        m_serialData[4] = 0;
        m_serialData[5] = slip.rearLeft;
        break;

    case NotSlipping:
        m_serialData[4] = 0;
        m_serialData[5] = 0;
        break;
    }

    if (m_oldRearLeftSlipStatus != rearLeftSlipStatus)
    {
        Q_EMIT rearLeftStatusUpdated(rearLeftSlipStatus);
        m_oldRearLeftSlipStatus = rearLeftSlipStatus;
    }

    switch (rearRightSlipStatus)
    {
    case SlippingFromBraking:
        m_serialData[6] = slip.rearRight;
        m_serialData[7] = 0;
        break;

    case SlippingFromGas:
        m_serialData[6] = 0;
        m_serialData[7] = slip.rearRight;
        break;

    case NotSlipping:
        m_serialData[6] = 0;
        m_serialData[7] = 0;
        break;
    }

    if (m_oldRearRightSlipStatus != rearRightSlipStatus)
    {
        Q_EMIT rearRightStatusUpdated(rearRightSlipStatus);
        m_oldRearRightSlipStatus = rearRightSlipStatus;
    }

    // Let everything vibrate a bit if bumping was detected
    if (bumping)
    {
        for (qint32 i = 0; i < 8; ++i)
        {
            if (m_serialData[i] < 3) m_serialData[i] = 3;
        }
    }

    // Only send if something has changed
    if (dataChanged())
    {
        sendData();
    }

    // Save current values for comparison with future values
    for (qint32 i = 0; i < 8; ++i)
    {
        m_lastSerialData[i] = m_serialData[i];
    }
}

bool TelemetryReader::dataChanged()
{
    for (qint32 i = 0; i < 8; ++i)
    {
        if (m_serialData[i] != m_lastSerialData[i])
        {
            return true;
        }
    }

    return false;
}

WheelValueInt TelemetryReader::getWheelSlip()
{
    WheelValueInt slip;
    slip.frontLeft = static_cast<qint32>(m_acData.getWheelSlip(Wheel::FrontLeft));
    slip.frontRight = static_cast<qint32>(m_acData.getWheelSlip(Wheel::FrontRight));
    slip.rearLeft = static_cast<qint32>(m_acData.getWheelSlip(Wheel::RearLeft));
    slip.rearRight = static_cast<qint32>(m_acData.getWheelSlip(Wheel::RearRight));

    // Be sure to stay between 0 and 255
    slip.frontLeft = qBound(0, slip.frontLeft, 255);
    slip.frontRight = qBound(0, slip.frontRight, 255);
    slip.rearLeft = qBound(0, slip.rearLeft, 255);
    slip.rearRight = qBound(0, slip.rearRight, 255);

    return slip;
}

float TelemetryReader::calculateSpeed(float tyreRadius, float wheelAngularSpeed)
{
    return ((2 * tyreRadius * static_cast<float>(M_PI) * wheelAngularSpeed * 60) / 100);
}

WheelValueFloat TelemetryReader::getCalculatedSpeed()
{
    WheelValueFloat calculatedSpeed;
    calculatedSpeed.frontLeft = calculateSpeed(m_tyreRadius.frontLeft, m_acData.getWheelAngularSpeed(Wheel::FrontLeft));
    calculatedSpeed.frontRight = calculateSpeed(m_tyreRadius.frontRight, m_acData.getWheelAngularSpeed(Wheel::FrontRight));
    calculatedSpeed.rearLeft = calculateSpeed(m_tyreRadius.rearLeft, m_acData.getWheelAngularSpeed(Wheel::RearLeft));
    calculatedSpeed.rearRight = calculateSpeed(m_tyreRadius.rearRight, m_acData.getWheelAngularSpeed(Wheel::RearRight));
    return calculatedSpeed;
}

WheelSlipStatus TelemetryReader::getSlipStatus(float slipValue, float calculatedSpeed)
{
    if (slipValue == 0.0f)
    {
        qDebug() << "Not slipping";
        return NotSlipping;
    }
    else if (calculatedSpeed < (m_speed * BRAKE_INDEX))
    {
        qDebug() << "Slipping from braking";
        return SlippingFromBraking;
    }
    else if (calculatedSpeed > (m_speed * GAS_INDEX))
    {
        qDebug() << "Slipping from gas";
        return SlippingFromGas;
    }

    qDebug() << "Not slipping";
    return NotSlipping;
}

void TelemetryReader::sendData()
{
    QByteArray data;
    for (qint32 i = 0; i < 8; ++i)
    {
        data.append(QChar(m_serialData[i]));
    }

    qDebug() << "Sending buffer: " << data;
    Q_EMIT sendData(data);
}
