#include "telemetryreader.h"
#include <QDebug>
#include <QtMath>
#include <QString>
#include <QChar>
#include <QProcess>
#include <QBuffer>
#include <QDataStream>
#include <QtEndian>
#include "settings.h"


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

    Settings* settings = Settings::getInstance();
    (void)connect(settings, &Settings::gasIndexChanged, this, &TelemetryReader::onGasIndexChanged);
    (void)connect(settings, &Settings::brakeIndexChanged, this, &TelemetryReader::onBrakeIndexChanged);
    (void)connect(settings, &Settings::bumpingIndexChanged, this, &TelemetryReader::onBumpingIndexChanged);
    (void)connect(settings, &Settings::windFanIndexChanged, this, &TelemetryReader::onWindFanIndexChanged);

    m_readTimer.setInterval(m_standbyInterval);
}

TelemetryReader::~TelemetryReader()
{
    m_readTimer.stop();
    (void)disconnect(this);
}

void TelemetryReader::run()
{
    if (m_liveInterval == 0)
    {
        qDebug() << "Cannot start read timer. No update rate set";
        return;
    }

    // Initially read settings once
    // Changes will be notified by signals from the settings
    readSettings();

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

void TelemetryReader::onGasIndexChanged()
{
    qint32 newIndex = Settings::getInstance()->getGasIndex();
    m_gasIndex = (static_cast<float>(newIndex) / 100);
}

void TelemetryReader::onBrakeIndexChanged()
{
    qint32 newIndex = Settings::getInstance()->getBrakeIndex();
    m_brakeIndex = (static_cast<float>(100 - newIndex) / 100);
}

void TelemetryReader::onBumpingIndexChanged()
{
    m_bumpingIndex = Settings::getInstance()->getBumpingIndex();
}

void TelemetryReader::onWindFanIndexChanged()
{
    m_windFanIndex = Settings::getInstance()->getWindFanIndex();
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
            Q_EMIT sendInitialValues();
            Q_EMIT speedUpdated(0);
            return;
        }
        else if (status == AC_LIVE)
        {
            if (m_liveInterval > 0)
            {
                m_readTimer.setInterval(m_liveInterval);
            }

            // Reset wheel slip states when switching to live state
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
        Q_EMIT sendInitialValues();
        Q_EMIT speedUpdated(0);
        m_tyreRadius.frontLeft = m_acData.getTyreRadius((Wheel::FrontLeft));
        m_tyreRadius.frontRight = m_acData.getTyreRadius(Wheel::FrontRight);
        m_tyreRadius.rearLeft = m_acData.getTyreRadius(Wheel::RearLeft);
        m_tyreRadius.rearRight = m_acData.getTyreRadius(Wheel::RearRight);
        m_readStaticData = true;
    }

    m_lastSpeed = m_speed;

    qint32 wheelAngularSpeedFL = qRound(m_acData.getWheelAngularSpeed(Wheel::FrontLeft));
    qint32 wheelAngularSpeedFR = qRound(m_acData.getWheelAngularSpeed(Wheel::FrontRight));
    qint32 wheelAngularSpeedRL = qRound(m_acData.getWheelAngularSpeed(Wheel::RearLeft));
    qint32 wheelAngularSpeedRR = qRound(m_acData.getWheelAngularSpeed(Wheel::RearRight));

    if ((wheelAngularSpeedFL <= 0)
            && (wheelAngularSpeedFR <= 0)
            && (wheelAngularSpeedRL <= 0)
            && (wheelAngularSpeedRR <= 0))
    {
        m_speed = 0;
    }
    else
    {
        m_speed = qRound(m_acData.getSpeedKmh());
    }

    if (m_speed != m_lastSpeed)
    {
        Q_EMIT speedUpdated(m_speed);
    }

    if (Settings::getInstance()->getWheelSlipEnabled())
    {
        calculateWheelSlip();
    }

    if (Settings::getInstance()->getLedFlagEnabled())
    {
        calculateLedFlagStatus();
    }

    if (Settings::getInstance()->getWindFanEnabled())
    {
        calculateWindFanSpeed();
    }
}

void TelemetryReader::readSettings()
{
    Settings* settings = Settings::getInstance();
    m_brakeIndex = (static_cast<float>(100 - settings->getBrakeIndex()) / 100);
    m_gasIndex = (static_cast<float>(settings->getGasIndex()) / 100);
    m_bumpingIndex = settings->getBumpingIndex();

    qDebug() << "BrakeIndex:" << m_brakeIndex;
    qDebug() << "GasIndex:" << m_gasIndex;
    qDebug() << "BumpingIndex:" << m_bumpingIndex;
}

void TelemetryReader::calculateWheelSlip()
{
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
        if (m_maxBrakeValue < m_bumpingIndex)
        {
            m_maxBrakeValue = m_bumpingIndex;
        }

        if (m_maxGasValue < m_bumpingIndex)
        {
            m_maxGasValue = m_bumpingIndex;
        }
    }

    // Only send if something has changed
    if (dataChanged())
    {
        quint8 gasValue = static_cast<quint8>(qBound(0, m_maxGasValue, 127));
        quint8 brakeValue = static_cast<quint8>(qBound(0, m_maxBrakeValue, 127));

        Q_EMIT sendWheelSlipValues(gasValue, brakeValue);
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
    else
    {
        qDebug() << "SlipValue:" << slipValue << "| calculated speed:" << calculatedSpeed << "| speed:" << m_speed << "| brakeIndex:" << m_brakeIndex;
        qDebug() << "speed*brakeIndex =" << (m_speed * m_brakeIndex);

        if (calculatedSpeed < (m_speed * m_brakeIndex))
        {
            return SlippingFromBraking;
        }
        else if (calculatedSpeed > (m_speed * m_gasIndex))
        {
            return SlippingFromGas;
        }
    }

    return NotSlipping;
}

void TelemetryReader::calculateLedFlagStatus()
{
    AC_FLAG_TYPE flagStatus =  m_acData.getFlagStatus();
    if (flagStatus == m_lastFlagStatus)
    {
        return;
    }

    Q_EMIT flagStatusUpdated(flagStatus);
    m_lastFlagStatus = flagStatus;

    quint8 flagValue = 0;
    switch (flagStatus)
    {
    case AC_BLUE_FLAG:
        flagValue = 1;
        break;
    case AC_YELLOW_FLAG:
        flagValue = 2;
        break;
    case AC_BLACK_FLAG:
        flagValue = 3;
        break;
    case AC_WHITE_FLAG:
        flagValue = 4;
        break;
    case AC_CHECKERED_FLAG:
        flagValue = 5;
        break;
    case AC_PENALTY_FLAG:
        flagValue = 6;
        break;
    case AC_NO_FLAG:
    default:
        break;
    }

    Q_EMIT sendLedFlagValue(flagValue);
}

void TelemetryReader::calculateWindFanSpeed()
{
    if (m_speed != m_lastSpeed)
    {
        quint8 windFanValue = static_cast<quint8>(qBound(0, ((m_speed / 3) * 2), 127));
        if (m_lastWindFanValue != windFanValue)
        {
            m_lastWindFanValue = windFanValue;
            //TODO add adjustable parameter for speed sensitivity
            Q_EMIT sendWindFanValue(windFanValue);
        }
    }
}
