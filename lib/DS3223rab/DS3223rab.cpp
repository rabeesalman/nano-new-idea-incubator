
#include <Arduino.h>
#include <DS3223rab.h>

void DS3223rab::Delay()
{
    delayMicroseconds(2); //2*5usec=100kHz
}

void DS3223rab::start_signal()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, HIGH);
    pinMode(cl, OUTPUT);
    digitalWrite(cl, HIGH);
    Delay();
    digitalWrite(da, LOW);
    Delay();
    digitalWrite(cl, LOW);
}

void DS3223rab::stop_signal()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, LOW);
    digitalWrite(cl, HIGH);
    Delay();
    digitalWrite(da, HIGH);
    Delay();
}

bool DS3223rab::ackn()
{
    bool cc = true;
    pinMode(da, INPUT);
    digitalWrite(cl, HIGH);
    Delay();
    while (cc)
    {
        cc = digitalRead(da);
    }
    digitalWrite(cl, LOW);
    Delay();
    return cc;
}

void DS3223rab::nack()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, HIGH);
    digitalWrite(cl, HIGH);
    Delay();
    digitalWrite(cl, LOW);
    Delay();
}

void DS3223rab::Byt_txrx(byte info)
{
    for (unsigned char i = 0; i < 8; i++)
    {
        digitalWrite(da, bitRead(info, 7 - i));
        Delay();
        digitalWrite(cl, HIGH);
        Delay();
        digitalWrite(cl, LOW);
        Delay();
    }
}

byte DS3223rab::read(byte M_add, byte s_add)
{
    start_signal();
    Byt_txrx(M_add << 1); //sending module address (assume write!!!)
    if (!ackn())
    {
        pinMode(da, OUTPUT);
        Byt_txrx(s_add); //send specific address location
        if (!ackn())
        {
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
                    Delay();
                    digitalWrite(cl, LOW);
                    Delay();
                }

                pinMode(da, OUTPUT); /////sending Nack to module
                digitalWrite(da, HIGH);
                digitalWrite(cl, HIGH);
                Delay();
                digitalWrite(cl, LOW);
                pinMode(da, INPUT);
                Delay();
                stop_signal();
            }
            // else
            // {
            //     stop_signal();
            //     Serial.print("nackn when read");
            // }
        }
        // else
        // {
        //     Serial.print("s_add fail");
        // }
    }
    // else
    // {
    //     Serial.print("Module not respond");
    // }
    return xnum;
}

void DS3223rab::write(byte M_add, byte s_add, byte data)
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
            // else
            // {
            //     Serial.print("break");
            // }
        }
        // else
        // {
        //     Serial.print("add reg not resp.");
        // }
    }
    // else
    // {
    //     Serial.print("module not respond");
    // }
    stop_signal();
}

uint8_t DS3223rab::dectobcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

uint8_t DS3223rab::bcdtodec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

byte *DS3223rab::rtcRead()
{
    byte _rtc[7];
    byte h = 0;
    _rtc[0] = bcdtodec(read(DS3223_ADD, SECOND));
    _rtc[1] = bcdtodec(read(DS3223_ADD, MINUT));
    h = read(DS3223_ADD, HOUR);
    if (bitRead(h, 6))
    {
        // Serial.print("  12system");
        // if (bitRead(h, 5))
        // {
        //     Serial.print("PM");
        // }
        // else
        // {
        //     Serial.print("AM");
        // }
        _rtc[2] = bcdtodec(h & 0b00011111);
    }
    else
    {
        // Serial.print("  24system");
        // Serial.print(h, BIN);
        _rtc[2] = bcdtodec(h & 0b00111111);
    }
    _rtc[3] = bcdtodec(read(DS3223_ADD, DAY_IN_WEEK));
    _rtc[4] = bcdtodec(read(DS3223_ADD, DAY_IN_MONTH));
    _rtc[5] = bcdtodec(read(DS3223_ADD, MONTH_IN_YEAR) & 0b00011111);
    _rtc[6] = bcdtodec(read(DS3223_ADD, YEAR)) + 2000;
    _rtc[6] = bcdtodec(read(DS3223_ADD, YEAR)) ;
    memcpy(&result,&_rtc,sizeof(result));
    return (result);
}

void DS3223rab::rtcWrite(uint8_t S, uint8_t M, uint8_t H, bool _12_, bool pm, uint8_t dinwek, uint8_t dinmoth, uint8_t monthiny, uint16_t yer)
{
    write(DS3223_ADD, SECOND, dectobcd(S));
    write(DS3223_ADD, MINUT, dectobcd(M));
    if (_12_)
    {
        if (H < 1 || H > 12)
        {
            H = 12;
        }
        H = dectobcd(H);
        bitSet(H, 6);
        pm ? bitSet(H, 5) : bitClear(H, 5);
        write(DS3223_ADD, HOUR, H);
    }
    else
    {
        if (H > 24 || H < 0)
        {
            H = 0;
        }
        H = H & 0b00111111;
        write(DS3223_ADD, HOUR, dectobcd(H));
    }
    write(DS3223_ADD, DAY_IN_WEEK, dectobcd(dinwek));
    write(DS3223_ADD, DAY_IN_MONTH, dectobcd(dinmoth));
    write(DS3223_ADD, MONTH_IN_YEAR, dectobcd(monthiny & 0b00011111));
    yer < 2000 ? yer = 2000 : yer = yer - 2000;
    write(DS3223_ADD, YEAR, dectobcd(yer));
}
