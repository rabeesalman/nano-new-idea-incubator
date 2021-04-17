
#include <Arduino.h>
#include <Stien.h>

float Stien::get_temp()
{
  float sum = 0.0;

  for (int i = 0; i < 7; i++) //200
  {
    digitalWrite(_vcc_pin, HIGH);

    delay(5);

    Vo = analogRead(_Therm_Pin);

    digitalWrite(_vcc_pin, LOW);

    delay(5);

    R2 = _R1 * (1024.0 / (float)Vo - 1.0); /// NTC resistance

    logR2 = log(R2);

    T = (1.0 / (_c1 + _c2 * logR2 + _c3 * logR2 * logR2 * logR2));
    T = (T - 273.15);

    sum = T + sum;

  }

  T = sum / 7.0;

  return T;

}
