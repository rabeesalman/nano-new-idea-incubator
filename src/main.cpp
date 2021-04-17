#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <FD650CA.h>
#include <I2C.h>
//pinout...
//rs =7,en=6,d4=5,d5=4,d6=3,d7=2,red8,9;green11,10;heater 12,motor 13,buz A2,DHT22_data_pin A3
FD650CA red(8, 9);
FD650CA green(11, 10);
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//**********************
#define heater 12
#define motor 13
#define buz A2
#define DHT22_data_pin A3
double heat_factor = 5.0;
double tmp_up;
double tmp_dwn;
float hum = 10.0;
double area;
bool st = 1, flag = 0;
unsigned long previousMillis1 = 0.0;
unsigned long tick = 0.0;
unsigned int cnt = 0, seconds = 0, minuts = 0;
// double Sum = 0.0;
bool lc1 = 0, lc2 = 0;
// unsigned long mil = 0;
bool sw = 0;
unsigned int cycle_time = 120;
float turning_time = 8.0;
bool _enable = 1;
uint8_t count = 0;
uint8_t counter = 0;
bool stable = 0;
uint8_t alar_count = 0;
double limit_up, limit_dn;
bool x = 0;
uint8_t hum_trig = 0;
const uint16_t t1_comp = 62500; //1sec =(1 sec)16M/256;
boolean result[41];             //for DHT11
unsigned int databytes[5];
unsigned int checksum;
float m_temp = 0.0;
bool b = 0;
float factor = 0.0;
double x2;
bool t = 0;
//*****************************from arduino to esp
bool lamp = false;
bool fan = true, water = true, door = false, eg = false;
float h;
char arryx[31];
char _data[sizeof(float)];
float g[3];
String num;
int eeAddress = 0;
bool save_signal = 0;
unsigned long sig = 0;
bool pm = 0;
struct data_tobe_send
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

} data = {0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
epromsaved data_to_save = {0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
////**************
#define DS3231_ADD 0x68 //0x68 same address for FD650
#define SECOND 0        //the register address 0
#define MINUT 1
#define HOUR 2
#define DAY_IN_WEEK 3
#define DAY_IN_MONTH 4
#define MONTH_IN_YEAR 5
#define YEAR 6
I2C DS3231(8, 9);

struct main
{
  uint8_t second;
  uint8_t minuts;
  uint8_t hours;
  uint8_t day_inweek;
  uint8_t day_inmonth;
  uint8_t month_inyear;
  uint16_t year;
} RTC;

uint8_t dectobcd(const uint8_t val)
{
  return ((val / 10 * 16) + (val % 10));
}

uint8_t bcdtodec(const uint8_t val)
{
  return ((val / 16 * 10) + (val % 16));
}

void rtcRead()
{
  byte h = 0;
  RTC.second = bcdtodec(DS3231.read(DS3231_ADD, SECOND));
  RTC.minuts = bcdtodec(DS3231.read(DS3231_ADD, MINUT));
  h = DS3231.read(DS3231_ADD, HOUR);
  if (bitRead(h, 6))
  {
    // Serial.print("  12system");
    if (bitRead(h, 5))
    {
      // Serial.print("PM");
    }
    else
    {
      // Serial.print("AM");
    }
    RTC.hours = bcdtodec(h & 0b00011111);
  }
  else
  {
    // Serial.print("  24system");
    // Serial.print(h, BIN);
    RTC.hours = bcdtodec(h & 0b00111111);
  }
  RTC.day_inweek = bcdtodec(DS3231.read(DS3231_ADD, DAY_IN_WEEK));
  RTC.day_inmonth = bcdtodec(DS3231.read(DS3231_ADD, DAY_IN_MONTH));
  RTC.month_inyear = bcdtodec(DS3231.read(DS3231_ADD, MONTH_IN_YEAR) & 0b00011111);
  RTC.year = bcdtodec(DS3231.read(DS3231_ADD, YEAR)) + 2000;
}

void rtcWrite(uint8_t S, uint8_t M, uint8_t H, bool _12_, bool pm, uint8_t dinwek, uint8_t dinmoth, uint8_t monthiny, uint16_t yer)
{
  DS3231.write(DS3231_ADD, SECOND, dectobcd(S));
  DS3231.write(DS3231_ADD, MINUT, dectobcd(M));
  if (_12_)
  {
    if (H < 1 || H > 12)
    {
      H = 12;
    }
    H = dectobcd(H);
    bitSet(H, 6);
    pm ? bitSet(H, 5) : bitClear(H, 5);
    DS3231.write(DS3231_ADD, HOUR, H);
  }
  else
  {
    if (H > 24 || H < 0)
    {
      H = 0;
    }
    H = H & 0b00111111;
    DS3231.write(DS3231_ADD, HOUR, dectobcd(H));
  }
  DS3231.write(DS3231_ADD, DAY_IN_WEEK, dectobcd(dinwek));
  DS3231.write(DS3231_ADD, DAY_IN_MONTH, dectobcd(dinmoth));
  DS3231.write(DS3231_ADD, MONTH_IN_YEAR, dectobcd(monthiny & 0b00011111));
  yer < 2000 ? yer = 2000 : yer = yer - 2000;
  DS3231.write(DS3231_ADD, YEAR, dectobcd(yer));
}

void saving_data()
{
  if (!Wire.available() && millis() - sig >= 5000 && save_signal)
  {
    sig = millis();
    save_signal = 0;
    EEPROM.begin();
    eeAddress = 0;
    eeAddress += sizeof(float);
    EEPROM.put(eeAddress, data_to_save);
    eeAddress = sizeof(float);
    EEPROM.get(eeAddress, custom);
    // counter++;
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
    data_to_save.PERIOD = toascii(arryx[18]); //IN HOURS
    data_to_save.HHI = toascii(arryx[16]);
    data_to_save.HLO = toascii(arryx[17]);
    data_to_save.HACHIN = toascii(arryx[19]);
    data_to_save.TURN = toascii(arryx[20]);

    ///////////////////////////////////////
    data_to_save.YEAR1 = toascii(arryx[21]);
    data_to_save.MONTH1 = toascii(arryx[22]);
    data_to_save.DAY1 = toascii(arryx[23]);
    data_to_save.HOUR1 = toascii(arryx[24]);
    data_to_save.MINUT1 = toascii(arryx[25]);
    if (toascii(arryx[24]) > 12)
    {
      pm = 1;
    }
    else
    {
      pm = 0;
    }
    // rtcWrite(30, toascii(arryx[25]), toascii(arryx[24]), 0, pm, 2, toascii(arryx[23]), toascii(arryx[22]), toascii(arryx[21]) + 2000);
    // 2021-03-26T20:47

    save_signal = 1;
  }
}

double stien_three(byte Thermistor_Pin, float Rdivid, float A, float B, float C)
{
  int Vo;
  double lnR2, R2, T;
  double sum = 0.0;
  for (uint8_t i = 0; i < 100; i++)
  {
    Vo = analogRead(Thermistor_Pin);
    R2 = Rdivid * (1024.0 / (float)Vo - 1.0); /// NTC resistance connected between Vcc & ADC Pin
    lnR2 = log(R2);
    T = 1 / (A + B * lnR2 + C * lnR2 * lnR2 * lnR2);
    T = (T - 273.15);
    sum = T + sum;
  }
  T = sum / 100.00;
  return T;
}

void humidity()
{
  while (hum_trig == 2)
  {
    pinMode(DHT22_data_pin, OUTPUT);
    digitalWrite(DHT22_data_pin, LOW);
    delay(18);
    digitalWrite(DHT22_data_pin, HIGH);
    pinMode(DHT22_data_pin, INPUT_PULLUP);
    for (uint8_t i = 0; i <= 40; i++)
    {
      result[i] = (pulseIn(DHT22_data_pin, HIGH) > 45);
    }

    for (uint8_t j = 0; j < 5; j++)
    {
      databytes[j] = 0;

      for (uint8_t i = 1; i <= 8; i++)
      {
        databytes[j] = databytes[j] << 1;
        if (result[j * 8 + i])
          databytes[j] |= 1;
      }
    }
    m_temp = ((databytes[2] & 0x7F) * 256 + databytes[3]) / (float)10; //Extract Temperature
    if ((databytes[2] & 0x80) > 0)
    {
      m_temp = -m_temp; //MSB of Temperature gives it sign
    }
    m_temp = m_temp * 1.023;
    hum = (databytes[0] * 256 + databytes[1]) / (float)10; //Extract Humidity
    hum *= 100.0;
    hum_trig = 0;
  }
}

void update_data()
{
  tmp_dwn = stien_three(A0, 10000.0, 9.2842025712E-04, 2.4620685389E-04, 1.9112690439E-07) + 0.08; //9965.0 ///Black wire.Steel old (ok)
  //*************************************************************************************************
  // tmp_dwn = stien_three(A0, 10000.0,1.776109629E-03,1.273236472E-04,5.098492533E-07); ///Gray wire on green segment (ok)
  // tmp_up = stien_three(A1, 10000.0, 7.8786994030E-04, 2.8985556847E-04, -1.3697290359E-07); // Black wire epoxy 10k small baghdad
  // tmp_up = stien_three(A1, 10000.0, 1.3411013609E-03, 1.7326863945E-04, 5.1373528749E-07); ////white wire epoxy 10k baghdad (ok)
  tmp_up = stien_three(A1, 10000.0, 1.286986010E-03, 2.067415798E-04, 1.923761216E-07); ///first one epoxy 1500 dinar ok
  area = custom.SETTMP - tmp_dwn;

  data.TEMP = float(tmp_dwn);
  data.year = RTC.year;
  data.HUMI = hum / 100;
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
}

void di()
{
  int ee = hum;
  green.shownum(2, ee / 10 % 10, 0);
  green.shownum(1, ee / 100 % 10, 1);
  green.shownum(0, ee / 1000 % 10, 0);

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

      while ((red.get_key() == 0x47) && (millis() - tick > 150))
      {
        tick = millis();
        stable = 0;
        lcd.clear();
        x = 0;
      }
    }
  }
}

