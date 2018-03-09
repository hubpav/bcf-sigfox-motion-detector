#ifndef _SIGFOX_H
#define _SIGFOX_H

#include <bc_common.h>

enum {
    SIGFOX_HEADER_BOOT = 0x00,
    SIGFOX_HEADER_BEACON = 0x01,
    SIGFOX_HEADER_ALERT_MOTION = 0x02,
    SIGFOX_HEADER_ALERT_TEMPERATURE = 0x03,
};

uint8_t sigfox_temperature_to_uint8(float temperature);

uint8_t sigfox_voltage_to_uint8(float voltage);

#endif // _SIGFOX_H
