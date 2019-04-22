#include "data.h"
#include <math.h>

#include <vector>
#include <iostream>
#include <sstream>

Data::Data()
{
    pitch = 0;
    roll  = 0;
    temp  = 0;
}

void Data::removeGravity(int rawX,
                int rawY,
                int rawZ,
                double &accelX,
                double &accelY,
                double &accelZ) {

    const double alpha = 0.5;

    // remove gravity component with low-pass filter
    accelX = rawX * alpha + (accelX * (1.0 - alpha));
    accelY = rawY * alpha + (accelY * (1.0 - alpha));
    accelZ = rawZ * alpha + (accelZ * (1.0 - alpha));
}

void Data::setPitch(int rawX, int rawY, int rawZ) {

    double accelX, accelY, accelZ = 0;
    // remove gravity
    removeGravity(rawX,rawY,rawZ,accelX,accelY,accelZ);

    Data::pitch = ((atan2(accelX, (sqrt(accelY*accelY + accelZ*accelZ))))*180.0)/M_PI;
}

void Data::setRoll(int rawX, int rawY, int rawZ) {

    double accelX, accelY, accelZ = 0;
    // remove gravity
    removeGravity(rawX,rawY,rawZ,accelX,accelY,accelZ);

    Data::roll = ((atan2(-accelY, accelZ))*180.0)/M_PI;
}

void Data::setTemp(int cpuTemp) {
    Data::temp = cpuTemp;
}

int Data::timeToSeconds(string time) {

    int totalSeconds = 0;

    // convert the time string to seconds
    std::vector<std::string> timeParts;
    std::istringstream ss(time);
    std::string item;
    while (getline(ss, item, ':')) {
        timeParts.push_back(item);
    }

    if(timeParts.size() == 3) {
        int hour = std::atoi(timeParts[0].c_str());
        int min = std::atoi(timeParts[1].c_str());
        int sec = std::atoi(timeParts[2].c_str());
        totalSeconds = (hour * 3600) + (min * 60) + sec;
    }
    return totalSeconds;
}