void manual_turn()
{
  tick = millis();
  while ((red.get_key() == 0x5f) && !lc2) ///left key=5f
  {
    if ((millis() - tick > 100))
    {
      tick = millis();
      lcd.clear();
      alarm(1);
      _enable = 0;
      lc2 = 1;
    }
  }
  while (lc2)
  {
    lcd.setCursor(2, 0);
    lcd.print("Rotate Motor");
    lcd.setCursor(2, 1);
    lcd.print("Press Down V");
    while ((red.get_key() == 0x57) && (millis() - tick > 50)) ////down key =57 pressed
    {
      tick = millis();
      digitalWrite(motor, 1);
    }
    while ((red.get_key() == 0x17) && (millis() - tick > 50)) ////down key =17 relesed
    {
      tick = millis();
      digitalWrite(motor, 0);
    }

    while ((red.get_key() == 0x5f) && (millis() - tick > 100))
    {
      // tick = millis();
      _enable = 1;
      lc2 = 0;
      digitalWrite(motor, 0);
      alarm(1);
    }
  }
}

void turning_tone()
{
  while (b)
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
    b = 0;
  }
}

void auto_turn()
{
  unsigned int _cycle_time = cycle_time * 60;

  if (_enable)
  {
    if (seconds < _cycle_time) //5 minuts
    {
      digitalWrite(motor, 0);
    }
    if (seconds >= _cycle_time)
    {
      digitalWrite(motor, 1);
    }
    if (seconds > (_cycle_time) + (turning_time))
    {
      digitalWrite(motor, 0);
      seconds = 0;
      b = 1;
      lcd.clear();
    }
  }
}

