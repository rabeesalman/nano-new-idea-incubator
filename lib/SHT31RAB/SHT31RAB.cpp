
#include <Arduino.h>
#include <SHT31RAB.h>

void SHT31RAB::del()
{
    delayMicroseconds(4); //2*5usec=100kHz
}

void SHT31RAB::start_signal()
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

void SHT31RAB::stop_signal()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, LOW);
    digitalWrite(cl, HIGH);
    del();
    digitalWrite(da, HIGH);
    del();
}

bool SHT31RAB::ackn()
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

void SHT31RAB::nack()
{
    pinMode(da, OUTPUT);
    digitalWrite(da, HIGH);
    digitalWrite(cl, HIGH);
    del();
    digitalWrite(cl, LOW);
    del();
}

void SHT31RAB::Byt_txrx(byte info)
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

void SHT31RAB::read(byte M_add, uint16_t command) ///Periodic Data Acquisition Mode
                                                  // 0x2130:  high repeatability mps - 2 measurements per second
{
    uint16_t tmp = 0;
    float temp = 0.0;
    uint16_t hum = 0;
    float humidity = 0.0;
    byte MSB_command = (command & 0b1111111100000000) >> 8; //get the MSByte of the 16-bit measurement command
    byte LSB_command = command & 0b0000000011111111;        //get the LSByte of the 16-bit measurement command

    start_signal();

    Byt_txrx(M_add << 1); //sending module address (assume write!!!)

    if (!ackn()) ///akn for module address
    {
        pinMode(da, OUTPUT);
        Byt_txrx(MSB_command); //send MSByte 0-7 of 16-bit measurement command

        if (!ackn())
        {
            pinMode(da, OUTPUT);
            Byt_txrx(LSB_command); //send LSByte 8-15 of 16-bit measurement command

            if (!ackn())
            {
                delay(20);
                start_signal();             //repeat start signal for reading
                Byt_txrx((M_add << 1) + 1); ///Read Mode starting

                if (!ackn())
                {
                    for (uint8_t counter = 0; counter < 6; counter++)
                    {
                        byte xnum = 0x00;

                        for (int i = 0; i < 8; i++) //read the incoming byte & save it to xnum
                        {
                            digitalRead(da) ? bitSet(xnum, 7 - i) : bitClear(xnum, 7 - i);
                            digitalWrite(cl, HIGH);
                            del();
                            digitalWrite(cl, LOW);
                            del();
                        }

                        pinMode(da, OUTPUT); /////sending ackn to module
                        digitalWrite(da, LOW);
                        digitalWrite(cl, HIGH);
                        del();
                        digitalWrite(cl, LOW);
                        pinMode(da, INPUT);
                        del();
                        switch (counter)
                        {
                        case 0:
                            tmp += xnum;
                            tmp = tmp << 8;
                            break;
                        case 1:
                            tmp = tmp + xnum;
                            temp = -45.0 + (175.0 * float(tmp) / 65535.0);
                            tempResult = temp;
                            // Serial.print(temp); Serial.print(" | ");
                            break;
                        case 3:
                            hum += xnum;
                            hum = hum << 8;
                            break;
                        case 4:
                            hum = hum + xnum;
                            humidity = 100.0 * float(hum) / 65535.0;
                            humidityResult = humidity;
                            //  Serial.print(humidity); Serial.println(" ");
                            break;
                        default:
                            xnum = 0;
                            break;
                        }
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
                // else
                // {
                //     stop_signal();
                //     Serial.print("nackn when read");
                // }
            }
            // else
            // {
            //     Serial.print("LSB AKN fail");
            // }
        }
        // else
        // {
        //     Serial.print("MSB AKN fail");
        // }
    }
    // else
    // {
    //     Serial.print("Module Address not respond");
    // }
    // Serial.print(temp);
    // Serial.print(" | ");
    // Serial.print(humidity);
    // Serial.println(" ");
}

void SHT31RAB::Break()
{
    byte MSB_command = (0x3093 & 0b1111111100000000) >> 8; //get the MSByte of the 16-bit break command 0x3093
    byte LSB_command = 0x3093 & 0b0000000011111111;        //get the LSByte of the 16-bit break command
    start_signal();
    Byt_txrx(0x44 << 1);
    if (!ackn())
    {
        pinMode(da, OUTPUT);
        Byt_txrx(MSB_command);

        if (!ackn())
        {
            pinMode(da, OUTPUT);
            Byt_txrx(LSB_command);
            if (!ackn())
            {
                stop_signal();
            }
        }
    }
}