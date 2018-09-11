#include "telemetryreader.h"
#include <QDebug>
#include <QtMath>
#include <QString>
#include <QChar>
#include <QProcess>
#include <QBuffer>
#include <QDataStream>
#include <QtEndian>

TelemetryReader::TelemetryReader(QObject *parent)
    : QObject(parent)
    , m_standbyInterval(1000)
    , m_liveInterval(0)
    , m_lastStatus(AC_OFF)
    , m_readStaticData(false)
    , m_speed(0)
    , m_lastSpeed(0)
    , m_lastBumping(false)
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
            sendInitialValues();
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
        // Reset serial data to 0
        sendInitialValues();
        m_tyreRadius.frontLeft = m_acData.getTyreRadius((Wheel::FrontLeft));
        m_tyreRadius.frontRight = m_acData.getTyreRadius(Wheel::FrontRight);
        m_tyreRadius.rearLeft = m_acData.getTyreRadius(Wheel::RearLeft);
        m_tyreRadius.rearRight = m_acData.getTyreRadius(Wheel::RearRight);
        m_readStaticData = true;
    }

    m_lastSpeed = m_speed;
    m_speed = qRound(m_acData.getSpeedKmh());

    if (m_speed != m_lastSpeed)
    {
        Q_EMIT speedUpdated(m_speed);
        qint32 bytesWritten = 0;
        QByteArray speedData;

        // Identification for "wind fan"
        speedData.append(0x02);
        ++bytesWritten;

        qint32 speed = qBound(0, ((m_speed / 3) * 2), 127);
        speedData.append(QChar(speed));
        ++bytesWritten;

        // Fill the buffer with 0x00
        for (qint32 i = bytesWritten; i < 8; ++i)
        {
            speedData.append(QChar(0));
        }

        Q_EMIT sendWindFanData(speedData);
    }

    WheelValueInt slip = getWheelSlip();
    WheelValueFloat calculatedSpeed = getCalculatedSpeed();

    WheelSlipStatus frontLeftSlipStatus = getSlipStatus(slip.frontLeft, calculatedSpeed.frontLeft);
    WheelSlipStatus frontRightSlipStatus = getSlipStatus(slip.frontRight, calculatedSpeed.frontRight);
    WheelSlipStatus rearLeftSlipStatus = getSlipStatus(slip.rearLeft, calculatedSpeed.rearLeft);
    WheelSlipStatus rearRightSlipStatus = getSlipStatus(slip.rearRight, calculatedSpeed.rearRight);

    // Check for bumping effect
    bool bumping = false;
    bumping |= (m_acData.getWheelLoad(Wheel::FrontLeft) == 0.0f);
    bumping |= (m_acData.getWheelLoad(Wheel::FrontRight) == 0.0f);
    bumping |= (m_acData.getWheelLoad(Wheel::RearLeft) == 0.0f);
    bumping |= (m_acData.getWheelLoad(Wheel::RearRight) == 0.0f);
    if (m_lastBumping != bumping)
    {
        Q_EMIT setBumpingState(bumping);
        m_lastBumping = bumping;
    }

    m_maxBrakeValue = 0;
    m_maxGasValue = 0;

    switch (frontLeftSlipStatus)
    {
    case SlippingFromBraking:
    {
        if (slip.frontLeft > m_maxBrakeValue)
        {
            m_maxBrakeValue = slip.frontLeft;
        }

        break;
    }
    case SlippingFromGas:
    {
        if (slip.frontLeft > m_maxGasValue)
        {
            m_maxGasValue = slip.frontLeft;
        }

        break;
    }
    case NotSlipping:
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
    {
        if (slip.frontRight > m_maxBrakeValue)
        {
            m_maxBrakeValue = slip.frontRight;
        }

        break;
    }
    case SlippingFromGas:
    {
        if (slip.frontRight > m_maxGasValue)
        {
            m_maxGasValue = slip.frontRight;
        }

        break;
    }
    case NotSlipping:
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
    {
        if (slip.rearLeft > m_maxBrakeValue)
        {
            m_maxBrakeValue = slip.rearLeft;
        }

        break;
    }
    case SlippingFromGas:
    {
        if (slip.rearLeft > m_maxGasValue)
        {
            m_maxGasValue = slip.rearLeft;
        }

        break;
    }
    case NotSlipping:
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
    {
        if (slip.rearRight > m_maxBrakeValue)
        {
            m_maxBrakeValue = slip.rearRight;
        }

        break;
    }
    case SlippingFromGas:
    {
        if (slip.rearRight > m_maxGasValue)
        {
            m_maxGasValue = slip.rearRight;
        }

        break;
    }
    case NotSlipping:
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
        if (m_maxBrakeValue < 3) m_maxBrakeValue = 3;
        if (m_maxGasValue < 3) m_maxGasValue = 3;
    }

    // Only send if something has changed
    if (dataChanged())
    {
        send();
    }

    // Save current values for comparison with future values
    m_lastMaxBrakeValue = m_maxBrakeValue;
    m_lastMaxGasValue = m_maxGasValue;
}

bool TelemetryReader::dataChanged()
{
    return ((m_lastMaxBrakeValue != m_maxBrakeValue) || (m_lastMaxGasValue != m_maxGasValue));
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
        return NotSlipping;
    }
    else if (calculatedSpeed < (m_speed * BRAKE_INDEX))
    {
        return SlippingFromBraking;
    }
    else if (calculatedSpeed > (m_speed * GAS_INDEX))
    {
        return SlippingFromGas;
    }

    return NotSlipping;
}

void TelemetryReader::sendInitialValues()
{
    QByteArray data;
    for (qint32 i = 0; i < 8; ++i)
    {
        data.append(QChar(0));
    }

    Q_EMIT sendWheelSlipData(data);

    QByteArray speedData;
    speedData.append(0x02);

    // Fill the buffer with 0x00
    for (qint32 i = 1; i < 8; ++i)
    {
        speedData.append(QChar(0));
    }

    Q_EMIT speedUpdated(0);
    Q_EMIT sendWindFanData(speedData);
}

void TelemetryReader::send()
{
    qint32 brakeValue = qBound(0, m_maxBrakeValue, 127);
    qint32 gasValue = qBound(0, m_maxGasValue, 127);

    qDebug() << "brakeValue:" << brakeValue;
    qDebug() << "gasValue:" << gasValue;

    QByteArray data;
    data.append(QChar(0));
    data.append(QChar(brakeValue));
    data.append(QChar(gasValue));

    // Fill the buffer with 0x00
    for (qint32 i = 3; i < 8; ++i)
    {
        data.append(QChar(0));
    }

    Q_EMIT sendWheelSlipData(data);
}
