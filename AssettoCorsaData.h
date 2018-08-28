
/* 
 * File:   AssettoCorsaData.h
 * Author: Simon Plum
 *
 * Created on 9. April 2018, 10:46
 */

#ifndef ASSETTOCORSADATA_H
#define ASSETTOCORSADATA_H

#include "SharedFileOut.h"
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

#endif /* ASSETTOCORSADATA_H */

