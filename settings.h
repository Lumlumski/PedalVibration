#ifndef SETTINGS_698F9153BC7A4C5289DDC784D0BBB172
#define SETTINGS_698F9153BC7A4C5289DDC784D0BBB172

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

    qint32 getBrakeIndex() const;
    void setBrakeIndex(const qint32 &brakeIndex);

    qint32 getGasIndex() const;
    void setGasIndex(const qint32 &gasIndex);

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

    qint32 m_brakeIndex;
    qint32 m_gasIndex;
};

#endif // SETTINGS_698F9153BC7A4C5289DDC784D0BBB172
