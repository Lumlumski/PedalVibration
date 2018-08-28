
/* 
 * File:   AssettoCorsaData.cpp
 * Author: Simon Plum
 * 
 * Created on 9. April 2018, 10:46
 */

#include "AssettoCorsaData.h"

AssettoCorsaData::AssettoCorsaData()
{
    initPhysics();
    initGraphics();
    initStatic();
}

AssettoCorsaData::~AssettoCorsaData()
{
    dismiss(m_physics);
    dismiss(m_graphics);
    dismiss(m_static);
}

void AssettoCorsaData::update()
{
    //Get AC-Data buffers
    m_pfp = (SPageFilePhysics*)m_physics.mapFileBuffer;
    m_pfg = (SPageFileGraphic*)m_graphics.mapFileBuffer;
    m_pfs = (SPageFileStatic*)m_static.mapFileBuffer;
}

AC_STATUS AssettoCorsaData::getStatus()
{
    return m_pfg->status;
}

float AssettoCorsaData::getAccG0()
{
    return m_pfp->accG[0];
}

float AssettoCorsaData::getAccG1()
{
    return m_pfp->accG[1];
}

float AssettoCorsaData::getAccG2()
{
    return m_pfp->accG[2];
}

float AssettoCorsaData::getWheelSlip(Wheel wheel)
{
    switch (wheel)
    {
        case Wheel::FrontLeft:
            return m_pfp->wheelSlip[0];
            
        case Wheel::FrontRight:
            return m_pfp->wheelSlip[1];
            
        case Wheel::RearLeft:
            return m_pfp->wheelSlip[2];
            
        case Wheel::RearRight:
            return m_pfp->wheelSlip[3];
            
        case Wheel::NotSet:
        default:
            break;
    }
    
    return 0.0;
}

float AssettoCorsaData::getWheelAngularSpeed(Wheel wheel)
{
    switch (wheel)
    {
        case Wheel::FrontLeft:
            return m_pfp->wheelAngularSpeed[0];
            
        case Wheel::FrontRight:
            return m_pfp->wheelAngularSpeed[1];
            
        case Wheel::RearLeft:
            return m_pfp->wheelAngularSpeed[2];
            
        case Wheel::RearRight:
            return m_pfp->wheelAngularSpeed[3];
            
        case Wheel::NotSet:
        default:
            break;
    }
    
    return 0.0;
}

float AssettoCorsaData::getTyreRadius(Wheel wheel)
{
    switch (wheel)
    {
        case Wheel::FrontLeft:
            return m_pfs->tyreRadius[0];
            
        case Wheel::FrontRight:
            return m_pfs->tyreRadius[1];
            
        case Wheel::RearLeft:
            return m_pfs->tyreRadius[2];
            
        case Wheel::RearRight:
            return m_pfs->tyreRadius[3];
            
        case Wheel::NotSet:
        default:
            break;
    }
    
    return 0.0;
}

float AssettoCorsaData::getSpeedKmh()
{
    return m_pfp->speedKmh;
}

float AssettoCorsaData::getSuspensionTravel(Wheel wheel)
{
        switch (wheel)
    {
        case Wheel::FrontLeft:
            return m_pfp->suspensionTravel[0];
            
        case Wheel::FrontRight:
            return m_pfp->suspensionTravel[1];
            
        case Wheel::RearLeft:
            return m_pfp->suspensionTravel[2];
            
        case Wheel::RearRight:
            return m_pfp->suspensionTravel[3];
            
        case Wheel::NotSet:
        default:
            break;
    }
    
    return 0.0;
}

float AssettoCorsaData::getSuspensionMaxTravel(Wheel wheel)
{
    switch (wheel)
    {
        case Wheel::FrontLeft:
            return m_pfs->suspensionMaxTravel[0];
            
        case Wheel::FrontRight:
            return m_pfs->suspensionMaxTravel[1];
            
        case Wheel::RearLeft:
            return m_pfs->suspensionMaxTravel[2];
            
        case Wheel::RearRight:
            return m_pfs->suspensionMaxTravel[3];
            
        case Wheel::NotSet:
        default:
            break;
    }
    
    return 0.0;
}

float AssettoCorsaData::getRideHeight(int index)
{
    if ((index == 0) || (index == 1))
    {
        return m_pfp->rideHeight[index];
    }
    
    return 0.0;
}

float AssettoCorsaData::getWheelLoad(Wheel wheel)
{
    switch (wheel)
    {
        case Wheel::FrontLeft:
            return m_pfp->wheelLoad[0];
            
        case Wheel::FrontRight:
            return m_pfp->wheelLoad[1];
            
        case Wheel::RearLeft:
            return m_pfp->wheelLoad[2];
            
        case Wheel::RearRight:
            return m_pfp->wheelLoad[3];
            
        case Wheel::NotSet:
        default:
            break;
    }
    
    return 0.0;
}

void AssettoCorsaData::initPhysics()
{
    TCHAR szName[] = TEXT("Local\\acpmf_physics");
    m_physics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFilePhysics), szName);
    if (!m_physics.hMapFile)
    {
        MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "ACS", MB_OK);
    }
    m_physics.mapFileBuffer = (unsigned char*)MapViewOfFile(m_physics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFilePhysics));
    if (!m_physics.mapFileBuffer)
    {
        MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "ACS", MB_OK);
    }
}

void AssettoCorsaData::initGraphics()
{
    TCHAR szName[] = TEXT("Local\\acpmf_graphics");
    m_graphics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileGraphic), szName);
    if (!m_graphics.hMapFile)
    {
        MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "ACS", MB_OK);
    }
    m_graphics.mapFileBuffer = (unsigned char*)MapViewOfFile(m_graphics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileGraphic));
    if (!m_graphics.mapFileBuffer)
    {
        MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "ACS", MB_OK);
    }
}

void AssettoCorsaData::initStatic()
{
    TCHAR szName[] = TEXT("Local\\acpmf_static");
    m_static.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileStatic), szName);
    if (!m_static.hMapFile)
    {
        MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "ACS", MB_OK);
    }
    m_static.mapFileBuffer = (unsigned char*)MapViewOfFile(m_static.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileStatic));
    if (!m_static.mapFileBuffer)
    {
        MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "ACS", MB_OK);
    }
}

void AssettoCorsaData::dismiss(SMElement element)
{
    UnmapViewOfFile(element.mapFileBuffer);
    CloseHandle(element.hMapFile);
}