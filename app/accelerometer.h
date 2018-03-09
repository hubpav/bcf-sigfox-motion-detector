#ifndef _ACCELEROMETER_H
#define _ACCELEROMETER_H

#include <bc_tick.h>

#define ACCELEROMETER_NUMBER_OF_SAMPLES 8

void accelerometer_init(bc_tick_t update_interval);

uint8_t accelerometer_position_get(void);

#endif // _ACCELEROMETER_H
