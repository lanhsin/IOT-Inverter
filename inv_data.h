#ifndef INV_DATA_H
#define INV_DATA_H

#include "inv_conf.h"
#include <string>


const unsigned int DC_NUM_MAX = 12;

/* Internal Definition
*  Register value from inverter
*/
enum inv_status {
    Init     ,  // Initial Mode (Power on Mode)
    Wait     ,  // Wait Mode (Standby Mode)
    Check    ,  // Check Mode
    Work     ,  // Work Mode(Line Mode)
    Offline  ,  // Offline Mode
    Fault    ,  // Fault Mode
    MFlash   ,  // Master flash Mode
    SFlash      // Slave flash Mode
};

/* Modbus Status
atWork Time :  OK:      Normal
               Errors:  Disconnect / SelectTimeout/ CrcError / BadSlaveId / DataError
offline Time: Disconnect
*/
enum modbus_status{
    Disconnect      ,   // Not at work time or disconnect to MODBUS
    Normal          ,   // Normal State
    SelectTimoeout  ,   // Can not received any packet from device.
    CrcError        ,   // The CRC of received packet is error.
    BadSlaveId      ,   // The slave id of received packet is error.
    DataError           // Others error.
};

struct inv_data {
    time_t Timestamp;           //timestamp
    char Serial[24];
    inv_status Status;
    unsigned int Pac;           //0.1W
    unsigned int EToday;        //1kW
    unsigned int ETotal;        //0.1kW-H
    unsigned int HTotal;
    unsigned int KWRating;      //1W	//From inverter
    unsigned int Temperature;   //0.1°C
    unsigned int Voltage[DC_NUM_MAX];   //0.1V
    unsigned int Current[DC_NUM_MAX];   //0.1A
    unsigned int Power[DC_NUM_MAX];     //0.01W
    unsigned int R_Voltage;     //0.1V
    unsigned int R_Current;     //0.1A
    unsigned int R_Power;       //0.01W
    unsigned int R_Frequency;   //0.01Hz
    unsigned int S_Voltage;
    unsigned int S_Current;
    unsigned int S_Power;
    unsigned int S_Frequency;
    unsigned int T_Voltage;
    unsigned int T_Current;
    unsigned int T_Power;
    unsigned int T_Frequency;
    std::string Error;

    void clear(void);
    void print(inv_brand brand, unsigned char dc_number);
};

#endif //INV_DATA_H
