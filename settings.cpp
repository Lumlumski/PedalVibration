#include "settings.h"
#include <QDir>
#include <QDebug>

static const QString PORT = "Port";
static const QString UPS = "UPS";
static const QString MINIMIZE_WITH_X = "MinimizeWithX";

Settings::Settings(QObject *parent)
    : QObject(parent)
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
    QString port = settings.value(PORT, QString()).toString();
    if (!port.isEmpty())
    {
        m_port = port;
    }

    qint32 ups = settings.value(UPS, 10).toInt();
    if (ups > 0)
    {
        m_ups = ups;
    }

    m_minimizeWithX = settings.value(MINIMIZE_WITH_X, false).toBool();
}

QString Settings::getPort() const
{
    return m_port;
}

void Settings::setPort(const QString &port)
{
    if (m_port != port)
    {
        qDebug() << "Settings::setPort(" << port << ")";
        m_port = port;
        QSettings().setValue(PORT, m_port);
        //Q_EMIT portChanged();
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
        //Q_EMIT upsChanged();
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
        //Q_EMIT minimizeWithXChanged();
    }
}
