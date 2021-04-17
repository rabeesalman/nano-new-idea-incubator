
#include <Arduino.h>
#include <I2C.h>

void I2C::del()
{
    delayMicroseconds(5); //2*5usec=100kHz
}

void I2C::start_signal()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, HIGH);
    pinMode(cl, OUTPUT);
    digitalWrite(cl, HIGH);
    del();
    digitalWrite(da, LOW);
    del();
    digitalWrite(cl, LOW);
}

void I2C::stop_signal()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, LOW);
    digitalWrite(cl, HIGH);
    del();
    digitalWrite(da, HIGH);
    del();
}

bool I2C::ackn()
{
    bool cc = true;
    pinMode(da, INPUT);
    digitalWrite(cl, HIGH);
    del();
    while (cc)
    {
        // Serial.print(" ok ");Serial.print(w);
        cc = digitalRead(da);
    }
    // w++;
    digitalWrite(cl, LOW);
    del();
    return cc;
}

void I2C::nack()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, HIGH);
    digitalWrite(cl, HIGH);
    del();
    digitalWrite(cl, LOW);
    del();
}

void I2C::Byt_txrx(byte info)
{
    for (unsigned char i = 0; i < 8; i++)
    {
        digitalWrite(da, bitRead(info, 7 - i));
        del();
        digitalWrite(cl, HIGH);
        del();
        digitalWrite(cl, LOW);
        del();
    }
}

byte I2C::read(byte M_add, byte s_add)
{
    start_signal();
    Byt_txrx(M_add << 1); //sending module address (assume write!!!)
    if (!ackn())
    {
        pinMode(da, OUTPUT);
        Byt_txrx(s_add); //send specific address location
        if (!ackn())
        {
            // M_add = (M_add << 1) + 1;
            start_signal(); //repeat start signal for reading
            Byt_txrx((M_add << 1) + 1);
            if (!ackn())
            {
               
                    bool x;
                    for (int i = 0; i < 8; i++) //read the incoming byte & save it to xnum
                    {
                        x = digitalRead(da);
                        bitWrite(xnum, 7 - i, x);
                        digitalWrite(cl, HIGH);
                        del();
                        digitalWrite(cl, LOW);
                        del();
                    }
                  
                pinMode(da, OUTPUT); /////sending Nack to module
                digitalWrite(da, HIGH);
                digitalWrite(cl, HIGH);
                del();
                digitalWrite(cl, LOW);
                pinMode(da, INPUT);
                del();
                stop_signal();
            }
            else
            {
                stop_signal();
                Serial.print("nackn when read");
            }
        }
        else
        {
            Serial.print("s_add fail");
        }
    }
    else
    {
        Serial.print("Module not respond");
    }
    return xnum;
}

void I2C::write(byte M_add, byte s_add, byte data)
{
    start_signal();
    Byt_txrx(M_add << 1); //sending module address
    if (!ackn())
    {
        pinMode(da, OUTPUT);
        Byt_txrx(s_add); //send specific address location
        if (!ackn())
        {
            pinMode(da, OUTPUT);
            Byt_txrx(data); //writing data byte inside the address= s_add
            if (!ackn())
            {
                stop_signal();
                delay(5);
            }
            else
            {
                Serial.print("break");
            }
        }
        else
        {
            Serial.print("add reg not resp.");
        }
    }
    else
    {
        Serial.print("module not respond");
    }
    stop_signal();
}
