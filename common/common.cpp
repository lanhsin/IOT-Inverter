#include "common.h"
#include <cstring>  // strcpy

void getDeviceName(int port, char *deviceName)
{
    switch (port) {
        case 0:
            strcpy(deviceName, "/dev/ttyUSB0");
            break;
        case 1:
            strcpy(deviceName, "/dev/ttyUSB1");
            break;
        case 2:
            strcpy(deviceName, "/dev/ttyUSB2");
            break;
        case 3:
            strcpy(deviceName, "/dev/ttyUSB3");
            break;
        default:
            strcpy(deviceName, "");
            break;
    }
}

unsigned short get_u2Byte(unsigned char hByte, unsigned char lByte)
{
    return (hByte << 8) | lByte;
}

unsigned int get_u4Byte(unsigned short hBytes, unsigned short lBytes)
{
    return (hBytes << 16) | lBytes;
}

unsigned short swap_u2Byte(unsigned short Bytes)
{
    unsigned char lByte = Bytes & 0xff;
    unsigned char hByte = Bytes >> 8;
    return (lByte << 8) | hByte;
}

void set_bit(unsigned int nr, unsigned int *addr)
{
    *addr  |= (1 << nr);
}

void clear_bit(unsigned int nr, unsigned int *addr)
{
    *addr  &= ~(1 << nr);
}

unsigned int get_bit(unsigned int nr, unsigned int addr)
{
    return 1 & (addr >> nr);
}

uint16_t sum16_calculate(uint8_t *buf, unsigned int len)
{
    unsigned int sum = 0;
    for (unsigned int i = 0; i < len ; i++) {
        sum += buf[i];
    }
	return sum;
}

bool local_time_init()
{
    time_t t = std::time(nullptr);
    struct tm tm_local;
    tm_local = *localtime_r(&t, &tm_local);
    if (strncmp(tm_local.tm_zone, "CST", 3) == 0)
        return true;
    else
        return false;
}

time_t get_local_zero_time(time_t t)
{
    struct tm tm_local;
    tm_local = *localtime_r(&t, &tm_local);
    tm_local.tm_hour = 0;
    tm_local.tm_min = 0;
    tm_local.tm_sec = 0;
    return mktime(&tm_local); // converts local calendar time to time since epoch
}