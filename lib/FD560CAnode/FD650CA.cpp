
#include <FD650CA.h>
#include <Arduino.h>

void FD650CA::start()
{
    pinMode(_SDA, OUTPUT);
    digitalWrite(_SDA, HIGH);
    del();
    digitalWrite(_SCL, HIGH);
    del();
    digitalWrite(_SDA, LOW);
    del();
    digitalWrite(_SCL, LOW);
}

void FD650CA::del()
{
    delayMicroseconds(5); /////2(5micro =100khz)
}

void FD650CA::stop()
{
    digitalWrite(_SDA, LOW);
    del();
    digitalWrite(_SCL, HIGH);
    del();
    digitalWrite(_SDA, HIGH);
    del();
}

void FD650CA::write_byte(byte addat)
{
    for (uint8_t i = 0; i != 8; i++)
    {

        if (addat & 0x80)
        {
            digitalWrite(_SDA, HIGH);
        }
        else
        {
            digitalWrite(_SDA, LOW);
        }
        del();
        digitalWrite(_SCL, HIGH);
        addat = addat << 1;
        digitalWrite(_SCL, LOW);
    }
    digitalWrite(_SDA, HIGH);
    del();
    digitalWrite(_SCL, HIGH);
    del();
    digitalWrite(_SCL, LOW);
}

void FD650CA::shownum(uint8_t digit, uint8_t Value, bool Dp)
{
    byte _Dp = (maping(Value));

    if (Dp)
    {
        bitClear(_Dp, 7);
    }
    else
    {
        bitSet(_Dp, 7);
    }

    if (digit >= 0 && digit < 4)

    {
        sendtoled(_Dp, digit);
    }
}

void FD650CA::allOn_bri(byte b, bool EN_DISABLE)
{
    if (7 < b && b < 0)
    {
        b = 7;
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        start();
        write_byte(0x48 + i * 2);           //0x48 is module address for(Setting the digits befor work)
        write_byte(EN_DISABLE | bright[b]); //display command settings(turning on display & Brightness level)
        stop();
    }
}

uint8_t FD650CA::maping(uint8_t num)
{
    switch (num)
    {
        //   Dgfedcba
    case 0:
        return ~0b00111111;
        break; //ctive hight segments
    case 1:
        return ~0b00000110;
        break;
    case 2:
        return ~0b01011011;
        break;
    case 3:
        return ~0b01001111;
        break;
    case 4:
        return ~0b01100110;
        break;
    case 5:
        return ~0b01101101;
        break;
    case 6:
        return ~0b01111101;
        break;
    case 7:
        return ~0b00000111;
        break;
    case 8:
        return ~0b01111111;
        break;
    case 9:
        return ~0b01101111;
        break;
    case 10:
        return ~0b00000000; ///blank
        break;
    default:
        return ~0b01010010;
        break;
    }
}

void FD650CA::clean()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        sendtoled(0x00, i);
    }
}

void FD650CA::sendtoled(byte _SDAta, byte this_digit)
{
    start();
    write_byte(0x68 + (this_digit * 2)); ///0x68 is the address of the first digit not the address of the module
    write_byte(_SDAta);                  //0x68 +2 is the second digit address//الكفرة هو نفس عنوان RTC
    stop();
}

bool FD650CA::ackn()
{
    bool cc = true;
    pinMode(_SDA, INPUT);
    digitalWrite(_SCL, HIGH);
    del();
    while (cc)
    {
        cc = digitalRead(_SDA);
    }
    digitalWrite(_SCL, LOW);
    del();
    return cc;
}

byte FD650CA::read_byte()
{
    byte dat = 0;
    pinMode(_SDA, INPUT);
    for (uint8_t i = 0; i != 8; i++)
    {
        digitalWrite(_SCL, HIGH);

        if (digitalRead(_SDA))
        {
            bitSet(dat, 7 - i);
        }
        digitalWrite(_SCL, LOW);
    }
    digitalWrite(_SDA, HIGH);
    del();
    digitalWrite(_SCL, HIGH);
    del();
    digitalWrite(_SCL, LOW);

    return dat;
}

uint8_t FD650CA::get_key()
{
    uint8_t key_code = 0;
    start();
    write_byte(0x4F);
    key_code = read_byte();
    stop();
    return key_code;
}
