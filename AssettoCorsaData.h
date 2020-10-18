#ifndef ASSETTOCORSADATA_BAFC17206BF9423AA9B6615821D526C3
#define ASSETTOCORSADATA_BAFC17206BF9423AA9B6615821D526C3

#include "sharedfileout.h"
#include <windows.h>

struct SMElement
{
    HANDLE hMapFile;
    unsigned char* mapFileBuffer;
};

enum Wheel
{
    NotSet,
    FrontLeft,
    FrontRight,
    RearLeft,
    RearRight
};

class AssettoCorsaData
{
public:
    AssettoCorsaData();
    ~AssettoCorsaData();
    
    void update();
    
    AC_STATUS getStatus();
    float getAccG0();
    float getAccG1();
    float getAccG2();
    
    float getWheelSlip(Wheel wheel);
    float getWheelAngularSpeed(Wheel wheel);
    float getTyreRadius(Wheel wheel);
    
    float getSpeedKmh();
    
    float getSuspensionTravel(Wheel wheel);
    float getSuspensionMaxTravel(Wheel wheel);
    
    float getRideHeight(int index);
    
    float getWheelLoad(Wheel wheel);

    AC_FLAG_TYPE getFlagStatus();
    
private:
    void initPhysics();
    void initGraphics();
    void initStatic();
    void dismiss(SMElement element);
    
    SPageFilePhysics* m_pfp;
    SPageFileGraphic* m_pfg;
    SPageFileStatic* m_pfs;
    
    SMElement m_graphics;
    SMElement m_physics;
    SMElement m_static;
};

#endif // ASSETTOCORSADATA_BAFC17206BF9423AA9B6615821D526C3

