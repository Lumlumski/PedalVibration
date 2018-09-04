#include "settings.h"
#include <QDir>
#include <QDebug>

static const QString WHEEL_SLIP_ENABLED = "WheelSlipEnabled";
static const QString LED_FLAG_ENABLED = "LEDFlagEnabled";
static const QString WHEEL_SLIP_PORT = "WheelSlipPort";
static const QString LED_FLAG_PORT = "LEDFlagPort";
static const QString UPS = "UPS";
static const QString MINIMIZE_WITH_X = "MinimizeWithX";

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_wheelSlipEnabled(false)
    , m_ledFlagEnabled(false)
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
    m_ledFlagPort = ledFlagPort;
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
