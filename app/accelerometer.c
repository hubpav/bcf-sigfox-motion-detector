#include <accelerometer.h>
#include <bc_lis2dh12.h>
#include <bc_data_stream.h>
#include <bc_dice.h>

#define _ACCELEROMETER_CHANGE 0.2f

BC_DATA_STREAM_FLOAT_BUFFER(_accelerometer_x_axis_buffer, ACCELEROMETER_NUMBER_OF_SAMPLES)
BC_DATA_STREAM_FLOAT_BUFFER(_accelerometer_y_axis_buffer, ACCELEROMETER_NUMBER_OF_SAMPLES)
BC_DATA_STREAM_FLOAT_BUFFER(_accelerometer_z_axis_buffer, ACCELEROMETER_NUMBER_OF_SAMPLES)

static struct
{
    bc_lis2dh12_t lis2dh12;

    bc_dice_t dice;

    struct
    {
        bc_data_stream_t x_axis;
        bc_data_stream_t y_axis;
        bc_data_stream_t z_axis;

    } data_stream;

} _accelerometer;

static void _accelerometer_lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param);

void accelerometer_init(bc_tick_t update_interval)
{
    memset(&_accelerometer, 0, sizeof(_accelerometer));

    bc_dice_init(&_accelerometer.dice, BC_DICE_FACE_UNKNOWN);

    bc_lis2dh12_init(&_accelerometer.lis2dh12, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_event_handler(&_accelerometer.lis2dh12, _accelerometer_lis2dh12_event_handler, NULL);
    bc_lis2dh12_set_update_interval(&_accelerometer.lis2dh12, update_interval);

    bc_data_stream_init(&_accelerometer.data_stream.x_axis, 1, &_accelerometer_x_axis_buffer);
    bc_data_stream_init(&_accelerometer.data_stream.y_axis, 1, &_accelerometer_y_axis_buffer);
    bc_data_stream_init(&_accelerometer.data_stream.z_axis, 1, &_accelerometer_z_axis_buffer);
}

uint8_t accelerometer_position_get(void)
{
    return bc_dice_get_face(&_accelerometer.dice);
}

static void _accelerometer_lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;
    float x_axis;
    float y_axis;
    float z_axis;

    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {
        bc_lis2dh12_result_g_t g;

        if (!bc_lis2dh12_get_result_g(self, &g))
        {
            return;
        }

        bc_data_stream_feed(&_accelerometer.data_stream.x_axis, &g.x_axis);
        bc_data_stream_feed(&_accelerometer.data_stream.y_axis, &g.y_axis);
        bc_data_stream_feed(&_accelerometer.data_stream.z_axis, &g.z_axis);

        bc_data_stream_get_median(&_accelerometer.data_stream.x_axis, &x_axis);
        bc_data_stream_get_median(&_accelerometer.data_stream.y_axis, &y_axis);
        bc_data_stream_get_median(&_accelerometer.data_stream.z_axis, &z_axis);

        bc_dice_feed_vectors(&_accelerometer.dice, x_axis, y_axis, z_axis);
    }
}
