#ifndef SETTINGS_698F9153BC7A4C5289DDC784D0BBB172
#define SETTINGS_698F9153BC7A4C5289DDC784D0BBB172

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
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

    qint32 getBumpingIndex() const;
    void setBumpingIndex(const qint32 &bumpingIndex);

    qint32 getWindFanIndex() const;
    void setWindFanIndex(const qint32 &windFanIndex);

    bool isWheelSlipPortActive() const;
    bool isLedFlagPortActive() const;
    bool isWindFanPortActive() const;

    void setWheelSlipPortActive(bool wheelSlipPortActive);
    void setLedFlagPortActive(bool ledFlagPortActive);
    void setWindFanPortActive(bool windFanPortActive);

Q_SIGNALS:
    void wheelSlipPortChanged();
    void ledFlagPortChanged();
    void windFanPortChanged();
    void upsChanged();
    void minimizeWithXChanged();

    void brakeIndexChanged();
    void gasIndexChanged();
    void bumpingIndexChanged();
    void windFanIndexChanged();

    void wheelSlipEnabledChanged();
    void windFanEnabledChanged();
    void ledFlagEnabledChanged();

private:
    explicit Settings(QObject* parent = nullptr);

    bool m_wheelSlipEnabled = false;
    bool m_ledFlagEnabled = false;
    bool m_windFanEnabled = false;
    QString m_wheelSlipPort;
    QString m_ledFlagPort;
    QString m_windFanPort;
    bool m_wheelSlipPortActive = false;
    bool m_ledFlagPortActive = false;
    bool m_windFanPortActive = false;
    qint32 m_ups;
    bool m_minimizeWithX = false;

    qint32 m_brakeIndex;
    qint32 m_gasIndex;
    qint32 m_bumpingIndex;
    qint32 m_windFanIndex;
};

#endif // SETTINGS_698F9153BC7A4C5289DDC784D0BBB172
