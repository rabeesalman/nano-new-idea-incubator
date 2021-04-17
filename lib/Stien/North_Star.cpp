#include <Arduino.h>
#define thermistor_current_pin A1
#define thermistor_analog_read_pin A0
//***********
float north_star(byte pin,byte vcc,double Rdivider,double a,double b,double c,double d)
{
    digitalWrite(vcc, HIGH);
    delay(100);
    int vo = analogRead(pin);
    digitalWrite(vcc, LOW);

    if (vo >= 1023)
    {
        vo = 1023;
    }
    double unfiltered_resistance = Rdivider * (vo / (1023.0 - vo));

    double calibration_factor = -3.27487396E-07 * unfiltered_resistance + 8.25744292E-03;

    double filtered_resistance = unfiltered_resistance * (1 + calibration_factor);

    double log_R = log(filtered_resistance);

    float T = 1 / (a + b * log_R + c * pow(log_R, 2) + d * pow(log_R, 3));

    float T = T - 273.15;

    delay(900);

}
void setup()
{
    pinMode(thermistor_current_pin, OUTPUT);
    pinMode(thermistor_analog_read_pin, INPUT);
}

void loop()
{
    north_star(A0,6,10000.0,1.21500454194620E-03,2.05334949463842E-04, 3.19176316497180E-06,-2.93752010251114E-08);
    // digitalWrite(thermistor_current_pin, HIGH);
    // delay(100);
    // int raw_thermistor_reading = analogRead(thermistor_analog_read_pin);
    // digitalWrite(thermistor_current_pin, LOW);

    // if (raw_thermistor_reading >= 1023)
    // {
    //     raw_thermistor_reading = 1023;
    // }
    // double unfiltered_resistance = voltage_divider_resistor * (raw_thermistor_reading / (1023.0 - raw_thermistor_reading));

    // double calibration_factor = -3.27487396E-07 * unfiltered_resistance + 8.25744292E-03;

    // double filtered_resistance = unfiltered_resistance * (1 + calibration_factor);

    // double SH_A_constant = 1.21500454194620E-03;
    // double SH_B_constant = 2.05334949463842E-04;
    // double SH_C_constant = 3.19176316497180E-06;
    // double SH_D_constant = -2.93752010251114E-08;

    // double natural_log_of_resistance = log(filtered_resistance);

    // float thermistor_temperature_kelvin = 1 / (SH_A_constant + SH_B_constant * natural_log_of_resistance + SH_C_constant * pow(natural_log_of_resistance, 2) + SH_D_constant * pow(natural_log_of_resistance, 3));

    // float celcius_thermistor_temperature = thermistor_temperature_kelvin - 273.15;

    // delay(900);
}
