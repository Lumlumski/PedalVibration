#ifndef SERIALTHREAD_FF743A8002DA468BA6F0DE694971841D
#define SERIALTHREAD_FF743A8002DA468BA6F0DE694971841D

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
    qint32 m_waitTimeout = 100;
    bool m_quit = false;
    const bool READ_RESPONSE = false;
};

#endif // SERIALTHREAD_FF743A8002DA468BA6F0DE694971841D
