
#include "Arduino.h"
#ifndef FD650CA_h
#define FD650CA_h

class FD650CA
{
private:
    uint8_t _SCL, _SDA;
    const uint8_t bright[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00};
    uint8_t maping(uint8_t num);
    void del();
    void start();
    void stop();
    void write_byte(byte addat);
    void sendtoled(byte data, byte this_digit);
    bool ackn();
    uint8_t read_byte();
public:
    FD650CA(uint8_t SCL, uint8_t SDA)
    {
        _SCL = SCL;
        _SDA = SDA;
        pinMode(_SCL, OUTPUT);
        pinMode(_SDA, OUTPUT);
    }; //constructor
    void clean();
    void allOn_bri(byte b, bool EN_DISABLE);
    void shownum(uint8_t digit, uint8_t Value, bool Dp);
    void holenum(int number, bool lzero);
    uint8_t get_key();
    ~FD650CA(){};
};

#endif