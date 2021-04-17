
#include <Arduino.h>
#include <FD650.h>

void FD650::start()
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

void FD650::del()
{
    delayMicroseconds(5); /////2(5micro =100khz)
}

void FD650::stop()
{
    digitalWrite(_SDA, LOW);
    del();
    digitalWrite(_SCL, HIGH);
    del();
    digitalWrite(_SDA, HIGH);
    del();
}

void FD650::write_byte(byte addat)
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

void FD650::shownum(unsigned char digit, byte Value, bool Dp)
{
    byte _Dp;

    if (Dp)
    {
        _Dp = 0b10000000;
    }
    else
    {
        _Dp = 0b00000000;
    }
    if (digit >= 0 && digit < 4)
    {
        sendtoled((maping(Value)) | _Dp, digit);
    }
}

void FD650::allOn_bri(byte b, bool EN_DISABLE)
{
    if (7 < b && b < 0)
    {
        b = 7;
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        start();
        write_byte(0x48 + i * 2);           ///0x48 is module address for writing.0x49 for reading
        write_byte(EN_DISABLE | bright[b]); ///display command settings(turning on display)
        stop();
    }
}

byte FD650::maping(uint8_t num)
{
    switch (num)
    {
        //   Dgfedcba////commen cathod
    case 0:
        return 0b00111111; //3F
        break;             //active hight segments (Commn Cathod Display Unit)
    case 1:
        return 0b00000110;
        break;
    case 2:
        return 0b01011011;
        break;
    case 3:
        return 0b01001111;
        break;
    case 4:
        return 0b01100110;
        break;
    case 5:
        return 0b01101101;
        break;
    case 6:
        return 0b01111101;
        break;
    case 7:
        return 0b00000111;
        break;
    case 8:
        return 0b01111111;
        break;
    case 9:
        return 0b01101111;
        break;
    case 10:
        return 0b00000000; ///blank
        break;
    ///////////////
    case 11:
        return 0b01101101; ///s
        break;
    case 12:
        return 0b01110111; ///a
        break;
    case 13:
        return 0b00111110; ///v
        break;
    case 14:
        return 0b01111001; ///e
        break;
    case 15:
        return 0b01111000; ///t
        break;
    case 16:
        return 0b01110110; ///h
        break;
    case 17:
        return 0b00111110; ///u
        break;
    case 18:
        return 0x39; ///c
        break;
    default:
        return 0b01010010;
        break;
    }
}

void FD650::clean()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        sendtoled(0x00, i);
    }
}

void FD650::holenum(int number, bool lzero)
{
    byte ons, tes, hud, tho;
    ons = maping(number / 1000 % 10);
    tes = maping(number / 100 % 10); //| 0b10000000;///for decimal point
    hud = maping(number / 10 % 10);
    tho = maping(number % 10);
    byte grop[4] = {ons, tes, hud, tho};
    for (uint8_t i = 0; i < 4; i++)
    {
        if (lzero || 1000 <= number)
        {
            sendtoled(grop[i], i);
        }
        else if (0 <= number && number < 10)
        {
            grop[0] = maping(10);
            grop[1] = maping(10);
            grop[2] = maping(10);
            sendtoled(grop[i], i);
        }
        else if (10 <= number && number < 100)
        {
            grop[0] = maping(10);
            grop[1] = maping(10);
            sendtoled(grop[i], i);
        }
        else if (100 <= number && number < 1000)
        {
            grop[0] = maping(10);
            sendtoled(grop[i], i);
        }
    }
}

void FD650::sendtoled(uint8_t _SDAta, uint8_t this_digit)
{
    start();
    write_byte(0x68 + (this_digit * 2)); ///0x68 is the address of the first digit not the address of the module
    write_byte(_SDAta);
    stop();
}

byte FD650::read_byte()
{
    byte dat = 0;
    pinMode(_SDA, INPUT);
    for (int i = 0; i != 8; i++)
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

uint8_t FD650::get_key()
{
    uint8_t key_code = 0;
    start();
    write_byte(0x4F);
    key_code = read_byte();
    stop();
    return key_code;
}
