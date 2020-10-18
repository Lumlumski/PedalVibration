#ifndef SENDER_6C348842166C430B809BD8E73B5AF2FC
#define SENDER_6C348842166C430B809BD8E73B5AF2FC

#include <QObject>
#include <QMap>
#include <bitset>
#include "serialthread.h"
#include "globals.h"


class Sender : public QObject
{
    Q_OBJECT
public:
    explicit Sender(QObject *parent = nullptr);

public Q_SLOTS:
    void onSendInitialValues();

    void onSendWheelSlipValues(quint8 gasValue, quint8 brakeValue);
    void onSendWindFanValue(quint8 value);
    void onSendLedFlagValue(quint8 value);

    void onWheelSlipEnabledChanged();
    void onWindFanEnabledChanged();
    void onLedFlagEnabledChanged();

    void onSelectedPortsChanged();

private Q_SLOTS:
    void onSerialError(const QString &error);

private:
    template <unsigned int N>
    QByteArray bitsetToQByteArray(std::bitset<BYTE_SIZE*N> data);

    //SerialThread m_wheelSlipSerialThread;
    //SerialThread m_ledFlagSerialThread;
    //SerialThread m_windFanSerialThread;

    QMap<QString, SerialThread*> m_serialThreads;

};

#endif // SENDER_6C348842166C430B809BD8E73B5AF2FC
