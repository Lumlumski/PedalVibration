#include "serialthread.h"
#include <QSerialPort>
#include <QTime>
#include <QDebug>

SerialThread::SerialThread(QObject *parent) :
    QThread(parent)
{
}

SerialThread::~SerialThread()
{
    m_mutex.lock();
    m_quit = true;
    m_cond.wakeOne();
    m_mutex.unlock();
    wait();
}

void SerialThread::transaction(const QString &portName, const QByteArray &request)
{
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_request = request;

    if (!isRunning())
    {
        start();
    }
    else
    {
        m_cond.wakeOne();
    }
}

void SerialThread::run()
{
    qDebug() << "SerialThread::run()";
    bool currentPortNameChanged = false;

    m_mutex.lock();
    QString currentPortName;
    if (currentPortName != m_portName)
    {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    qint32 currentWaitTimeout = m_waitTimeout;
    QByteArray currentRequest = m_request;
    m_mutex.unlock();
    QSerialPort serial;

    if (currentPortName.isEmpty())
    {
        Q_EMIT error("No port name specified");
        return;
    }

    while (!m_quit)
    {
        if (currentPortNameChanged)
        {
            serial.close();
            serial.setPortName(currentPortName);
            serial.setBaudRate(9600);

            if (!serial.open(QIODevice::ReadWrite))
            {
                Q_EMIT error("Can't open " + m_portName + ", error code " + serial.error());
                return;
            }
        }
        // write request
        const QByteArray requestData = currentRequest;

        qDebug() << "Request:";
        for (qint32 i = 0; i < requestData.size(); ++i)
        {
            QChar current = requestData.at(i);
            qDebug() << i << ") " << QString::number(current.toLatin1());
        }

        qint64 bytesSent = serial.write(requestData);
        if (serial.waitForBytesWritten(m_waitTimeout))
        {
            qDebug() << "Sent" << bytesSent << "Bytes";

            if (READ_RESPONSE)
            {
                // Read response
                if (serial.waitForReadyRead(currentWaitTimeout))
                {
                    QByteArray responseData = serial.readAll();
                    while (serial.waitForReadyRead(10))
                    {
                        responseData += serial.readAll();
                    }

                    qDebug() << "Response:";
                    for (qint32 i = 0; i < responseData.size(); ++i)
                    {
                        QChar current = responseData.at(i);
                        qDebug() << i << ") " << QString::number(current.toLatin1());
                    }
                }
            }
        }

        m_mutex.lock();
        m_cond.wait(&m_mutex);
        if (currentPortName != m_portName)
        {
            currentPortName = m_portName;
            currentPortNameChanged = true;
        }
        else
        {
            currentPortNameChanged = false;
        }

        //currentWaitTimeout = m_waitTimeout;
        currentRequest = m_request;
        m_mutex.unlock();
    }
}
