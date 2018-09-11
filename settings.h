#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
public:
    static Settings *getInstance();
    virtual ~Settings();

    void loadSettings();

    bool getWheelSlipEnabled() const;
    void setWheelSlipEnabled(bool wheelSlipEnabled);

    bool getLedFlagEnabled() const;
    void setLedFlagEnabled(bool ledFlagEnabled);

    bool getWindFanEnabled() const;
    void setWindFanEnabled(bool windFanEnabled);

    QString getWheelSlipPort() const;
    void setWheelSlipPort(const QString &port);

    QString getLedFlagPort() const;
    void setLedFlagPort(const QString &ledFlagPort);

    QString getWindFanPort() const;
    void setWindFanPort(const QString &windFanPort);

    qint32 getUps() const;
    void setUps(const qint32 &ups);

    bool getMinimizeWithX() const;
    void setMinimizeWithX(bool minimizeWithX);

Q_SIGNALS:
    void wheelSlipPortChanged();
    void ledFlagPortChanged();
    void windFanPortChanged();
    void upsChanged();
    void minimizeWithXChanged();

private:
    explicit Settings(QObject* parent = nullptr);

    bool m_wheelSlipEnabled;
    bool m_ledFlagEnabled;
    bool m_windFanEnabled;
    QString m_wheelSlipPort;
    QString m_ledFlagPort;
    QString m_windFanPort;
    qint32 m_ups;
    bool m_minimizeWithX;
};

#endif // SETTINGS_H
