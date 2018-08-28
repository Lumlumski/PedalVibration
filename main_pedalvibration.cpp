#include <windows.h>
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "SharedFileOut.h"
#include "AssettoCorsaData.h"

using namespace std;

const static int UPS = 10;

const static string PROJECT_NAME = "PedalVibration";
const static string INI_FILE_PATH = "apps/python/" + PROJECT_NAME + "/";
const static string INI_FILE_NAME = PROJECT_NAME + ".ini";

const float PI = 3.1415927;

HANDLE hSerial;
DCB dcbSerialParams = {0};
COMMTIMEOUTS timeouts = {0};

string readSerialPortFromConfig()
{
    ifstream iniFile(INI_FILE_PATH + INI_FILE_NAME);
    string line;
    getline(iniFile, line);
    
    if (line.empty())
    {
        cout << "Error: Config file not found or empty" << endl;
        return string();
    }
    
    while (!line.empty())
    {
        if (line != "[serial]")
        {
            getline(iniFile, line);
            continue;
        }
        
        getline(iniFile, line);
        if (line.substr(0, 8) == "comport=")
        {
            return line.substr(8, (line.length() - 8));
        }
    }
    
    return string();
}

void printIniFile()
{
    ifstream iniFile(INI_FILE_PATH + INI_FILE_NAME);
    string line;
    getline(iniFile, line);
    if (line.empty())
    {
        return;
    }
    
    cout << INI_FILE_NAME << "\n[\n";
    
    while (!line.empty())
    {
        cout << "\t" << line << "\n";
        getline(iniFile, line);
    }
    
    cout << "\n]" << endl;
}

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

bool initSerialPort(string comPort)
{
    cout << "Opening serial port... ";
    comPort.insert(0, "\\\\.\\");  
    hSerial = CreateFile(comPort.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        cout << "Error";
        return false;
    }
    else
    {
        cout << "OK\n";
    }
     
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        cout << "Error getting device state";
        CloseHandle(hSerial);
        return false;
    }
    
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = 1;
    dcbSerialParams.Parity = 0;
    if(SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        cout << "Error setting device parameters";
        CloseHandle(hSerial);
        return false;
    }
 
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(SetCommTimeouts(hSerial, &timeouts) == 0)
    {
        cout << "Error setting timeouts";
        CloseHandle(hSerial);
        return false;
    }
    
    cout << "COM-Port initialized successfully\n";
    return true;
}

void sendSerial(char* data_out)
{
    DWORD bytes_written = 0;
    if(!WriteFile(hSerial, data_out, 8, &bytes_written, NULL))
    {
        cout << "Error\n";
        CloseHandle(hSerial);
        return;
    }
}

void readSerial(char* data_out)
{
    DWORD bytes_read = 0;
    ReadFile(hSerial, data_out, 8, &bytes_read, NULL);
}

void closeSerialPort()
{
    // Close serial port
    cout << "Closing serial port... ";
    if (CloseHandle(hSerial) == 0)
    {
        cout << "Error\n";
        return;
    }
    
    cout << "OK\n";
}

static float TYRE_RADIUS[4] = {0.0, 0.0, 0.0, 0.0};

enum WheelSlipStatus
{
    NotSlipping,
    SlippingFromBraking,
    SlippingFromGas
};

