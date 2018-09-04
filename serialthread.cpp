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
    bool currentPortNameChanged = false;

    m_mutex.lock();
    QString currentPortName;
    if (currentPortName != m_portName)
    {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    //int currentWaitTimeout = m_waitTimeout;
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

            if (!serial.open(QIODevice::ReadWrite))
            {
                Q_EMIT error("Can't open " + m_portName + ", error code " + serial.error());
                return;
            }
        }
        // write request
        const QByteArray requestData = currentRequest;
        serial.write(requestData);

//        if (serial.waitForBytesWritten(m_waitTimeout))
//        {
//            // read response
//            if (serial.waitForReadyRead(currentWaitTimeout))
//            {
//                QByteArray responseData = serial.readAll();
//                while (serial.waitForReadyRead(10))
//                {
//                    responseData += serial.readAll();
//                }

//                const QString response = QString::fromUtf8(responseData);
//                Q_EMIT this->response(response);
//            }
//            else
//            {
//                Q_EMIT timeout("Wait read response timeout " + QTime::currentTime().toString());
//            }
//        }
//        else
//        {
//            Q_EMIT timeout("Wait write request timeout " + QTime::currentTime().toString());
//        }

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
