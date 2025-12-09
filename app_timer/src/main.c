#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_timer, CONFIG_LOG_MAX_LEVEL);
#define LED_NODE                DT_ALIAS(led0)


struct k_timer my_timer;
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);


void led_blink_handler(struct k_timer *timer)
{
    if( timer == &my_timer)
    {
        gpio_pin_toggle_dt(&led);
        LOG_INF(" GPIO toggled");
    }
}
int main()
{   

    int8_t ret = 0; 

    /** 
    TODO: Configure LED pin
    */
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
    if(ret != 0)
    {
        LOG_ERR("Configure Port %s pin %d as output failed", led.port->name, led.pin);
    }
    
    /** 
    TODO: Initialize timer 
    */
    k_timer_init(&my_timer, led_blink_handler, NULL);

    /**
    TODO: Create Timer duration 
    * @param k_timer: pointer to timer instance
    * @param duration: wait a time duration before callback
    * @param period: callback every duration
    */
    k_timer_start(&my_timer, K_NO_WAIT, K_MSEC(1000));

    /**    
    TODO: Ensure the timer has expired
    */
    k_timer_status_sync(&my_timer);
    return 0; 
}