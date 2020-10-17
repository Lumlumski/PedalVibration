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

    (void)connect(&m_wheelSlipSerialThread, &SerialThread::error, this, &Sender::onSerialError);
    (void)connect(&m_ledFlagSerialThread, &SerialThread::error, this, &Sender::onSerialError);
    (void)connect(&m_windFanSerialThread, &SerialThread::error, this, &Sender::onSerialError);
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

    m_wheelSlipSerialThread.transaction(port, dataOut);
}

void Sender::onSendWindFanValue(quint8 value)
{
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

    m_windFanSerialThread.transaction(port, dataOut);
}

void Sender::onSendLedFlagValue(quint8 value)
{
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

    m_ledFlagSerialThread.transaction(port, dataOut);
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
