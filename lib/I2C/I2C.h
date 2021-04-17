
#include <Arduino.h>
#ifndef I2C_h
#define I2C_h
class I2C
{
private:

    unsigned char da, cl;
    byte xnum=0;
    void del();
    void start_signal();
    void stop_signal();
    bool ackn();
    void nack();
    void Byt_txrx(byte info);

public:

    I2C(unsigned char clpin, unsigned char datpin)
    {
        da = datpin;
        cl = clpin;

    };
    byte read(byte M_add, byte s_add);
    void write(byte M_add, byte s_add, byte data);
    ~I2C(){};
};

#endif