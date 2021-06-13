#include <Arduino.h>
#ifndef Stien_h
#define Stien_h

class Stien
{
private:
    byte _Therm_Pin;
    uint8_t _vcc_pin;
    int Vo;
    float logR2, R2, _R1, T, _c1, _c2, _c3;

public:
    Stien(byte Therm_Pin, uint8_t vcc_pin, float Rdivid, float A, float B, float C) ////Constructor
    {
        _Therm_Pin = Therm_Pin;
        _vcc_pin = vcc_pin;
        pinMode(_vcc_pin, OUTPUT);
        digitalWrite(_vcc_pin, 0);
        _R1 = Rdivid;
        _c1 = A;
        _c2 = B;
        _c3 = C;
    }
    float get_temp();
    ~Stien(){}; ///Distractor
};

#endif