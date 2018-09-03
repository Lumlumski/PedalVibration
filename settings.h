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

    QString getPort() const;
    void setPort(const QString &port);

    qint32 getUps() const;
    void setUps(const qint32 &ups);

    bool getMinimizeWithX() const;
    void setMinimizeWithX(bool minimizeWithX);

Q_SIGNALS:
    void portChanged();
    void upsChanged();
    void minimizeWithXChanged();

private:
    explicit Settings(QObject* parent = nullptr);

    QString m_port;
    qint32 m_ups;
    bool m_minimizeWithX;
};

#endif // SETTINGS_H
