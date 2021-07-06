//pinout...
//lcdi2c 5,6  red8,9;green11,10;heater 12,motor 13,buz A2,DHT22_data_pin A3
/*
  top =   0x44(pressed)....0x04(relesed)[ ++]
  right = 0x4C(pressed)....0x0C(relesed) [Next]
  buttom= 0x4F(pressed)....0x0F(relesed) [--]
  left=   0x47(pressed)....0x07(relesed)
  top&right=   0x7C(pressed)....0x3C(relesed) [Setting Manue]
  buttom&left= 0x7F(pressed)....0x07(relesed) [Clock Manue]
*/
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FD650CA.h>
#include <DS3223rab.h>
#include <LiquidCrystal_Software_I2C.h>
#include <SHT31RAB.h>
SHT31RAB sht31(3, 4);
LiquidCrystal_I2C lcd(0x27, 16, 2, 5, 6); // Set the LCD address to 0x27 for a 16 chars and 2 line display
DS3223rab DS3231(8, 9);
FD650CA red(8, 9);
FD650CA green(11, 10);
//**********************
#define heater 12
#define motor 13
#define buz A2
#define DHT22_data_pin A3
#define SHT31_ADD 0x44
uint8_t *ptr;
float heat_factor = 5.0;
float tmp_up;
float tmp_dwn;
float hum = 10.0;
float area;
bool st = 1, flag = 0;
bool Setting_Mode = 0, lc2 = 0;
bool sw = 0;
bool stable = 0;
bool x = 0;
bool t = 0;
bool keystat = 0;
bool save_signal = 0;
unsigned long previousMillis1 = 0;
unsigned long tick = 0;
/////////////////////////
float turning_time = 8.0;
bool _enableRotate = 1;
uint8_t count = 0;
uint8_t clock_manue = 0;
uint8_t counter = 0;
uint8_t alar_count = 0;
float limit_up, limit_dn;
uint8_t hum_trig = 0;
const uint16_t t1_comp = 62500; //1sec =(1 sec)16M/256;
// boolean result[41];             //for DHT11
// unsigned int databytes[5];
// unsigned int checksum;
float m_temp = 0.0;
float factor = 0.0;
//*****************************from arduino to esp
bool fan = true, water = true, door = false, eg = false, lamp = false;
char arryx[31];
char _data[sizeof(float)];
float g[3];
int eeAddress = 0;
unsigned long sig = 0;
bool pm = 0;
unsigned long turnTimeCounter = 0;
bool Motor_Signal = 0;
uint8_t SEC = 0, MIN = 0, HOR = 0;
unsigned long manu_counter = 3000;
uint8_t periods[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //[0]seconds,[1]minuts,[2]hours{instance of time now}

struct data_tobe_send ////sending live data to esp8266(size 16 byte)
{
  float TEMP;
  uint16_t year;
  uint8_t HUMI;
  uint8_t COUNTER;
  uint8_t DAYS;
  uint8_t second;
  uint8_t minuts;
  uint8_t hours;
  uint8_t day_inweek;
  uint8_t day_inmonth;
  uint8_t month_inyear;
  ////////////////////////
  uint8_t GROP; //
  /// GROP= 0, 0, FAN, BATT, DOOR,LAMP,WATER,EGG
  ////////  D7,D6, D5,  D4,  D3,  D2,  D1,   D0
  ////////////////////////////////
  // bool FAN;
  // bool BATT;
  // bool DOOR;
  // bool LAMP;
  // bool WATER;
  // bool EGG;
  uint8_t Esecond;
  uint8_t Eminuts;
  uint8_t Ehours;

} data = {0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct epromsaved
{
  float SETTMP;
  float TMPHI;
  float TMPLO;
  uint8_t PERIOD; //IN HOURS
  uint8_t HHI;
  uint8_t HLO;
  uint8_t HACHIN;
  bool TURN;
  ///RTC data
  uint8_t YEAR1;
  uint8_t MONTH1;
  uint8_t DAY1;
  uint8_t HOUR1;
  uint8_t MINUT1;

} factory_defult = {37.5f, 37.9f, 37.3f, 8, 70, 40, 21, 1, 21, 1, 1, 12, 33};
//  {37.5f, 37.9f, 37.1f, 8, 70, 40, 21, 1};
epromsaved custom = {0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
epromsaved _Custom = {0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; ////for copy original values before saving in panel
epromsaved data_to_save = {0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct Clock
{
  uint8_t second;
  uint8_t minuts;
  uint8_t hours;
  uint8_t day_inweek;
  uint8_t day_inmonth;
  uint8_t month_inyear;
  uint16_t year;
} RTC;

void Get_Time()
{
  ptr = DS3231.rtcRead();
  RTC.second = *(ptr + 0);
  RTC.minuts = *(ptr + 1);
  RTC.hours = *(ptr + 2);
  RTC.day_inweek = *(ptr + 3);
  RTC.day_inmonth = *(ptr + 4);
  RTC.month_inyear = *(ptr + 5);
  RTC.year = *(ptr + 6) + 2000;
}

void zeroticks()
{
  tick = millis();
}

void Motor_Control()
{
  if (millis() > (turning_time * 3000)) ///to prevent turning after reset arduino (turning_time=8sec)
  {
    if ((millis() - turnTimeCounter) < (turning_time * 1000))
    {
      digitalWrite(motor, 1);
    }
    else
    {
      Motor_Signal = 0;
      digitalWrite(motor, 0);
    }
  }
}

void turning_tone()
{
  pinMode(buz, OUTPUT);
  tone(buz, 900, 150);
  delay(225);
  noTone(buz);
  tone(buz, 900, 150);
  delay(225);
  noTone(buz);
  delay(50);
  tone(buz, 900, 150);
  delay(500);
  noTone(buz);
  ////////////////////
  tone(buz, 900, 150);
  delay(180);
  noTone(buz);
  delay(1);

  tone(buz, 900, 150);
  delay(270);
  noTone(buz);
  delay(70);

  tone(buz, 900, 150);
  delay(180);
  noTone(buz);
  delay(1);

  tone(buz, 900, 150);
  delay(370);
  noTone(buz);
  // delay(45);
}

void Time_Remain()
{
  if (custom.TURN)
  {
    //********************************************************generic time
    for (uint8_t i = 3; i < (24 / custom.PERIOD) + 3; i++)
    {
      if ((periods[i] - RTC.hours < custom.PERIOD) && (periods[i] - RTC.hours >= 0))
      {
        HOR = periods[i] - RTC.hours;
        MIN = periods[1] + (59 - periods[1]) - RTC.minuts;
        SEC = periods[0] + (59 - periods[0]) - RTC.second;
        if (HOR == 0 && MIN == 0 && SEC == 0 && !Motor_Signal)
        {
          turning_tone();
          turnTimeCounter = millis();
          Motor_Signal = 1;
        }
      }
    }
  }
  else
  {
    digitalWrite(motor, 0);
  }
}

void Saving_data_Web()
{
  if ((!Wire.available()) && (millis() - sig >= 5000) && (save_signal))
  {
    sig = millis();
    save_signal = 0;
    EEPROM.begin();
    eeAddress = 0;
    eeAddress += sizeof(float);
    EEPROM.put(eeAddress, data_to_save);
    eeAddress = sizeof(float);
    EEPROM.get(eeAddress, custom);
    /////////////////
    for (uint8_t i = 0; i < sizeof(periods); i++)
    {
      periods[i] = 0;
    }
    periods[0] = RTC.second;
    periods[1] = RTC.minuts;
    periods[2] = RTC.hours;
    for (uint8_t i = 3; i < (24 / custom.PERIOD) + 3; i++)
    {
      periods[i] = periods[i - 1] + custom.PERIOD;
      if (periods[i] > 23)
      {
        periods[i] = periods[i] - 24;
      }
    }
    eeAddress = sizeof(float) + sizeof(data_to_save);
    EEPROM.put(eeAddress, periods);
    EEPROM.get(eeAddress, periods);
    // for (uint8_t i = 0; i < sizeof(periods); i++)
    // {
    //   Serial.print(periods[i]);
    //   Serial.print(" | ");
    // }
    // Serial.println("");

    // float *p1;
    // // take address of custom and assign to the pointer
    // p1 = (float *)&custom;
    // // loop thorugh the elements of the struct
    // for (uint8_t cnt = 0; cnt < 3; cnt++)
    // {
    //   // p points to an address of an element in the array; *p gets you the value ofthat address
    //   // print it and next point the pointer to the address of the next element
    //   Serial.print(*(p1++));
    //   Serial.print(" | ");
    // }
    // uint8_t *p;
    // // take address of custom and assign to the pointer
    // p = (uint8_t *)&custom;
    // p += 12;
    // // p++;

    // for (uint8_t cnt = 10; cnt < sizeof(custom) / sizeof(uint8_t); cnt++)
    // {
    //   Serial.print(*(p++));
    //   Serial.print(" | ");
    // }
    // Serial.println("  ");
  }
}

void on_Request() ///on Request do this
{
  switch (arryx[0])
  {
  case 'p':
    Wire.write((byte *)(&custom), sizeof(custom));
    break;
  case 'd':
    Wire.write((byte *)(&data), sizeof(data));
    break;

  case 's':
    // Wire.write((byte *)(&custom), sizeof(custom));
    break;
  default:
    break;
  }
}

void receiveEvent(int howMany)
{
  if (Wire.available() > 0)
  {

    for (unsigned int i = 0; i < sizeof(arryx); i++)
    {
      arryx[i] = Wire.read();
    }
  }
  if (arryx[0] == 's')
  {
    for (unsigned int j = 1; j < 4; j++)
    {
      for (unsigned int i = 0; i < sizeof(float); i++)
      {
        _data[i] = arryx[i + (4 * j)];
      }
      memcpy(&g[j - 1], _data, sizeof(float));
    }
    data_to_save.SETTMP = g[0];
    data_to_save.TMPHI = g[1];
    data_to_save.TMPLO = g[2];
    data_to_save.HHI = char(arryx[16]);
    data_to_save.HLO = char(arryx[17]);
    data_to_save.PERIOD = char(arryx[18]); //IN HOURS
    data_to_save.HACHIN = char(arryx[19]);
    data_to_save.TURN = char(arryx[20]);
    ///////////////////////////////////////
    data_to_save.YEAR1 = char(arryx[21]);
    data_to_save.MONTH1 = char(arryx[22]);
    data_to_save.DAY1 = char(arryx[23]);
    data_to_save.HOUR1 = char(arryx[24]);
    data_to_save.MINUT1 = char(arryx[25]);

    if (char(arryx[24]) > 12)
    {
      pm = 1;
    }
    else
    {
      pm = 0;
    }
    // for (uint16_t i = 0; i < sizeof(arryx); i++)
    // {
    //   Serial.print(toascii(arryx[i]));
    //   Serial.print(" | ");
    // }
    // Serial.println("");
    // rtcWrite(30, toascii(arryx[25]), toascii(arryx[24]), 0, pm, 2, toascii(arryx[23]), toascii(arryx[22]), toascii(arryx[21]) + 2000);
    // 2021-03-26T20:47

    save_signal = 1;
  }
}

float stien_three(byte Thermistor_Pin, float Rdivid, float A, float B, float C)
{
  int Vo;
  double lnR2, R2;
  float T;
  float sum = 0.0;
  uint8_t iteration = 10;
  for (uint8_t i = 0; i < iteration; i++)
  {
    Vo = analogRead(Thermistor_Pin);
    R2 = Rdivid * (1024.0 / (float)Vo - 1.0); /// NTC resistance connected between Vcc & ADC Pin
    lnR2 = log(R2);
    T = 1 / (A + B * lnR2 + C * lnR2 * lnR2 * lnR2);
    T = (T - 273.15);
    sum = T + sum;
  }
  T = sum / float(iteration);
  return T;
}

void Humidity()
{
  if (hum_trig >= 1)
  {
    // pinMode(DHT22_data_pin, OUTPUT);
    // digitalWrite(DHT22_data_pin, LOW);
    // delay(18);
    // digitalWrite(DHT22_data_pin, HIGH);
    // pinMode(DHT22_data_pin, INPUT_PULLUP);
    // for (uint8_t i = 0; i <= 40; i++)
    // {
    //   result[i] = (pulseIn(DHT22_data_pin, HIGH) > 45);
    // }

    // for (uint8_t j = 0; j < 5; j++)
    // {
    //   databytes[j] = 0;

    //   for (uint8_t i = 1; i <= 8; i++)
    //   {
    //     databytes[j] = databytes[j] << 1;
    //     if (result[j * 8 + i])
    //       databytes[j] |= 1;
    //   }
    // }
    //m_temp = ((databytes[2] & 0x7F) * 256 + databytes[3]) / (float)10; //Extract Temperature
    // if ((databytes[2] & 0x80) > 0)
    // {
    //   m_temp = -m_temp; //MSB of Temperature gives it sign
    // }
    // m_temp = m_temp * 1.023;
    // hum = (databytes[0] * 256 + databytes[1]) / (float)10; //Extract Humidity
    // hum *= 100.0;
    //**********************************
    sht31.read(SHT31_ADD, 0x2220);
    m_temp = sht31.tempResult;
    hum = sht31.humidityResult;
    area = custom.SETTMP - m_temp;
    //*************************************
    hum_trig = 0;
  }
}

void Update_data()
{
  // tmp_dwn = stien_three(A0, 10000.0, 9.2842025712E-04, 2.4620685389E-04, 1.9112690439E-07) + 0.08; //9965.0 ///Black wire.Steel old (ok)
  //*************************************************************************************************
  // tmp_dwn = stien_three(A0, 10000.0,1.776109629E-03,1.273236472E-04,5.098492533E-07); ///Gray wire on green segment (ok)
  // tmp_up = stien_three(A1, 10000.0, 7.8786994030E-04, 2.8985556847E-04, -1.3697290359E-07); // Black wire epoxy 10k small baghdad
  // tmp_up = stien_three(A1, 10000.0, 1.3411013609E-03, 1.7326863945E-04, 5.1373528749E-07); ////white wire epoxy 10k baghdad (ok)
  tmp_up = stien_three(A1, 10000.0, 1.286986010E-03, 2.067415798E-04, 1.923761216E-07); ///first one epoxy 1500 dinar ok
  //  if (hum_trig >= 1)
  // {
  //    sht31.read(SHT31_ADD, 0x2220);
  //   m_temp = sht31.tempResult;
  //   hum = sht31.humidityResult;
  //   area = custom.SETTMP - m_temp;
  //   //*************************************
  //   hum_trig = 0;
  // }
  // area = custom.SETTMP - tmp_dwn;
  // data.TEMP = float(tmp_dwn);
  data.TEMP = m_temp;
  data.HUMI = int(hum);
  data.year = RTC.year;
  // data.COUNTER = RTC.second;
  data.second = RTC.second;
  data.DAYS = RTC.day_inmonth;
  data.minuts = RTC.minuts;
  data.hours = RTC.hours;
  data.day_inweek = RTC.day_inweek;
  data.day_inmonth = RTC.day_inmonth;
  data.month_inyear = RTC.month_inyear;
  ////////////////////////
  /// GROP= 0, 0, FAN, BATT, DOOR,LAMP,WATER,EGG
  ////////  D7,D6, D5,  D4,  D3,  D2,  D1,   D0
  bitWrite(data.GROP, 5, fan);                 ///D5 fan
  bitWrite(data.GROP, 4, 1);                   ///D4 battery ture
  bitWrite(data.GROP, 3, door);                ///D3 door
  bitWrite(data.GROP, 2, digitalRead(heater)); ///D2 lamp
  bitWrite(data.GROP, 1, water);               ///D1 water
  bitWrite(data.GROP, 0, eg);                  ///D0 egg

  limit_up = custom.TMPHI;
  limit_dn = custom.TMPLO;
  data.Esecond = SEC;
  data.Eminuts = MIN;
  data.Ehours = HOR;
}

void SevenSegmentDisplay()
{
  green.shownum(2, int(hum * 100) / 10 % 10, 0);
  green.shownum(1, int(hum * 100) / 100 % 10, 1);
  green.shownum(0, int(hum * 100) / 1000 % 10, 0);

  red.shownum(2, int(m_temp * 100) / 10 % 10, 0);
  red.shownum(1, int(m_temp * 100) / 100 % 10, 1);
  red.shownum(0, int(m_temp * 100) / 1000 % 10, 0);
}

bool Delay(float interval)
{
  if (flag)
  {
    if (millis() - previousMillis1 >= interval)
    {
      previousMillis1 = millis();
      st = !st;
    }
  }
  return st;
}

void alarm(uint8_t number_beeb)
{
  if (number_beeb >= 1)
  {
    pinMode(buz, OUTPUT);

    for (uint8_t j = 0; j != number_beeb; j++)
    {
      for (uint8_t i = 0; i != 30; i++)
      {
        digitalWrite(buz, 1);
        delayMicroseconds(500);
        digitalWrite(buz, 0);
        delayMicroseconds(500);
      }
      if (number_beeb >= 2)
      {
        delay(80);
      }
      else
      {
        delayMicroseconds(1);
      }
    }
  }
}

void limit_warning()
{
  if (stable)
  {
    while ((tmp_dwn >= limit_up || tmp_dwn <= limit_dn) && (stable))
    {
      while (!x)
      {
        lcd.clear();
        x = 1;
        lcd.setCursor(2, 0);
        lcd.print("WARNING !!");
        lcd.setCursor(2, 1);
        lcd.print("Temp limit");
        digitalWrite(heater, 0);
        digitalWrite(motor, 0);
      }

      alarm(2);

      while ((red.get_key() == 0x44) && (millis() - tick > 150))
      {
        zeroticks();
        stable = 0;
        lcd.clear();
        x = 0;
      }
    }
  }
}

void Manual_Turn()
{
  if (!sw)
  {
    bool sta = 1;
    zeroticks();
    while ((red.get_key() == 0x47) && !lc2) //left key=0x47H
    {
      if ((millis() - tick > 100))
      {
        zeroticks();
        lcd.clear();
        alarm(1);
        _enableRotate = 0;
        lc2 = 1;
      }
    }
    while (lc2)
    {
      lcd.setCursor(2, 0);
      lcd.print("Rotate Motor");
      lcd.setCursor(2, 1);
      lcd.print("Press Down V");
      while ((red.get_key() == 0x4F) && (millis() - tick > 50)) ////down key =4F pressed
      {
        zeroticks();
        digitalWrite(motor, 1);
      }
      while ((red.get_key() == 0x0F) && (millis() - tick > 50)) ////down key =0F relesed
      {
        zeroticks();
        digitalWrite(motor, 0);
      }

      while ((red.get_key() == 0x47) && (millis() - tick > 100) && sta)
      {
        // zeroticks();
        _enableRotate = 1;
        lc2 = 0;
        digitalWrite(motor, 0);
        alarm(1);
        sta = 0;
      }
    }
  }
}

void Display()
{
  if (!sw)
  {
    lcd.setCursor(0, 0);
    lcd.print("TP=");
    if (tmp_up <= -20)
    {
      lcd.print("OPEN");
    }
    else
      lcd.print(tmp_up);
    lcd.print("/");
    lcd.print(RTC.hours);
    lcd.print(":");
    lcd.print(RTC.minuts);
    lcd.print(":");
    if (RTC.second == 0)
    {
      lcd.print("0   ");
    }
    else
    {
      lcd.print(RTC.second);
    }
    lcd.setCursor(0, 1);
    lcd.print("BN=");
    if (m_temp <= -10)
    {
      lcd.print("OPEN");
    }
    else if (custom.TURN)
    {
      lcd.print(m_temp);
      lcd.print("/");
      lcd.print(HOR);
      lcd.print(":");
      lcd.print(MIN);
      lcd.print(":");
      lcd.print(SEC);
      if (SEC == 0)
      {
        lcd.print("0   ");
      }
      else
      {
        lcd.print(SEC);
      }
    }
    else
    {
      lcd.print(m_temp);
      lcd.print("/");
      lcd.print("STOPED!");
    }
  }
}

uint8_t Plus_Minus(uint8_t featu, uint8_t limhi, uint8_t limlo)
{
  while ((red.get_key() == 0x44) && (millis() - tick > 150))
  {
    zeroticks();
    featu++;
    while (featu > limhi)
    {
      featu = limhi;
    }
  }
  while ((red.get_key() == 0x4F) && (millis() - tick > 150))
  {
    zeroticks();
    featu--;
    while (featu < limlo)
    {
      featu = limlo;
    }
  }
  return featu;
}

void next_button(uint8_t x) //next button
{
  while ((red.get_key() == 0x4C) && (millis() - tick > 200)) //next button
  {
    zeroticks();
    lcd.clear();
    count = x;
    alarm(1);
  }
}

// void preview()
// {
//   lcd.setCursor(0, 0);
//   lcd.print("Temp=");
//   lcd.print(_Custom.SETTMP);
//   lcd.print("C");
//   lcd.setCursor(0, 1);
//   lcd.print("Cyc=");
//   lcd.print(cycle_time);
//   lcd.print(",");
//   lcd.print("Turn=");
//   lcd.print(turning_time);
//   lcd.print(" ");
//   red.get_key();
// }

void summry()
{
  while (count == 9)
  {
    // preview();
    next_button(10);
  }
}

void sub_disp()
{
  lcd.setCursor(4, 0);
  lcd.print("SET TEMP");
  lcd.setCursor(4, 1);
  lcd.print("[");
  lcd.print(_Custom.SETTMP);
  lcd.print("]");
  lcd.print("C");
}

void Previous()
{
  while ((red.get_key() == 0x47) && (millis() - tick > 200))
  {
    zeroticks();
    lcd.clear();
    count--;
    if (count < 1)
    {
      sw = 0;
      Setting_Mode = 0;
      count = 0;
      stable = 0;
      x = 0;
    }
    alarm(1);
  }
}

void Set_temp() // setting set point
{
  zeroticks();
  sub_disp();
  bool a = 0;
  while (count == 1)
  {
    if ((red.get_key() == 0x44))
    {

      if ((millis() - tick < 1000))
      {
        a = 0;
        zeroticks();
      }
      while (a && (red.get_key() == 0x44))
      {
        _Custom.SETTMP += 0.1;

        while (_Custom.SETTMP > 42.5)
        {
          _Custom.SETTMP = 42.5;
        }
        sub_disp();
        if ((red.get_key() == 0x07))
        {
          a = 0;
          zeroticks();
        }
      }
      while (!a && (red.get_key() == 0x44))
      {
        _Custom.SETTMP += 0.1;

        while (_Custom.SETTMP > 42.5)
        {
          _Custom.SETTMP = 42.5;
        }

        sub_disp();

        delay(300);

        if ((millis() - tick > 2500))
        {
          a = 1;
        }
        if ((red.get_key() == 0x07))
        {
          a = 0;
          zeroticks();
        }
      }
    }
    ////*********
    if ((red.get_key() == 0x4F))
    {
      if ((millis() - tick < 1000))
      {
        a = 0;
      }
      while (a && (red.get_key() == 0x4F))
      {
        _Custom.SETTMP -= 0.1;
        while (_Custom.SETTMP < 34.0)
        {
          _Custom.SETTMP = 34.0;
        }
        sub_disp();
        if ((red.get_key() == 0x17))
        {
          a = 0;
          zeroticks();
        }
      }
      while (!a && (red.get_key() == 0x4F))
      {
        delay(300);
        _Custom.SETTMP -= 0.1;
        while (_Custom.SETTMP < 20.5)
        {
          _Custom.SETTMP = 20.5;
        }
        sub_disp();
        if ((millis() - tick > 2000))
        {
          a = 1;
        }
        if ((red.get_key() == 0x17))
        {
          a = 0;
          zeroticks();
        }
      }
    }
    next_button(2);
    Previous();
  }
}

void hitmpAlarm()
{
  while (count == 2)
  {

    while ((red.get_key() == 0x44) && (millis() - tick > 150))
    {
      zeroticks();
      _Custom.TMPHI += 0.05;
      while (_Custom.TMPHI > _Custom.SETTMP + 1.5)
      {
        _Custom.TMPHI = _Custom.SETTMP + 1.5;
      }
    }
    while ((red.get_key() == 0x4F) && (millis() - tick > 150))
    {
      zeroticks();
      _Custom.TMPHI -= 0.05;
      while (_Custom.TMPHI < _Custom.SETTMP + 0.25)
      {
        _Custom.TMPHI = _Custom.SETTMP + 0.25;
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("HIGH TEMP ALARM");
    lcd.setCursor(0, 1);
    lcd.print("  (");
    lcd.print(_Custom.TMPHI);
    lcd.print(") C  ");
    next_button(3);
    Previous();
  }
}

void lotmpAlarm()
{

  while (count == 3)
  {
    while ((red.get_key() == 0x44) && (millis() - tick > 150))
    {
      zeroticks();
      _Custom.TMPLO += 0.05;
      while (_Custom.TMPLO > _Custom.SETTMP - 0.25)
      {
        _Custom.TMPLO = _Custom.SETTMP - 0.25;
      }
    }
    while ((red.get_key() == 0x4F) && (millis() - tick > 150))
    {
      zeroticks();
      _Custom.TMPLO -= 0.05;
      while (_Custom.TMPLO < _Custom.SETTMP - 2.5)
      {
        _Custom.TMPLO = _Custom.SETTMP - 2.5;
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("LOW TEMP ALARM");
    lcd.setCursor(0, 1);
    lcd.print("  (");
    lcd.print(_Custom.TMPLO);
    lcd.print(") C  ");
    next_button(4);
    Previous();
  }
}

void hihumAlarm()
{
  while (count == 4)
  {
    _Custom.HHI = Plus_Minus(_Custom.HHI, 80, 30);
    lcd.setCursor(0, 0);
    lcd.print("HI HUMIDITY ALAR");
    lcd.setCursor(0, 1);
    lcd.print("     (");
    lcd.print(_Custom.HHI);
    lcd.print(")%    ");
    next_button(5);
    Previous();
  }
}

void lohumAlarm()
{
  while (count == 5)
  {
    _Custom.HLO = Plus_Minus(_Custom.HLO, _Custom.HHI - 5, 30);
    lcd.setCursor(0, 0);
    lcd.print("LO HUMIDITY ALAR");
    lcd.setCursor(0, 1);
    lcd.print("    (");
    lcd.print(_Custom.HLO);
    lcd.print(")%  ");
    next_button(6);
    Previous();
  }
}

void turningSetting()
{
  while (count == 6)
  {
    while ((red.get_key() == 0x44) && (millis() - tick > 150))
    {
      zeroticks();
      while (_Custom.TURN)
      {
        _Custom.TURN = !_Custom.TURN;
      }
    }
    while ((red.get_key() == 0x4F) && (millis() - tick > 150))
    {
      zeroticks();
      while (!_Custom.TURN)
      {
        _Custom.TURN = !_Custom.TURN;
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("TURNING SETTING");
    lcd.setCursor(0, 1);

    if (_Custom.TURN)
    {
      lcd.print(" TURN IS ON NOW ");
    }
    else
    {
      lcd.print(" TURN IS OFF NOW");
    }

    next_button(7);
    Previous();
  }
}

void cycle_time_setting() // setting cycle_time
{
  while (count == 7 && _Custom.TURN)
  {
    _Custom.PERIOD = Plus_Minus(_Custom.PERIOD, 12, 2);
    lcd.setCursor(0, 0);
    lcd.print("Set [Cycle Time]");
    lcd.setCursor(0, 1);
    lcd.print("EVERY(");
    lcd.print(_Custom.PERIOD);
    lcd.print(")HOURS");
    next_button(8);
    Previous();
  }
  while (count == 7 && !_Custom.TURN)
  {
    count = 9;
    // summry();
  }
}

void turning_time_setting() // setting turning_time
{
  while (count == 8)
  {
    while ((red.get_key() == 0x44) && (millis() - tick > 150))
    {
      zeroticks();
      turning_time += 0.25;
      while (turning_time > 9.0)
      {
        turning_time = 9.0;
      }
    }
    while ((red.get_key() == 0x4F) && (millis() - tick > 150))
    {
      zeroticks();
      turning_time -= 0.25;
      while (turning_time < 4.0)
      {
        turning_time = 4.0;
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("Set [Turn Time]");
    lcd.setCursor(0, 1);
    lcd.print("Turn =");
    lcd.print(turning_time);
    lcd.print("Second");
    next_button(9);
    Previous();
  }
}

void Hatching_setting()
{
  while (count == 9)
  {
    _Custom.HACHIN = Plus_Minus(_Custom.HACHIN, 32, 18);
    lcd.setCursor(0, 0);
    lcd.print("HACHING DAYS");
    lcd.setCursor(0, 1);
    lcd.print("    (");
    lcd.print(_Custom.HACHIN);
    lcd.print(")  ");
    next_button(10);
    Previous();
  }
}

void Save_Panel()
{
  while (count == 10)
  {
    lcd.setCursor(0, 0);
    lcd.print("[SAVING DATA...]");
    EEPROM.begin();
    eeAddress = 0;
    eeAddress += sizeof(float);
    EEPROM.put(eeAddress, _Custom);
    eeAddress = sizeof(float);
    EEPROM.get(eeAddress, custom);
    delay(500);
    ///////////////
    Get_Time();
    periods[0] = RTC.second;
    periods[1] = RTC.minuts;
    periods[2] = RTC.hours;
    for (uint8_t i = 3; i < (24 / custom.PERIOD) + 3; i++)
    {
      periods[i] = periods[i - 1] + custom.PERIOD;
      if (periods[i] > 23)
      {
        periods[i] = periods[i] - 24;
      }
    }
    eeAddress = sizeof(float) + sizeof(data_to_save);
    EEPROM.put(eeAddress, periods);
    delay(500);
    EEPROM.get(eeAddress, periods);
    //////////
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" [SAVING DONE!] ");
    delay(1000);
    sw = 0;
    Setting_Mode = 0;
    count = 0;
    stable = 0;
    x = 0;
    limit_up = custom.TMPHI;
    limit_dn = custom.TMPLO;
    alarm(2);
    lcd.clear();
  }
}

void Heater_Controller()
{
  if (!Setting_Mode)
  {
    if ((area > 0.0) || (tmp_dwn < custom.SETTMP))
    {
      if (area > 1.5) //1.5 area = custom.SETTMP - tmp_dwn;
      {
        flag = 0;
        digitalWrite(heater, HIGH);
        heat_factor = 4.0; ///heat_factor = 4.0;
      }
      else
      {
        flag = 1;
        heat_factor = 1.3;
        digitalWrite(heater, Delay((area * 2000.0) + 1000.0)); /// Delay((area * 2000.0) + 1000.0));
      }
    }

    if ((area <= 0.0) || ((tmp_up) > (custom.SETTMP + area + heat_factor))) ///0.85
    {
      flag = 0;
      digitalWrite(heater, 0);
    }

    if (abs(area) < 0.01 && (!stable) && (alar_count == 0))
    {
      stable = 1;
      alar_count = 1;
      alarm(1);
    }
    if ((tmp_dwn >= custom.SETTMP - 0.1) & (tmp_dwn <= custom.SETTMP + 0.1)) // alarm  activate zone
    {
      alar_count = 0;
    }
  }
  else
  {
    digitalWrite(heater, 0);
  }
}

void Checking_Keys()
{

  //There is conflict with Rtc after 15 minuts because [i2c address of FD560 = i2c address of DS3231 ](on the same pins)

  while ((red.get_key() == 0x7C) && !Setting_Mode) ///set key=7f on press,3f on relesed k1+k2(dual keys)(Setting manue)
  {
    zeroticks();
    bool condition = 0;
    while (red.get_key() == 0x7C)
    {
      if ((millis() - tick >= 1200))
      {
        condition = 1;
        break;
      }
    }
    while (!keystat && condition)
    {
      if (millis() - tick > 500)
      {
        zeroticks();
        keystat = 1;
        if ((red.get_key() == 0x7C) && !Setting_Mode)
        {
          manu_counter = millis();
          Setting_Mode = 1;
          digitalWrite(heater, 0);
          hum_trig = 0;
          sht31.Break();
          memcpy((void *)&_Custom, (void *)&custom, sizeof(_Custom));
          sw = 1;
          lcd.clear();
          count = 1;
          alarm(1);
        }
      }
    }
    keystat = 0;
  }
}

void Switching_between_Setting()
{
  switch (count)
  {
  case 1:
    Set_temp();
    break;
  case 2:
    hitmpAlarm();
    break;
  case 3:
    lotmpAlarm();
    break;
  case 4:
    hihumAlarm();
    break;
  case 5:
    lohumAlarm();
    break;
  case 6:
    turningSetting();
    break;
  case 7:
    cycle_time_setting();
    break;
  case 8:
    turning_time_setting();
    break;
  case 9:
    Hatching_setting();
    break;
  case 10:
    Save_Panel();
    break;
  default:
    break;
  }
}

uint8_t WHILE_NEXT(uint8_t v)
{
  uint8_t var = v;
  while ((red.get_key() == 0x4C) && (millis() - tick > 200)) //next button
  {
    zeroticks();
    lcd.clear();
    var++;
    alarm(1);
  }
  return var;
}

void EXIT()
{
  while ((red.get_key() == 0x47) && (millis() - tick > 150)) ///Exit
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("EXIT WITHOU SAVE");
    delay(500);
    alarm(1);
    zeroticks();
    clock_manue = 0;
    keystat = 1;
  }
}

void Clock_setting()
{
  while ((red.get_key() == 0x7F) && !clock_manue) ///Clock setting....Buttom & left= 0x7F(pressed)
  {
    zeroticks();
    bool condition = 0;
    while (red.get_key() == 0x7F)
    {
      if ((millis() - tick >= 1200))
      {
        condition = 1;
        break;
      }
    }
    while (!keystat && condition)
    {
      if ((millis() - tick > 500))
      {
        zeroticks();
        digitalWrite(heater, 0);
        uint8_t subhour, submin, subsec, subyear, _year, submonth, subday;
        subhour = RTC.hours;
        submin = RTC.minuts;
        subsec = RTC.second;
        subyear = RTC.year - 2000;
        _year = subyear;
        submonth = RTC.month_inyear;
        subday = RTC.day_inmonth;
        lcd.clear();
        alarm(1);
        lcd.setCursor(2, 0);
        lcd.print(" Set  Clock");
        delay(1000);
        lcd.clear();
        clock_manue++;
        while (clock_manue == 1) ///hours
        {
          lcd.setCursor(3, 0);
          lcd.print("[H]: M : S");
          lcd.setCursor(3, 1);
          subhour = Plus_Minus(subhour, 23, 0);
          if (subhour > 23)
          {
            subhour = 0;
          }
          lcd.print("[");
          lcd.print(subhour);
          lcd.print("]");
          lcd.print(": ");
          lcd.print(submin);
          lcd.print(" :");
          lcd.print(subsec);
          clock_manue = WHILE_NEXT(clock_manue);
          EXIT();
        }
        //////
        while (clock_manue == 2) ///minuts
        {
          lcd.setCursor(3, 0);
          lcd.print("H :[M]: S");
          lcd.setCursor(3, 1);
          submin = Plus_Minus(submin, 59, 0);
          if (submin > 59)
          {
            submin = 0;
          }
          lcd.print(subhour);
          lcd.print(" :");
          lcd.print("[");
          lcd.print(submin);
          lcd.print("]");
          lcd.print(":");
          lcd.print(subsec);
          clock_manue = WHILE_NEXT(clock_manue);
          EXIT();
        }
        /////////
        while (clock_manue == 3) ///seconds
        {
          lcd.setCursor(3, 0);
          lcd.print("H : M :[S]");
          lcd.setCursor(3, 1);
          subsec = Plus_Minus(subsec, 59, 0);
          if (subsec > 59)
          {
            subsec = 0;
          }
          lcd.print(subhour);
          lcd.print(" : ");
          lcd.print(submin);
          lcd.print(":[");
          lcd.print(subsec);
          lcd.print("]");
          while ((red.get_key() == 0x4C) && (millis() - tick > 200)) //next button
          {
            zeroticks();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(" [TIME UPDATED] ");
            delay(500);
            alarm(1);
            lcd.clear();
            clock_manue = 4;
          }
          EXIT();
        }

        while (clock_manue == 4) ///Year
        {
          lcd.setCursor(2, 0);
          lcd.print("[YEAR]/ M / D");
          lcd.setCursor(2, 1);
          subyear = Plus_Minus(subyear, 50, _year);
          if (subyear > 50)
          {
            subyear = 50;
          }
          lcd.print("[");
          lcd.print(subyear + 2000);
          lcd.print("]");
          lcd.print("/");
          lcd.print(submonth);
          lcd.print("/");
          lcd.print(subday);
          clock_manue = WHILE_NEXT(clock_manue);
          EXIT();
        }

        while (clock_manue == 5) ///Month
        {
          lcd.setCursor(3, 0);
          lcd.print("Y / [M] / D");
          lcd.setCursor(1, 1);
          submonth = Plus_Minus(submonth, 12, 1);
          if (submonth > 12)
          {
            submonth = 12;
          }
          lcd.print(subyear + 2000);
          lcd.print("/ [");
          lcd.print(submonth);
          lcd.print("] / ");
          lcd.print(subday);
          clock_manue = WHILE_NEXT(clock_manue);
          EXIT();
        }

        while (clock_manue == 6) ///Day
        {
          lcd.setCursor(3, 0);
          lcd.print("Y / M /[D]");
          lcd.setCursor(1, 1);
          subday = Plus_Minus(subday, 31, 1);
          if (subday > 31)
          {
            subday = 31;
          }
          lcd.print(subyear + 2000);
          lcd.print("/ ");
          lcd.print(submonth);
          lcd.print(" /[");
          lcd.print(subday);
          lcd.print("]");
          while ((red.get_key() == 0x4C) && (millis() - tick > 200)) //next button
          {
            zeroticks();
            // DS3231.rtcWrite(subsec, submin,subhour, 0, 0, 1, subday, submonth, subyear);
            lcd.clear();
            lcd.print(" [SAVING DONE!] ");
            delay(1000);
            clock_manue = 0;
            keystat = 1;
            alarm(2);
          }
          EXIT();
        }
      }
    }
    keystat = 0;
  }
}

void setup()
{
  // rtcWrite(30, 43, 8, 0, 0, 1, 22, 5, 2021);
  lcd.init();
  lcd.backlight();
  // Serial.begin(9600);
  EEPROM.begin();
  //////////////////////////
  // eeAddress += sizeof(float);
  // EEPROM.put(eeAddress, factory_defult);
  /////////////////////////////////////////////
  eeAddress = sizeof(float);
  EEPROM.get(eeAddress, custom);
  for (uint8_t i = 0; i < sizeof(periods); i++)
  {
    periods[i] = 0;
  }
  eeAddress = sizeof(float) + sizeof(data_to_save);
  delay(200);
  EEPROM.get(eeAddress, periods);
  delay(200);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(on_Request);
  analogReference(EXTERNAL);
  pinMode(heater, OUTPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, 0);
  digitalWrite(heater, 0);
  red.clean();
  green.clean();
  red.allOn_bri(2, 1);
  green.allOn_bri(1, 1);
  //**************************************
  limit_up = custom.TMPHI;
  limit_dn = custom.TMPLO;
  tmp_up = stien_three(A1, 10000.0, 1.286986010E-03, 2.067415798E-04, 1.923761216E-07); ///first one epoxy 1500 dinar ok
  // tmp_dwn = stien_three(A0, 10000.0, 9.5569562991E-04, 2.4161535576E-04, 2.1035712840E-07); ///Black wire.Steel old (ok)
  // area = custom.SETTMP - tmp_dwn;
  //***************
  sht31.read(SHT31_ADD, 0x2220);
  m_temp = sht31.tempResult;
  hum = sht31.humidityResult;
  area = custom.SETTMP - m_temp;
  ///***************************
  alarm(2);
  TCCR1A = 0;            ///Reset Timer1;
  TCCR1B |= (1 << CS12); ///Set the Prescalor(1,0,0)256
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS10);
  //////
  TCNT1 = 0;       //initial value
  OCR1A = t1_comp; ///compare value
  /////
  TIMSK1 = (1 << OCIE1A); ///Set the timer 1 interrupt bit enable
  //////
  sei(); //////Enable the Gloable interrupt
  zeroticks();
}

void loop()
{
  Humidity();
  Heater_Controller();
  Checking_Keys();
  Switching_between_Setting();
  Clock_setting();
  Time_Remain();
  Manual_Turn();
  Display();
  SevenSegmentDisplay();
  Motor_Control();
  // limit_warning();
  Saving_data_Web();
  // if ((red.get_key() == 0x44)) ///Preview screen
  // {
  //   lcd.clear();
  //   preview();
  //   delay(4000);
  //   lcd.clear();
  // }
}

ISR(TIMER1_COMPA_vect)
{
  TCNT1 = 0;
  hum_trig++;
  Get_Time();
  Update_data();
  if ((millis() - manu_counter >= (180000)) && (Setting_Mode)) ///3 minuts to return to main if no key pressed
  {
    manu_counter = millis();
    Setting_Mode = 0;
    stable = 0;
    x = 0;
    lcd.clear();
    sw = 0;
    count = 0;
  }
}
