#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define LED_NODE                      DT_ALIAS(led0)
/* Create Log thread Macro */
#define LOG_THREAD_STACK_SIZE         254
#define LOG_THREAD_PRIORITY           1
/* Create LED thread Macro */
#define LED_THREAD_STACK_SIZE         254
#define LED_THREAD_PRIORITY           2

LOG_MODULE_REGISTER(app_thread, CONFIG_LOG_DEFAULT_LEVEL);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

K_THREAD_STACK_DEFINE(led_thread_stack, LED_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(log_thread_stack, LOG_THREAD_STACK_SIZE);

struct k_thread led_thread_data; 
struct k_thread log_thread_data; 


/**
 * TODO: Create a Blinking Thread
 */
void led_blink_thread(void *arg1, void *arg2, void* arg3)
{

    while(1)
    {
        gpio_pin_toggle_dt(&led);
        k_msleep(500);
    }
    
}


/**
 * TODO: Create a print Log Thread
 */
void log_print_thread(void *arg1, void *arg2, void *arg3)
{
  

    while(1)
    {
        LOG_INF("Message from log thread");
        k_msleep(500);
    }
}

int main(void)
{

    int8_t ret = 0;

    /*
     * Configure the LED pin as output and check if it's succeed or not  
     */
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
    if(ret != 0)
    {
        LOG_ERR("Config port %s pin %d as output failed", led.port->name, led.pin);
    }

    /*
    * Create LED thread 
    */
    k_tid_t led_thread_id = k_thread_create(&led_thread_data, 
                                            led_thread_stack,
                                            LED_THREAD_STACK_SIZE, 
                                            led_blink_thread, 
                                            NULL, NULL, NULL, 
                                            LED_THREAD_PRIORITY, 
                                            0, 
                                            K_NO_WAIT);
    
    /*
    * Create LOG thread 
    */
    k_tid_t log_thread_id = k_thread_create(&log_thread_data, 
                                            log_thread_stack,
                                            LOG_THREAD_STACK_SIZE, 
                                            log_print_thread,
                                            NULL, NULL, NULL,
                                            LOG_THREAD_PRIORITY, 
                                            0,
                                            K_NO_WAIT);
    // Run forever
    while(1)
    {
        k_msleep(100);
    }
    return 0; 
}