#include "settings.h"
#include <QDir>
#include <QDebug>

static const QString WHEEL_SLIP_ENABLED = "WheelSlipEnabled";
static const QString LED_FLAG_ENABLED = "LEDFlagEnabled";
static const QString WIND_FAN_ENABLED = "WindFanEnabled";
static const QString WHEEL_SLIP_PORT = "WheelSlipPort";
static const QString WIND_FAN_PORT = "WindFanPort";
static const QString LED_FLAG_PORT = "LEDFlagPort";
static const QString UPS = "UPS";
static const QString MINIMIZE_WITH_X = "MinimizeWithX";

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_wheelSlipEnabled(false)
    , m_ledFlagEnabled(false)
    , m_windFanEnabled(false)
    , m_ups(10)
    , m_minimizeWithX(false)
{
    loadSettings();
}

Settings* Settings::getInstance()
{
    static Settings* settings;
    if (settings == nullptr)
    {
        settings = new Settings();
    }

    return settings;
}

Settings::~Settings()
{

}

void Settings::loadSettings()
{
    QSettings settings;

    m_wheelSlipEnabled = settings.value(WHEEL_SLIP_ENABLED, false).toBool();
    m_ledFlagEnabled = settings.value(LED_FLAG_ENABLED, false).toBool();
    m_windFanEnabled = settings.value(WIND_FAN_ENABLED, false).toBool();

    QString wheelSlipPort = settings.value(WHEEL_SLIP_PORT, QString()).toString();
    if (!wheelSlipPort.isEmpty())
    {
        m_wheelSlipPort = wheelSlipPort;
    }

    QString ledFlagPort = settings.value(LED_FLAG_PORT, QString()).toString();
    if (!ledFlagPort.isEmpty())
    {
        m_ledFlagPort = ledFlagPort;
    }

    QString windFanPort = settings.value(WIND_FAN_PORT, QString()).toString();
    if (!windFanPort.isEmpty())
    {
        m_windFanPort = windFanPort;
    }

    qint32 ups = settings.value(UPS, 10).toInt();
    if (ups > 0)
    {
        m_ups = ups;
    }

    m_minimizeWithX = settings.value(MINIMIZE_WITH_X, false).toBool();
}

bool Settings::getWheelSlipEnabled() const
{
    return m_wheelSlipEnabled;
}

void Settings::setWheelSlipEnabled(bool wheelSlipEnabled)
{
    if (m_wheelSlipEnabled != wheelSlipEnabled)
    {
        qDebug() << "Settings::setWheelSlipEnabled(" << wheelSlipEnabled << ")";
        m_wheelSlipEnabled = wheelSlipEnabled;
        QSettings().setValue(WHEEL_SLIP_ENABLED, m_wheelSlipEnabled);
    }
}

bool Settings::getLedFlagEnabled() const
{
    return m_ledFlagEnabled;
}

void Settings::setLedFlagEnabled(bool ledFlagEnabled)
{
    if (m_ledFlagEnabled != ledFlagEnabled)
    {
        qDebug() << "Settings::setLedFlagEnabled(" << ledFlagEnabled << ")";
        m_ledFlagEnabled = ledFlagEnabled;
        QSettings().setValue(LED_FLAG_ENABLED, m_ledFlagEnabled);
    }
}

bool Settings::getWindFanEnabled() const
{
    return m_windFanEnabled;
}

void Settings::setWindFanEnabled(bool windFanEnabled)
{
    if (m_windFanEnabled != windFanEnabled)
    {
        qDebug() << "Settings::setWindFanEnabled(" << windFanEnabled << ")";
        m_windFanEnabled = windFanEnabled;
        QSettings().setValue(WIND_FAN_ENABLED, m_windFanEnabled);
    }
}

QString Settings::getWheelSlipPort() const
{
    return m_wheelSlipPort;
}

void Settings::setWheelSlipPort(const QString &port)
{
    if (m_wheelSlipPort != port)
    {
        qDebug() << "Settings::setPort(" << port << ")";
        m_wheelSlipPort = port;
        QSettings().setValue(WHEEL_SLIP_PORT, m_wheelSlipPort);
    }
}

QString Settings::getLedFlagPort() const
{
    return m_ledFlagPort;
}

void Settings::setLedFlagPort(const QString &ledFlagPort)
{
    if (m_ledFlagPort != ledFlagPort)
    {
        qDebug() << "Settings::setPort(" << ledFlagPort << ")";
        m_ledFlagPort = ledFlagPort;
        QSettings().setValue(LED_FLAG_PORT, m_ledFlagPort);
    }
}

QString Settings::getWindFanPort() const
{
    return m_windFanPort;
}

void Settings::setWindFanPort(const QString &windFanPort)
{
    if (m_windFanPort != windFanPort)
    {
        qDebug() << "Settings::setPort(" << windFanPort << ")";
        m_windFanPort = windFanPort;
        QSettings().setValue(WIND_FAN_PORT, m_windFanPort);
    }
}

qint32 Settings::getUps() const
{
    return m_ups;
}

void Settings::setUps(const qint32 &ups)
{
    if (m_ups != ups)
    {
        m_ups = ups;
        QSettings().setValue(UPS, m_ups);
    }
}

bool Settings::getMinimizeWithX() const
{
    return m_minimizeWithX;
}

void Settings::setMinimizeWithX(bool minimizeWithX)
{
    if (m_minimizeWithX != minimizeWithX)
    {
        m_minimizeWithX = minimizeWithX;
        QSettings().setValue(MINIMIZE_WITH_X, m_minimizeWithX);
    }
}
