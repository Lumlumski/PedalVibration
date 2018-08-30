#include "telemetryreader.h"
#include <QDebug>
#include <QtMath>

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
    if ((status == AC_OFF) && (m_lastStatus != AC_OFF))
    {
        m_readTimer.setInterval(m_standbyInterval);
        m_lastStatus = status;
        return;
    }
    else if ((status == AC_LIVE) && (m_lastStatus != AC_LIVE) && (m_liveInterval > 0))
    {
        m_readTimer.setInterval(m_liveInterval);
    }

    m_lastStatus = status;

    if (status != AC_LIVE)
    {
        qDebug() << "Not live";
        m_readTimer.setInterval(m_standbyInterval);
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

    // Let everything vibrate if bumping was detected
    if (bumping)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (m_serialData[i] < 3) m_serialData[i] = 3;
        }
    }

    // Only send if something has changed
    if (( m_serialData[0] != m_lastSerialData[0])
            || ( m_serialData[1] != m_lastSerialData[1])
            || ( m_serialData[2] != m_lastSerialData[2])
            || ( m_serialData[3] != m_lastSerialData[3])
            || ( m_serialData[4] != m_lastSerialData[4])
            || ( m_serialData[5] != m_lastSerialData[5])
            || ( m_serialData[6] != m_lastSerialData[6])
            || ( m_serialData[7] != m_lastSerialData[7]))
    {
        // TODO do send
        //sendSerial( m_serialData);
        qDebug() << "Sending these values:";
        QDebug debug = qDebug();
        debug << "[";
        for (qint32 i = 0; i < 8; ++i)
        {
            debug << m_serialData[i];
            if (i < 7) debug << ", ";
        }

        debug << "]";
    }

    // Save current values for comparison with future values
    for (qint32 i = 0; i < 8; ++i)
    {
        m_lastSerialData[i] = m_serialData[i];
    }
}

WheelValueInt TelemetryReader::getWheelSlip()
{
    WheelValueInt slip;
    slip.frontLeft = static_cast<qint32>(m_acData.getWheelSlip(Wheel::FrontLeft));
    slip.frontRight = static_cast<qint32>(m_acData.getWheelSlip(Wheel::FrontRight));
    slip.rearLeft = static_cast<qint32>(m_acData.getWheelSlip(Wheel::RearLeft));
    slip.rearRight = static_cast<qint32>(m_acData.getWheelSlip(Wheel::RearRight));

    // Set cap to 255
    if (slip.frontLeft > 255) slip.frontLeft = 255;
    if (slip.frontRight > 255) slip.frontRight = 255;
    if (slip.rearLeft > 255) slip.rearLeft = 255;
    if (slip.rearRight > 255) slip.rearRight = 255;

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
