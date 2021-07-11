#include <Arduino.h>
#ifndef DS3223rab_h
#define DS3223rab_h
#define DS3223_ADD 0x68
#define SECOND 0
#define MINUT 1
#define HOUR 2
#define DAY_IN_WEEK 3
#define DAY_IN_MONTH 4
#define MONTH_IN_YEAR 5
#define YEAR 6

class DS3223rab
{
private:
    unsigned char da, cl;
    byte xnum = 0;
    void Delay();
    void start_signal();
    void stop_signal();
    bool ackn();
    void nack();
    void Byt_txrx(byte info);
    uint8_t dectobcd(const uint8_t val);
    uint8_t bcdtodec(const uint8_t val);
    uint8_t read(byte M_add, byte s_add);
    void write(byte M_add, byte s_add, byte data);

public:
    DS3223rab(unsigned char clpin, unsigned char datpin)
    {
        da = datpin;
        cl = clpin;
    }
    uint8_t _rtc[7];
    uint8_t *rtcRead();
    void rtcWrite(uint8_t S, uint8_t M, uint8_t H, bool _12_, bool pm, uint8_t dinwek, uint8_t dinmoth, uint8_t monthiny, uint16_t yer);
    ~DS3223rab(){};
};

#endif