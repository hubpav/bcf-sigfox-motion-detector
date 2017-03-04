#include <application.h>

#define PIR_COOLDOWN_RESET 30000
#define PIR_COOLDOWN (15 * 60 * 1000)

typedef enum
{
    SIGFOX_HEADER_RESET = 0,
    SIGFOX_HEADER_MOTION = 1,
    SIGFOX_HEADER_BUTTON = 2,
    SIGFOX_HEADER_BATTERY = 3

} sigfox_header_t;

bc_tick_t tick_cooldown = PIR_COOLDOWN_RESET;

bc_scheduler_task_id_t led_cooldown_task_id;
bc_scheduler_task_id_t transmit_reset_task_id;
bc_scheduler_task_id_t transmit_motion_task_id;
bc_scheduler_task_id_t transmit_button_task_id;

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Temperature Tag instance
bc_tag_temperature_t temperature_tag;

// SigFox Module instance
bc_module_sigfox_t sigfox_module;

// PIR Module instance
bc_module_pir_t pir_module;

void led_cooldown_task(void *param)
{
    (void) param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);
}

void transmit_reset_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint8_t buffer[1] = { SIGFOX_HEADER_RESET };

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void transmit_motion_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint8_t buffer[1] = { SIGFOX_HEADER_MOTION };

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void transmit_button_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint8_t buffer[1] = { SIGFOX_HEADER_BUTTON };

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_CLICK)
    {
        bc_scheduler_plan_now(transmit_button_task_id);
    }

    if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_module_sigfox_continuous_wave(&sigfox_module);
    }
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (bc_module_sigfox_is_ready(&sigfox_module))
        {
            float temperature;

            // Read temperature
            bc_tag_temperature_get_temperature_celsius(self, &temperature);
        }
    }
}

void pir_module_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is motion...
    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        bc_tick_t tick_now = bc_tick_get();

        if (tick_cooldown < tick_now)
        {
            tick_cooldown = tick_now + PIR_COOLDOWN;

            bc_scheduler_plan_now(transmit_motion_task_id);
        }
        else
        {
            // Generate 10 millisecond LED pulse
            if (!bc_led_is_pulse(&led))
            {
                bc_led_pulse(&led, 10);
            }
        }
    }
    // If event is error...
    else if (event == BC_MODULE_PIR_EVENT_ERROR)
    {
        // Indicate sensor error by LED blinking
        bc_led_pulse(&led, 0);
        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    static bool has_device_id = false;
    static bool has_device_pac = false;

    // If event is ready...
    if (event == BC_MODULE_SIGFOX_EVENT_READY)
    {
        if (!has_device_id)
        {
            bc_module_sigfox_read_device_id(self);
        }
        else if (!has_device_pac)
        {
            bc_module_sigfox_read_device_pac(self);
        }
    }
    // If event is error...
    else if (event == BC_MODULE_SIGFOX_EVENT_ERROR)
    {
        bc_led_pulse(&led, 0);
        bc_led_set_mode(&led, BC_LED_MODE_BLINK);

        bc_scheduler_plan_now(led_cooldown_task_id);
    }
    // If event is end of transmission...
    else if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
    // If event is read device ID...
    else if (event == BC_MODULE_SIGFOX_EVENT_READ_DEVICE_ID)
    {
        has_device_id = true;
    }
    // If event is read device PAC...
    else if (event == BC_MODULE_SIGFOX_EVENT_READ_DEVICE_PAC)
    {
        has_device_pac = true;
    }
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize Temperature Tag
    bc_tag_temperature_init(&temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&temperature_tag, 10000);
    bc_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    // Initialize PIR Module
    bc_module_pir_init(&pir_module);
    bc_module_pir_set_event_handler(&pir_module, pir_module_event_handler, NULL);
    bc_module_pir_set_sensitivity(&pir_module, BC_MODULE_PIR_SENSITIVITY_MEDIUM);

    // Initialize SigFox Module
    bc_module_sigfox_init(&sigfox_module, BC_MODULE_SIGFOX_REVISION_R2);
    bc_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);

    led_cooldown_task_id = bc_scheduler_register(led_cooldown_task, NULL, BC_TICK_INFINITY);
    transmit_reset_task_id = bc_scheduler_register(transmit_reset_task, NULL, 0);
    transmit_motion_task_id = bc_scheduler_register(transmit_motion_task, NULL, BC_TICK_INFINITY);
    transmit_button_task_id = bc_scheduler_register(transmit_button_task, NULL, BC_TICK_INFINITY);

    // Generate 30 second LED pulse
    bc_led_pulse(&led, tick_cooldown);
}
