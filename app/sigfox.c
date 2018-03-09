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
    else if (temperature > 100)
    {
        temperature = 100;
    }

    return (temperature + 28) * 2;
}

uint8_t sigfox_voltage_to_uint8(float voltage)
{
        if (voltage < 1.8)
    {
        voltage = 1.8;
    }
    else if (voltage > 3.8)
    {
        voltage = 3.8;
    }

    return (voltage - 1.8f) * 8;
}
