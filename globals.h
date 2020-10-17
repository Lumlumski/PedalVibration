#ifndef GLOBALS_91B700E529094B50A400A45F5F951EAE
#define GLOBALS_91B700E529094B50A400A45F5F951EAE

static const quint8 BYTE_SIZE = 0x08;
static const quint8 START_BIT = 0x80;

enum ID
{
    WheelSlip = 0x00,
    LEDFlag = 0x01,
    WindFan = 0x02
};

struct WheelValueInt
{
    qint32 frontLeft;
    qint32 frontRight;
    qint32 rearLeft;
    qint32 rearRight;

    WheelValueInt()
    {
        frontLeft = 0;
        frontRight = 0;
        rearLeft = 0;
        rearRight = 0;
    }
};

struct WheelValueFloat
{
    float frontLeft;
    float frontRight;
    float rearLeft;
    float rearRight;

    WheelValueFloat()
    {
        frontLeft = 0.0f;
        frontRight = 0.0f;
        rearLeft = 0.0f;
        rearRight = 0.0f;
    }
};

enum WheelSlipStatus
{
    NotSlipping,
    SlippingFromBraking,
    SlippingFromGas
};


#endif // GLOBALS_91B700E529094B50A400A45F5F951EAE
