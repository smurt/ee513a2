#ifndef DATA_H
#define DATA_H

#include <string>
using namespace std;

class Data
{
private:
    double pitch;
    double roll;
    int temp;

    static void removeGravity(int,int,int,double&,double&,double&);

public:
    Data();
    void setPitch(int,int,int); // calculates pitch from latest accelerometer data
    void setRoll(int,int,int);  // calculates roll from latest accelerometer data
    void setTemp(int);          // calculates CPU temp from latest RaspberryPi data
    int timeToSeconds(string time); // calculates time in seconds from RaspberryPi data

    double getPitch() {return pitch;}
    double getRoll() {return roll;}
    int getTemp() {return temp;}
};
#endif // DATA_H
