#include <application.h>

#define PIR_COOLDOWN_RESET_SECONDS 30
#define PIR_COOLDOWN_SECONDS (15 * 60)

typedef enum
{
    SIGFOX_HEADER_RESET = 0,
    SIGFOX_HEADER_MOTION = 1

} sigfox_header_t;

bc_led_t led;
bc_module_sigfox_t sigfox_module;
bc_module_pir_t pir_module;

bool motion_transmission_active = false;

void transmit_reset_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    bc_scheduler_unregister(bc_scheduler_get_current_task_id());

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

    bc_scheduler_unregister(bc_scheduler_get_current_task_id());

    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint8_t buffer[1] = { SIGFOX_HEADER_MOTION };

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void pir_module_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        static bc_tick_t tick_cooldown = PIR_COOLDOWN_RESET_SECONDS * 1000;

        bc_tick_t tick_now = bc_tick_get();

        if (tick_cooldown < tick_now && !motion_transmission_active)
        {
            tick_cooldown = tick_now + PIR_COOLDOWN_SECONDS * 1000;

            bc_scheduler_register(transmit_motion_task, NULL, 0);

            motion_transmission_active = true;
        }
        else
        {
            if (!bc_led_is_pulse(&led))
            {
                bc_led_pulse(&led, 10);
            }
        }
    }
    else if (event == BC_MODULE_PIR_EVENT_ERROR)
    {
        bc_led_pulse(&led, 0);

        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_SIGFOX_EVENT_ERROR)
    {
        bc_led_pulse(&led, 0);

        bc_led_set_mode(&led, BC_LED_MODE_BLINK);
    }
    else if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        motion_transmission_active = false;
    }
}

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_module_pir_init(&pir_module);
    bc_module_pir_set_event_handler(&pir_module, pir_module_event_handler, NULL);
    bc_module_pir_set_sensitivity(&pir_module, BC_MODULE_PIR_SENSITIVITY_MEDIUM);

    bc_module_sigfox_init(&sigfox_module, BC_MODULE_SIGFOX_REVISION_R2);
    bc_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);

    bc_scheduler_register(transmit_reset_task, NULL, 0);

    bc_led_pulse(&led, PIR_COOLDOWN_RESET_SECONDS * 1000);
}
