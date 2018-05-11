#include <application.h>
#include <accelerometer.h>
#include <sigfox.h>

#define THERMOMETER_UPDATE_INTERVAL   (30 * 1000)
#define ACCELEROMETER_UPDATE_INTERVAL (30 * 1000)
#define BEACON_INTERVAL      (4 * 60 * 60 * 1000)
#define ALERT_INTERVAL           (20 * 60 * 1000)
#define BOOT_INTERVAL                  (2 * 1000)
#define TEMPERATURE_ALERT_DIFF              (5.f)
#define TEMPERATURE_ALERT_SAMPLES             (2)

bc_led_t led;
bc_tmp112_t thermometer;
bc_module_sigfox_t sigfox_module;
bc_module_pir_t pir;
bc_data_stream_t data_stream_temperature;
bc_data_stream_t data_stream_voltage;
BC_DATA_STREAM_FLOAT_BUFFER(data_stream_temperature_buffer, 8)
BC_DATA_STREAM_FLOAT_BUFFER(data_stream_voltage_buffer, 8)
bc_scheduler_task_id_t battery_measure_task_id;
bc_scheduler_task_id_t transmit_beacon_task_id;
bc_scheduler_task_id_t transmit_alert_motion_task_id;
bc_scheduler_task_id_t transmit_alert_temperature_task_id;
bool alert = false;
bool boot_send = false;
uint16_t count = 0;
uint16_t sent_count = 0;
float temperature_alert = NAN;
float temperature_previous = NAN;

void tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_TMP112_EVENT_UPDATE)
    {
        float temperature;

        if (!bc_tmp112_get_temperature_celsius(self, &temperature))
        {
            bc_data_stream_reset(&data_stream_temperature);

            return;
        }

        float temperature_nth;

        for (int i = -1; i >= -TEMPERATURE_ALERT_SAMPLES; i--)
        {
            if (bc_data_stream_get_nth(&data_stream_temperature, i, &temperature_nth))
            {
                if (fabsf(temperature_nth - temperature) > TEMPERATURE_ALERT_DIFF)
                {
                    if (isnan(temperature_alert))
                    {
                        bc_scheduler_plan_now(transmit_alert_temperature_task_id);
                    }

                    temperature_alert = temperature;
                    temperature_previous = temperature_nth;
                }
            }
        }

        bc_data_stream_feed(&data_stream_temperature, &temperature);
    }
}

void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    float voltage;

    if (!bc_module_battery_get_voltage(&voltage))
    {
        voltage = NAN;
    }

    bc_data_stream_feed(&data_stream_voltage, &voltage);
}

void battery_measure_task(void *param)
{
    (void) param;

    if (!bc_module_battery_measure())
    {
        bc_scheduler_plan_current_now();
    }
}

void pir_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        if (!boot_send)
        {
            return;
        }

        bc_led_pulse(&led, 50);

        count++;

        if (!alert)
        {
            alert = true;

            bc_scheduler_plan_now(transmit_alert_motion_task_id);
        }

        if (count == UINT16_MAX)
        {
            bc_scheduler_plan_now(transmit_alert_motion_task_id);
        }
    }

}

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START)
    {
        bc_led_set_mode(&led, BC_LED_MODE_ON);

        bc_scheduler_plan_relative(battery_measure_task_id, 20);
    }
    else if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        bc_scheduler_plan_relative(transmit_beacon_task_id, BEACON_INTERVAL);

        boot_send = true;
    }
    else if (event == BC_MODULE_SIGFOX_EVENT_ERROR)
    {
        bc_led_set_mode(&led, BC_LED_MODE_BLINK);
    }
}

void sigfox_fill_header(uint8_t state, uint8_t *buffer)
{
    float voltage = NAN;
    float temperature = NAN;

    bc_data_stream_get_average(&data_stream_voltage, &voltage);
    bc_data_stream_get_average(&data_stream_temperature, &temperature);

    buffer[0] = state << 4;
    buffer[0] |= accelerometer_position_get();

    buffer[1] = sigfox_voltage_to_uint8(voltage) << 4;

    buffer[2] = sigfox_temperature_to_uint8(temperature);
}

void transmit_boot_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    bc_scheduler_unregister(bc_scheduler_get_current_task_id());

    uint8_t buffer[3];

    sigfox_fill_header(SIGFOX_HEADER_BOOT, buffer);

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void transmit_beacon_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    uint8_t buffer[3];

    sigfox_fill_header(SIGFOX_HEADER_BEACON, buffer);

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void transmit_alert_motion_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    uint8_t buffer[5];

    sigfox_fill_header(SIGFOX_HEADER_ALERT_MOTION, buffer);

    buffer[3] = count << 8;
    buffer[4] = count;

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));

    if (count == sent_count)
    {
        alert = false;

        bc_scheduler_plan_current_absolute(BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_current_relative(ALERT_INTERVAL);
    }

    sent_count = count;
}

void transmit_alert_temperature_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    uint8_t buffer[5];

    sigfox_fill_header(SIGFOX_HEADER_ALERT_TEMPERATURE, buffer);

    buffer[3] = sigfox_temperature_to_uint8(temperature_alert);
    buffer[4] = sigfox_temperature_to_uint8(temperature_previous);

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));

    if (isnan(temperature_alert))
    {
        bc_scheduler_plan_current_absolute(BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_current_relative(ALERT_INTERVAL);
    }

    temperature_alert = NAN;
    temperature_previous = NAN;
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    while (BOOT_INTERVAL > bc_tick_get())
    {
        continue;
    }

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    bc_data_stream_init(&data_stream_temperature, 1, &data_stream_temperature_buffer);
    bc_data_stream_init(&data_stream_voltage, 1, &data_stream_voltage_buffer);

    // Initialize accelerometer sensor
    accelerometer_init(ACCELEROMETER_UPDATE_INTERVAL);

    // Initialize thermometer sensor
    bc_tmp112_init(&thermometer, BC_I2C_I2C0, 0x49);
    bc_tmp112_set_event_handler(&thermometer, tmp112_event_handler, NULL);
    bc_tmp112_set_update_interval(&thermometer, THERMOMETER_UPDATE_INTERVAL);

    // Initialize  battery
    bc_module_battery_init(BC_MODULE_BATTERY_FORMAT_MINI);
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    battery_measure_task_id = bc_scheduler_register(battery_measure_task, NULL, 2020);

    // Initialize sigfox modems
    bc_module_sigfox_init(&sigfox_module, BC_MODULE_SIGFOX_REVISION_R2);
    bc_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);

    // Initialize pir
    bc_module_pir_init(&pir);
    bc_module_pir_set_event_handler(&pir, pir_event_handler, NULL);

    transmit_alert_motion_task_id = bc_scheduler_register(transmit_alert_motion_task, NULL, BC_TICK_INFINITY);
    transmit_alert_temperature_task_id = bc_scheduler_register(transmit_alert_temperature_task, NULL, BC_TICK_INFINITY);
    transmit_beacon_task_id = bc_scheduler_register(transmit_beacon_task, NULL, BEACON_INTERVAL);

    bc_scheduler_register(transmit_boot_task, NULL, BOOT_INTERVAL + 1000);
}
