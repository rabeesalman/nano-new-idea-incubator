
#include <Arduino.h>
#ifndef SHT31RAB_h
#define SHT31RAB_h
class SHT31RAB
{
private:
    unsigned char da, cl;
    void del();
    void start_signal();
    void stop_signal();
    bool ackn();
    void nack();
    void Byt_txrx(byte info);

public:
    SHT31RAB(unsigned char clpin, unsigned char datpin)
    {
        da = datpin;
        cl = clpin;
    }
    void read(byte Module_address, uint16_t commands); // 0x2130:  high repeatability mps - 2 measurements per second
    float tempResult;
    float humidityResult;
    ~SHT31RAB(){};
};

#endif