#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h> 
#define SW0_NODE               DT_ALIAS(sw0)
#define LED0_NODE              DT_ALIAS(led0)
static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static struct gpio_callback btn_callback_data;
volatile uint8_t mode = 0;
volatile uint8_t prev_mode = 0xFF;  // Track previous mode to detect changes

LOG_MODULE_REGISTER(app_exti, CONFIG_LOG_DEFAULT_LEVEL);

void btn_pressed(const struct device* dev, struct gpio_callback* cb, uint32_t pins)
{
    mode++;
    if(mode > 3){
        mode = 0;  // Reset to 0
    }
    LOG_DBG("Button pressed! Mode: %d", mode);  // Debug log in ISR
}


int main()
{
    LOG_INF("============= EXTERNAL INTERRUPTS EXAMPLE =============");

    int8_t ret; 

    /*
    TODO: Check is gpio available 
    */
    if(!gpio_is_ready_dt(&btn))
    {
        return -1;
    }

    /*
    TODO: configure Button as input 
    */
    ret = gpio_pin_configure_dt(&btn, GPIO_INPUT); 
    if(ret != 0)
    {
        LOG_ERR("ERROR %d: Configure port %s pin %d failed as input failed", ret, btn.port->name , btn.pin);
    }
    
    /*
    TODO: Configure interrupts for Button's pin
    */
    ret = gpio_pin_interrupt_configure(btn.port, btn.pin, GPIO_INT_EDGE_RISING);
    if(ret != 0)
    {
         LOG_ERR("ERROR %d: Configure interrupt for port %s pin %d failed", ret, btn.port->name , btn.pin);
    }
    
    /*
    TODO: Configure LED as output
    */
   ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
   if(ret != 0)
   {
       LOG_ERR("ERROR %d: Configure port %s pin %d failed as output failed", ret, led.port->name, led.pin);

   }

   /*
   TODO: Initialize Callback function
   */
   gpio_init_callback(&btn_callback_data, btn_pressed, BIT(btn.pin));
   /*
   TODO: Add Callback data
   */
   gpio_add_callback(btn.port, &btn_callback_data);

   LOG_INF("=================== Setup interrupt complete ===================");
   LOG_INF("=================== Press the button to change LED states ===================");
    while(1)
    {
        if(mode != prev_mode)
        {
            prev_mode = mode;
            LOG_INF("Mode changed to: %d", mode);

            switch(mode)
            {
                case 0:
                    gpio_pin_set_dt(&led, 0);
                    LOG_INF("LED OFF");
                    break;
                case 1:
                    gpio_pin_set_dt(&led, 1);
                    LOG_INF("LED ON");
                    break;
                case 2:
                    LOG_INF("LED BLINKING - 1Hz");
                    break;
                case 3:
                    LOG_INF("LED BLINKING - 2Hz");
                    break;
            }
        }

        // Handle blinking modes
        if(mode == 2)
        {
            gpio_pin_toggle_dt(&led);
            k_msleep(500);  // 1Hz (500ms on, 500ms off)
        }
        else if(mode == 3)
        {
            gpio_pin_toggle_dt(&led);
            k_msleep(250);  // 2Hz (250ms on, 250ms off)
        }
        else
        {
            k_msleep(100);  // Small delay to prevent busy-waiting
        }
    }
    return 0;
}