void display()
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
    // lcd.print(x2);
    lcd.print("/");
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
    if (tmp_dwn <= -20)
    {
      lcd.print("OPEN");
    }
    else
      lcd.print(tmp_dwn);
    lcd.print("/");
    lcd.print("SP:");
    lcd.print(custom.SETTMP);
    // lcd.print("A:");
    // lcd.print((area * 2000.0 + (1000.0)) / 1000);
  }
}

void while_(uint8_t x)
{
  while ((red.get_key() == 0x4f) && (millis() - tick > 200))
  {
    tick = millis();
    lcd.clear();
    count = x;
    alarm(1);
  }
}

void sub_disp()
{
  lcd.setCursor(4, 0);
  lcd.print("SET TEMP");
  lcd.setCursor(4, 1);
  lcd.print("[");
  lcd.print(custom.SETTMP);
  lcd.print("]");
  lcd.print("C");
}

void manu1() ////for setting set point
{
  sub_disp();

  bool a = 0;

  while (count == 1)
  {
    if ((red.get_key() == 0x47))
    {

      if ((millis() - tick < 1000))
      {
        a = 0;
        tick = millis();
      }
      while (a && (red.get_key() == 0x47))
      {
        custom.SETTMP += 0.01;

        while (custom.SETTMP > 48.25)
        {
          custom.SETTMP = 48.25;
        }
        sub_disp();
        if ((red.get_key() == 0x07))
        {
          a = 0;
          tick = millis();
        }
      }
      while (!a && (red.get_key() == 0x47))
      {
        custom.SETTMP += 0.01;

        while (custom.SETTMP > 48.25)
        {
          custom.SETTMP = 48.25;
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
          tick = millis();
        }
      }
    }
    ////*********
    if ((red.get_key() == 0x57))
    {

      if ((millis() - tick < 1000))
      {
        a = 0;
      }
      while (a && (red.get_key() == 0x57))
      {
        custom.SETTMP -= 0.01;
        while (custom.SETTMP < 12.25)
        {
          custom.SETTMP = 12.25;
        }

        sub_disp();

        if ((red.get_key() == 0x17))
        {
          a = 0;
          tick = millis();
        }
      }
      while (!a && (red.get_key() == 0x57))
      {
        delay(300);
        custom.SETTMP -= 0.01;
        while (custom.SETTMP < 12.25)
        {
          custom.SETTMP = 12.25;
        }

        sub_disp();

        if ((millis() - tick > 2000))
        {
          a = 1;
        }
        if ((red.get_key() == 0x17))
        {
          a = 0;
          tick = millis();
        }
      }
    }
    while_(2);
  }
}

