
#ifndef FD650_h
#define FD650_h
#include "Arduino.h"
class FD650
{
private:

    uint8_t _SCL, _SDA;
    const byte bright[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x00};
    byte maping(uint8_t num);
    void del();
    void start();
    void stop();
    void write_byte(uint8_t addat);
    void sendtoled(uint8_t data, uint8_t this_digit);
    byte read_byte();

public:
    FD650(uint8_t SCL, uint8_t SDA)////SCK_PIN,SDA_PIN
    {
        _SCL = SCL;
        _SDA = SDA;
        pinMode(_SCL, OUTPUT);
        pinMode(_SDA, OUTPUT);
    }; //constructor
    void clean();
    void allOn_bri(byte b, bool EN_DISABLE);///b=Brightness(0-7)levels,bool EN_DISABLE=enable display(1),disable(0)
    void shownum(uint8_t digit, byte Value, bool Dp);
    void holenum(int number, bool lzero);
    uint8_t get_key();
    ~FD650(){};
};

#endif