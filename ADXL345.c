/*
 * ADXL345.c
 *
 *  Created on: 9 Apr 2019
 *      Author: Sinead Murtagh
 *      (Original code provided by Dr. Derek Molloy)
 */

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <stdint.h>
#include <string.h>

#define DATAX0 0x32
#define DATAX1 0x33

#define DATAY0 0x34
#define DATAY1 0x35

#define DATAZ0 0x36
#define DATAZ1 0x37

#define POWER_CTL 0x2D
#define FIFO_MODE 0x38

// Convert binary coded decimal to normal decimal numbers
int bcdToDec(uint8_t b)
{
    return ((b / 16) * 10 + (b % 16));
}

// Convert normal decimal numbers to binary coded decimal
uint8_t decToBcd(int d)
{
    return(uint8_t)((d / 10 * 16) + (d % 10));
}

// READ DATA FROM A REGISTER
// where param address is the register address
uint8_t readByte(uint8_t* address)
{
    /* How to use address:
     * write to to the specified register (address)
     * read returned value
     */

   int file;
   uint8_t temp[1];

    if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return -1;
    }

    if (ioctl(file, I2C_SLAVE, 0x53) < 0) {
        perror("Failed to connect to the RTC\n");
        return -1;
    }

    // first write to get specified register
    if (write(file, address, 1) != 1) {
        perror("Failed to write to the RTC register\n");
        return -1;
    }

    // now read from specified register
    if (read(file, temp, 1) != 1) {
        perror("Failed to read from the RTC register\n");
        return 1;
    }

    close(file);
    return temp[0];
}

// WRITE DATA TO A REGISTER
// where param addressAndValue is a char array:
// first byte is the register address
// second byte is the data to write to register
bool writeByte(uint8_t* addressAndValue)
{
    /* How to use addressAndValue:
     * byte0 - the register address
     * byte1 - the value to write to this address
     */

    int file;

    if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return false;
    }

    if (ioctl(file, I2C_SLAVE, 0x53) < 0) {
        perror("Failed to connect to the RTC\n");
        return false;
    }

    // write value to the specified register
    if (write(file, addressAndValue, 2) != 2) {
        perror("Failed to reset the RTC register data\n");
        return false;
    }

    close(file);
    return true;
}

// enable Accelerometer
bool enableAccelerometer()
{
    uint8_t writeBuf[2];

    // write sampling-mode to FIFO register
    writeBuf[0] = FIFO_MODE;
    writeBuf[1] = 0x80;
    if (writeByte(writeBuf) != true) {
        return false;
    }

    // set bit3 HIGH in POWER_CTL register
    writeBuf[0] = POWER_CTL;
    writeBuf[1] = 0x80; // enable measurements
    if (writeByte(writeBuf) != true) {
        return false;
    }
    return true;
}

// disable Accelerometer
bool disableAccelerometer()
{
    uint8_t writeBuf[2];

    // set bit3 LOW in POWER_CTL register
    writeBuf[0] = FIFO_MODE;
    writeBuf[1] = 0;
    if (writeByte(writeBuf) != true) {
        return false;
    }
    return true;
}

// read Accelerometer Data
void readAccelerometerData(int& x, int& y, int& z)
{
    uint8_t address[1];

    // get X data
    address[0] = DATAX0;
    int x0 = readByte(address);
    if (x0 < 0) {
        perror("Failed to read X data0 from Accelerometer\n");
    }
    address[0] = DATAX1;
    int x1 = readByte(address);
    if (x1 < 0) {
        perror("Failed to read X data1 from Accelerometer\n");
    }
    x = bcdToDec((x1<<8)|(x0));

    // get Y data
    address[0] = DATAY0;
    int y0 = readByte(address);
    if (y0 < 0) {
        perror("Failed to read Y data0 from Accelerometer\n");
    }
    address[0] = DATAY1;
    int y1 = readByte(address);
    if (y1 < 0) {
        perror("Failed to read Y data1 from Accelerometer\n");
    }
    y = bcdToDec((y1<<8)|(y0));

    // get Z data
    address[0] = DATAZ0;
    int z0 = readByte(address);
    if (z0 < 0) {
        perror("Failed to read Z data0 from Accelerometer\n");
    }
    address[0] = DATAZ1;
    int z1 = readByte(address);
    if (z1 < 0) {
        perror("Failed to read Z data1 from Accelerometer\n");
    }
    z = bcdToDec((z1<<8)|(z0));
}
