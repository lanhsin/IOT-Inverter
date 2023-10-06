#include <cstdint>  // uint16_t
#include <ctime>    // time_t

/* File operation */
void getDeviceName(int port, char *deviceName);

/* Byte manipulation */
unsigned short get_u2Byte(unsigned char hByte, unsigned char lByte);
unsigned int get_u4Byte(unsigned short hBytes, unsigned short lBytes);
unsigned short swap_u2Byte(unsigned short Bytes);
void set_bit(unsigned int nr, unsigned int *addr);
void clear_bit(unsigned int nr, unsigned int *addr);
unsigned int get_bit(unsigned int nr, unsigned int addr);

/* CRC manipulation */
uint16_t sum16_calculate(uint8_t *buf, unsigned int len);

/* Get local zero time */
bool local_time_init();
time_t get_local_zero_time(time_t t);