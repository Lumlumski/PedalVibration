#include "sender.h"
#include <QDebug>
#include "settings.h"
#include "globals.h"


Sender::Sender(QObject *parent)
    : QObject(parent)
{
    Settings* settings = Settings::getInstance();
    (void)connect(settings, &Settings::wheelSlipEnabledChanged, this, &Sender::onWheelSlipEnabledChanged);
    (void)connect(settings, &Settings::windFanEnabledChanged, this, &Sender::onWindFanEnabledChanged);
    (void)connect(settings, &Settings::ledFlagEnabledChanged, this, &Sender::onLedFlagEnabledChanged);

    (void)connect(settings, &Settings::wheelSlipPortChanged, this, &Sender::onSelectedPortsChanged);
    (void)connect(settings, &Settings::ledFlagPortChanged, this, &Sender::onSelectedPortsChanged);
    (void)connect(settings, &Settings::windFanPortChanged, this, &Sender::onSelectedPortsChanged);
}

void Sender::onSerialError(const QString &error)
{
    qWarning() << "Error in serial thread!" << error;
}

void Sender::onSendInitialValues()
{
    onSendWheelSlipValues(0, 0);
    onSendWindFanValue(0);
    onSendLedFlagValue(0);
}

void Sender::onSendWheelSlipValues(quint8 gasValue, quint8 brakeValue)
{
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        return;
    }

    qDebug() << QString("sendWheelSlipValues(%1, %2)").arg(gasValue).arg(brakeValue);

    QString port = Settings::getInstance()->getWheelSlipPort();
    if (port.isEmpty() || (!Settings::getInstance()->isWheelSlipPortActive()))
    {
        qWarning() << "Wheel slip port not found";
        return;
    }

    std::bitset<BYTE_SIZE*3> data = ((START_BIT << (BYTE_SIZE*2))
                                     | (ID::WheelSlip << (BYTE_SIZE*2))
                                     | (gasValue << BYTE_SIZE)
                                     | (brakeValue));

    QByteArray dataOut = bitsetToQByteArray<3>(data);
    for (qint32 i = 0; i < dataOut.size(); ++i)
    {
        qDebug() << "Byte" << i << ")" << static_cast<unsigned char>(dataOut.at(i));
    }

    SerialThread* thread = m_serialThreads.value(port);
    if (thread == nullptr)
    {
        thread = new SerialThread();
        (void)connect(thread, &SerialThread::error, this, &Sender::onSerialError);
        m_serialThreads.insert(port, thread);
    }

    thread->transaction(port, dataOut);
}

void Sender::onSendWindFanValue(quint8 value)
{
    if (!Settings::getInstance()->getWindFanEnabled())
    {
        return;
    }

    qDebug() << QString("onSendWindFanValue(%1)").arg(value);

    QString port = Settings::getInstance()->getWindFanPort();
    if (port.isEmpty() || (!Settings::getInstance()->isWindFanPortActive()))
    {
        qWarning() << "Wind fan port not found";
        return;
    }

    std::bitset<BYTE_SIZE*2> data = ((START_BIT << BYTE_SIZE)
                                     | (ID::WindFan << BYTE_SIZE)
                                     | (value));

    QByteArray dataOut = bitsetToQByteArray<2>(data);
    for (qint32 i = 0; i < dataOut.size(); ++i)
    {
        qDebug() << "Byte" << i << ")" << static_cast<unsigned char>(dataOut.at(i));
    }

    //m_windFanSerialThread.transaction(port, dataOut);

    SerialThread* thread = m_serialThreads.value(port);
    if (thread == nullptr)
    {
        thread = new SerialThread(this);
        (void)connect(thread, &SerialThread::error, this, &Sender::onSerialError);
        m_serialThreads.insert(port, thread);
    }

    thread->transaction(port, dataOut);
}

void Sender::onSendLedFlagValue(quint8 value)
{
    if (!Settings::getInstance()->getLedFlagEnabled())
    {
        return;
    }

    qDebug() << QString("onSendLedFlagValue(%1)").arg(value);

    QString port = Settings::getInstance()->getLedFlagPort();
    if (port.isEmpty() || (!Settings::getInstance()->isLedFlagPortActive()))
    {
        qWarning() << "LED flag port not found";
        return;
    }

    std::bitset<BYTE_SIZE*2> data = ((START_BIT << BYTE_SIZE)
                                     | (ID::LEDFlag << BYTE_SIZE)
                                     | (value));

    QByteArray dataOut = bitsetToQByteArray<2>(data);
    for (qint32 i = 0; i < dataOut.size(); ++i)
    {
        qDebug() << "Byte" << i << ")" << static_cast<unsigned char>(dataOut.at(i));
    }

    //m_ledFlagSerialThread.transaction(port, dataOut);

    SerialThread* thread = m_serialThreads.value(port);
    if (thread == nullptr)
    {
        qDebug() << "Create" << port << "thread";
        thread = new SerialThread(this);
        (void)connect(thread, &SerialThread::error, this, &Sender::onSerialError);
        m_serialThreads.insert(port, thread);
    }

    thread->transaction(port, dataOut);
}

void Sender::onWheelSlipEnabledChanged()
{
    if (!Settings::getInstance()->getWheelSlipEnabled())
    {
        onSendWheelSlipValues(0, 0);
    }
}

void Sender::onWindFanEnabledChanged()
{
    if (!Settings::getInstance()->getWindFanEnabled())
    {
        onSendWindFanValue(0);
    }
}

void Sender::onLedFlagEnabledChanged()
{
    if (!Settings::getInstance()->getLedFlagEnabled())
    {
        onSendLedFlagValue(0);
    }
}

void Sender::onSelectedPortsChanged()
{
    qDebug() << "onSelectedPortsChanged()";
    QList<QString> threadPorts = m_serialThreads.keys();
    QList<QString> selectedPorts;
    selectedPorts << Settings::getInstance()->getWheelSlipPort();
    selectedPorts << Settings::getInstance()->getLedFlagPort();
    selectedPorts << Settings::getInstance()->getWindFanPort();

    QList<QString> portsToDelete;
    for (QString port : threadPorts)
    {
        if (!selectedPorts.contains(port))
        {
            qDebug() << "Thread" << port << "shall be killed";
            portsToDelete << port;
        }
    }

    for (QString port : portsToDelete)
    {
        SerialThread* thread = m_serialThreads.value(port);
        if (thread != nullptr)
        {
            qDebug() << "Kill" << port << "thread";
            thread->terminate();
            (void)disconnect(thread, &SerialThread::error, this, &Sender::onSerialError);
            delete thread;
            thread = nullptr;
        }

        m_serialThreads.remove(port);
    }
}

template <unsigned int N>
QByteArray Sender::bitsetToQByteArray(std::bitset<BYTE_SIZE*N> data)
{
    QByteArray result;

    quint32 filter = 0xff;
    filter <<= BYTE_SIZE * (N-1);

    for (quint8 i = N; i > 0; --i)
    {
        quint32 dataULong = ((data & std::bitset<BYTE_SIZE*N>(filter)) >> (BYTE_SIZE * (i - 1))).to_ulong();
        unsigned char dataUChar = static_cast<unsigned char>(dataULong);
        result.append(dataUChar);
        filter >>= BYTE_SIZE;
    }

    return result;
}