void manu2() ////for setting set point
{
  while (count == 2)
  {
    while ((red.get_key() == 0x47) && (millis() - tick > 150))
    {
      tick = millis();
      cycle_time++;
      while (cycle_time > 360)
      {
        cycle_time = 1;
      }
    }
    while ((red.get_key() == 0x57) && (millis() - tick > 150))
    {
      tick = millis();
      cycle_time--;
      while (cycle_time < 1)
      {
        cycle_time = 360;
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("Set [Cycle Time]");
    lcd.setCursor(0, 1);
    lcd.print("Cycle=");
    lcd.print(cycle_time);
    lcd.print("Minute");
    while_(3);
  }
}

void manu3() ////for setting set point
{
  while (count == 3)
  {
    while ((red.get_key() == 0x47) && (millis() - tick > 150))
    {
      tick = millis();
      turning_time += 0.25;
      while (turning_time > 9.0)
      {
        turning_time = 9.0;
      }
    }
    while ((red.get_key() == 0x57) && (millis() - tick > 150))
    {
      tick = millis();
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
    while_(4);
  }
}

void preview()
{
  lcd.setCursor(0, 0);
  lcd.print("Temp=");
  lcd.print(custom.SETTMP);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Cyc=");
  lcd.print(cycle_time);
  lcd.print(",");
  lcd.print("Turn=");
  lcd.print(turning_time);
  lcd.print(" ");
  red.get_key();
}

void summry() ////for setting set point
{
  while (count == 4)
  {
    preview();
    while_(5);
  }
}

void save_func()
{
  while (count == 5)
  {
    lcd.setCursor(0, 0);
    lcd.print("[SAVING DATA...]");
    // custom.SETTMP *= 100.00;
    // sp0 = (int)custom.SETTMP;
    // sp0 = sp0 / 100 % 100;
    // sp1 = (int)custom.SETTMP;
    // sp1 = sp1 % 100;
    // eeprom.write(0x50, 0, sp0);
    // delay(500);
    // eeprom.write(0x50, 1, sp1);
    delay(500);
    // eeprom.write(0x50, 2, cycle_time);
    delay(500);
    // turning_time *= 100.00;
    // sp0 = (int)turning_time;
    // sp0 = sp0 / 100 % 100;
    // sp1 = (int)turning_time;
    // sp1 = sp1 % 100;
    // eeprom.write(0x50, 3, sp0);
    // delay(500);
    // eeprom.write(0x50, 4, sp1);
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print(" [SAVING DONE.] ");
    delay(500);
    // sp0 = eeprom.read(0x50, 0);
    // sp0 *= 100;
    // sp1 = eeprom.read(0x50, 1);
    // custom.SETTMP = (sp1) + (sp0);
    // custom.SETTMP /= 100;
    // cycle_time = eeprom.read(0x50, 2);
    // sp0 = eeprom.read(0x50, 3);
    // sp0 *= 100;
    // sp1 = eeprom.read(0x50, 4);
    // turning_time = (sp1) + (sp0);
    // turning_time /= 100;
    sw = 0;
    lc1 = 0;
    count = 0;
    stable = 0;
    x = 0;
    limit_up = custom.TMPHI;
    limit_dn = custom.TMPLO;
    alarm(2);
    lcd.clear();
  }
}

// void Resistor()
// {
//   int Vo = 0;
//   Vo = analogRead(A0);
//   x2 = 10000.0 * (1024.0 / (float)Vo - 1.0); /// NTC resistance connected between Vcc & ADC Pin
// lcd.setCursor(0, 1);
// lcd.print("A0= ");
// lcd.print(x2);
// lcd.print("  ");
//   Vo = analogRead(A1);
//   R2 = 10000.0 * (1024.0 / (float)Vo - 1.0); /// NTC resistance connected between Vcc & ADC Pin
//   lcd.setCursor(0, 0);
//   lcd.print("A1= ");
//   lcd.print(R2);
//   lcd.print("  ");
//   while ((red.get_key() == 0x5f) && !lc2 && (millis() - tick > 200)) ///left key=5f
//   {
//     tick = millis();
//     delay(1500);
//   }
// }

void setup()
{
  Serial.begin(9600);
  EEPROM.begin();
  //////////////////////////
  // eeAddress += sizeof(float);
  // EEPROM.put(eeAddress, factory_defult);
  /////////////////////////////////////////////
  eeAddress = sizeof(float);
  EEPROM.get(eeAddress, custom);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(on_Request);
  analogReference(EXTERNAL);
  pinMode(heater, OUTPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, 0);
  digitalWrite(heater, 0);
  lcd.begin(16, 2);
  red.clean();
  green.clean();
  red.allOn_bri(2, 1);
  green.allOn_bri(1, 1);
  //*************************
  // Reading (setpoint) from EEPROM
  // sp0 = eeprom.read(0x50, 0);
  // sp0 *= 100;
  // sp1 = eeprom.read(0x50, 1);
  // setpoint = (sp1) + (sp0);
  // setpoint /= 100;
  //*******************
  // Reading (Cycle time) fom turning motor from EEPROM
  // cycle_time = eeprom.read(0x50, 2);
  //*************************
  // Reading (turning_time) from EEPROM
  // sp0 = eeprom.read(0x50, 3);
  // sp0 *= 100;
  // sp1 = eeprom.read(0x50, 4);
  // turning_time = (sp1) + (sp0);
  // turning_time /= 100;
  //**************************************
  tick = millis();
  limit_up = custom.TMPHI;
  limit_dn = custom.TMPLO;
  tmp_up = stien_three(A1, 10000.0, 1.286986010E-03, 2.067415798E-04, 1.923761216E-07);     ///first one epoxy 1500 dinar ok
  tmp_dwn = stien_three(A0, 10000.0, 9.5569562991E-04, 2.4161535576E-04, 2.1035712840E-07); ///Black wire.Steel old (ok)
  area = custom.SETTMP - tmp_dwn;
  alarm(2);
  TCCR1A = 0;            ///Reset Timer1;
  TCCR1B |= (1 << CS12); ///Set the Prescalor(1,0,0)256
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS10);
  //////
  TCNT1 = 0;       //start value
  OCR1A = t1_comp; ///compare value
  /////
  TIMSK1 = (1 << OCIE1A); ///Set the timer 1 interrupt bit enable
  //////
  sei(); //////Enable the Gloable interrupt
  delay(300);
}

void loop()
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
  /////there is conflict with Rtc after 15 minuts becuse [i2c address of FD560 = i2c address of DS3231 ](on the same pins)
  // while ((red.get_key() == 0x7F) && !lc1) ///set key=7f on press,3f on relesed k1+k2(dual keys)
  // {
  //   tick = millis();
  //   lc1 = 1;
  //   sw = 1;
  //   lcd.clear();
  //   count = 1;
  //   alarm(1);
  //   manu1();
  //   manu2();
  //   manu3();
  //   summry();
  //   save_func();
  // }
  while (bitRead(red.get_key(), 6) && !t) ///Bit 6 of the return byte is the status bit detecting if key is pressed or relesed
  {
    Serial.print(String((red.get_key()), HEX) + "H | "); ///When key Pressed
    t = 1;
  }

  while (!bitRead(red.get_key(), 6) && t)
  {
    Serial.println(String((red.get_key()), HEX) + "H"); ///When key Relesed
    t = 0;
  }

  if ((red.get_key() == 0x47)) ///Preview screen
  {
    lcd.clear();
    preview();
    delay(4000);
    lcd.clear();
  }

  manual_turn();

  humidity();

  display();

  di();

  limit_warning();

  turning_tone();

  saving_data();
}

ISR(TIMER1_COMPA_vect)
{
  TCNT1 = 0;
  rtcRead();
  seconds++;
  if (seconds > 59)
  {
    seconds = 0;
    minuts++;
  }

  hum_trig++;
  auto_turn();
  update_data();
}
