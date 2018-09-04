#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class SerialThread : public QThread
{
    Q_OBJECT
public:
    explicit SerialThread(QObject* parent = nullptr);
    ~SerialThread() override;

    void transaction(const QString &portName, const QByteArray &request);

Q_SIGNALS:
    void error(const QString &s);

private:
    void run() override;

    QString m_portName;
    QByteArray m_request;
    QMutex m_mutex;
    QWaitCondition m_cond;
    bool m_quit = false;
};

#endif // SERIALTHREAD_H
