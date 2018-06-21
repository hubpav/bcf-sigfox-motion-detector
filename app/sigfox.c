#include <sigfox.h>

uint8_t sigfox_temperature_to_uint8(float temperature)
{
    if (isnan(temperature))
    {
        return 0xff;
    }

    if (temperature < -28)
    {
        temperature = -28;
    }
    else if (temperature > 99)
    {
        temperature = 99;
    }

    return (temperature + 28) * 2;
}

uint8_t sigfox_voltage_to_uint8(float voltage)
{
    if (voltage < 2.f)
    {
        voltage = 2.f;
    }
    else if (voltage > 6.f)
    {
        voltage = 6.f;
    }

    return (voltage - 2.f) * 4;
}