int main(int argc, char *argv[]) {
    
    AssettoCorsaData acData;
    
    std::string comPort = readSerialPortFromConfig();
    if (comPort.empty())
    {
        cout << "Error: COM-Port not found. Please check the COM-Port in " << INI_FILE_NAME << "\n";
        printIniFile();
        while (true)
        {
            Sleep(10000);
            // Endless loop so that the above cout is readable in console
            // The app will be killed by AC
        }
    }
    
    cout << "COM-Port: " << comPort << endl;
    
    // Wait for a valid serial connection
    while (!initSerialPort(comPort))
    {
        Sleep(200);
        cout << "\r";
    }
    
    float fpsCalc = 1000.0/(double)UPS;
    
    struct timeval start;
    struct timeval loopStart;
    struct timeval end;
    struct timeval loopEnd;
    
    gettimeofday(&start, 0);
    int ups = 0;
    
    char oldSerialData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    char serialData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    sendSerial(serialData);
    
    bool readStaticData = false;
    AC_STATUS lastStatus = AC_OFF;
    
    while (true)
    {
        system("cls");
        gettimeofday(&loopStart, 0);
        ups++;
        
        acData.update();
        
        // Send initial values when the game is not in live mode
        AC_STATUS status = acData.getStatus();
        if ((status == AC_OFF) && (lastStatus != AC_OFF))
        {
            return 0;
        }
        
        lastStatus = status;
        
        if (status != AC_LIVE)
        {
            continue;
        }
        
        // Set the tyre radius once
        if (!readStaticData)
        {
            TYRE_RADIUS[0] = acData.getTyreRadius(Wheel::FrontLeft);
            TYRE_RADIUS[1] = acData.getTyreRadius(Wheel::FrontRight);
            TYRE_RADIUS[2] = acData.getTyreRadius(Wheel::RearLeft);
            TYRE_RADIUS[3] = acData.getTyreRadius(Wheel::RearRight);
            
            readStaticData = true;
        }
        
        int slip[4];
        slip[0] = (int)acData.getWheelSlip(Wheel::FrontLeft);
        slip[1] = (int)acData.getWheelSlip(Wheel::FrontRight);
        slip[2] = (int)acData.getWheelSlip(Wheel::RearLeft);
        slip[3] = (int)acData.getWheelSlip(Wheel::RearRight);
        
        // Set cap to 255
        if (slip[0] > 255) slip[0] = 255;
        if (slip[1] > 255) slip[1] = 255;
        if (slip[2] > 255) slip[2] = 255;
        if (slip[3] > 255) slip[3] = 255;
        
        float wheelAngularSpeed0 = acData.getWheelAngularSpeed(Wheel::FrontLeft);
        float wheelAngularSpeed1 = acData.getWheelAngularSpeed(Wheel::FrontRight);
        float wheelAngularSpeed2 = acData.getWheelAngularSpeed(Wheel::RearLeft);
        float wheelAngularSpeed3 = acData.getWheelAngularSpeed(Wheel::RearRight);
        
        float calculatedSpeed0 = ((2 * TYRE_RADIUS[0] * PI * wheelAngularSpeed0 * 60) / 100);
        float calculatedSpeed1 = ((2 * TYRE_RADIUS[1] * PI * wheelAngularSpeed1 * 60) / 100);
        float calculatedSpeed2 = ((2 * TYRE_RADIUS[2] * PI * wheelAngularSpeed2 * 60) / 100);
        float calculatedSpeed3 = ((2 * TYRE_RADIUS[3] * PI * wheelAngularSpeed3 * 60) / 100);
        
        float speed = acData.getSpeedKmh();

        float brakeIndex = 0.98;
        float gasIndex = 1.08;
        
        WheelSlipStatus frontLeftSlipStatus = NotSlipping;
        WheelSlipStatus frontRightSlipStatus = NotSlipping;
        WheelSlipStatus rearLeftSlipStatus = NotSlipping;
        WheelSlipStatus rearRightSlipStatus = NotSlipping;
        
        cout << "Front left wheel: ";
        if (slip[0] == 0)
        {
            cout << "Not slipping" << endl;
        }
        else if (calculatedSpeed0 < (speed * brakeIndex))
        {
            // Slipping from braking
            cout << "Slipping from braking" << endl;
            frontLeftSlipStatus = SlippingFromBraking;
        }
        else if (calculatedSpeed0 > (speed * gasIndex))
        {
            // Slipping from gas
            cout << "Slipping from gas" << endl;
            frontLeftSlipStatus = SlippingFromGas;
        }
        else
        {
            cout << "Not slipping" << endl;
        }
        
        cout << "Front right wheel: ";
        if (slip[1] == 0)
        {
            cout << "Not slipping" << endl;
        }
        else if (calculatedSpeed1 < (speed * brakeIndex))
        {
            // Slipping from braking
            cout << "Slipping from braking" << endl;
            frontRightSlipStatus = SlippingFromBraking;
        }
        else if (calculatedSpeed1 > (speed * gasIndex))
        {
            // Slipping from gas
            cout << "Slipping from gas" << endl;
            frontRightSlipStatus = SlippingFromGas;
        }
        else
        {
            cout << "Not slipping" << endl;
        }
        
        cout << "Rear left wheel: ";
        if (slip[2] == 0)
        {
            cout << "Not slipping" << endl;
        }
        else if (calculatedSpeed2 < (speed * brakeIndex))
        {
            // Slipping from braking
            cout << "Slipping from braking" << endl;
            rearLeftSlipStatus = SlippingFromBraking;
        }
        else if (calculatedSpeed2 > (speed * gasIndex))
        {
            // Slipping from gas
            cout << "Slipping from gas" << endl;
            rearLeftSlipStatus = SlippingFromGas;
        }
        else
        {
            cout << "Not slipping" << endl;
        }
        
        cout << "Rear right wheel: ";
        if (slip[3] == 0)
        {
            cout << "Not slipping" << endl;
        }
        else if (calculatedSpeed3 < (speed * brakeIndex))
        {
            // Slipping from braking
            cout << "Slipping from braking" << endl;
            rearRightSlipStatus = SlippingFromBraking;
        }
        else if (calculatedSpeed3 > (speed * gasIndex))
        {
            // Slipping from gas
            cout << "Slipping from gas" << endl;
            rearRightSlipStatus = SlippingFromGas;
        }
        else
        {
            cout << "Not slipping" << endl;
        }
        
        cout << endl;

        float wheelLoad[4];
        wheelLoad[0] = acData.getWheelLoad(Wheel::FrontLeft);
        wheelLoad[1] = acData.getWheelLoad(Wheel::FrontRight);
        wheelLoad[2] = acData.getWheelLoad(Wheel::RearLeft);
        wheelLoad[3] = acData.getWheelLoad(Wheel::RearRight);
        
        bool bumping = false;
        for (int i = 0; i < 4; ++i)
        {
            if (wheelLoad[i] == 0)
            {
                bumping = true;
                break;
            }
        }
        
        if (frontLeftSlipStatus == SlippingFromBraking)
        {
            serialData[0] = slip[0];
            serialData[1] = 0;
        }
        else
        {
            serialData[0] = 0;
            serialData[1] = slip[0];
        }
        
        if (frontRightSlipStatus == SlippingFromBraking)
        {
            serialData[2] = slip[1];
            serialData[3] = 0;
        }
        else
        {
            serialData[2] = 0;
            serialData[3] = slip[1];
        }

        if (rearLeftSlipStatus == SlippingFromBraking)
        {
            serialData[4] = slip[2];
            serialData[5] = 0;
        }
        else
        {
            serialData[4] = 0;
            serialData[5] = slip[2];
        }
        
        if (rearRightSlipStatus == SlippingFromBraking)
        {
            serialData[6] = slip[3];
            serialData[7] = 0;
        }
        else
        {
            serialData[6] = 0;
            serialData[7] = slip[3];
        }
        
        if (bumping)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (serialData[i] < 3) serialData[i] = 3;
            }
        }
        
        // Only send if something has changed
        if ((serialData[0] != oldSerialData[0])
                || (serialData[1] != oldSerialData[1])
                || (serialData[2] != oldSerialData[2])
                || (serialData[3] != oldSerialData[3])
                || (serialData[4] != oldSerialData[4])
                || (serialData[5] != oldSerialData[5])
                || (serialData[6] != oldSerialData[6])
                || (serialData[7] != oldSerialData[7]))
        {
            sendSerial(serialData);
        }
        
        // Save current values for comparison with future values
        oldSerialData[0] = serialData[0];
        oldSerialData[1] = serialData[1];
        oldSerialData[2] = serialData[2];
        oldSerialData[3] = serialData[3];
        oldSerialData[4] = serialData[4];
        oldSerialData[5] = serialData[5];
        oldSerialData[6] = serialData[6];
        oldSerialData[7] = serialData[7];
        
        // Timing stuff
        gettimeofday(&end, 0);
        float elapsed = timedifference_msec(loopStart, end);
        while (elapsed < fpsCalc)
        {
            // wait
            Sleep(1);
            gettimeofday(&end, 0);
            elapsed = timedifference_msec(loopStart, end);
        }

        gettimeofday(&loopEnd, 0);
        elapsed = timedifference_msec(start, loopEnd);
        if (elapsed >= 1000)
        {
            cout << "UPS: " << ups;
            ups = 0;
            gettimeofday(&start, 0);
        }
    }
    
    closeSerialPort();
    return 0;
}

