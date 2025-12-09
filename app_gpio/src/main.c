#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define LED_NODE          DT_ALIAS(myled)
#define BTN_NODE          DT_ALIAS(mybtn)

#define TIME_MS           1000

LOG_MODULE_REGISTER(app_gpio, CONFIG_LOG_MAX_LEVEL);

static const struct gpio_dt_spec myled0 = GPIO_DT_SPEC_GET(LED_NODE, gpios);   
static const struct gpio_dt_spec mybtn0 = GPIO_DT_SPEC_GET(BTN_NODE, gpios); 

int main()
{
	int8_t ret = 0; 
	uint8_t btn_state = 0; 
	uint8_t prev_btn_state = 0;
	uint8_t mode = 0;

	// TODO: OUTPUT CONFIGURATION
	ret = gpio_pin_configure_dt(&myled0, GPIO_OUTPUT_INACTIVE);
	if(ret < 0) LOG_ERR("LED configure as output failed");

	// TODO: INPUT CONFIGURATION 
	ret = gpio_pin_configure_dt(&mybtn0, GPIO_INPUT|GPIO_PULL_UP);
	if(ret < 0) LOG_ERR("Button configure as input failed");

	while(1)
	{
		//TODO: Debounce algorithm
		btn_state = gpio_pin_get_dt(&mybtn0);
		k_msleep(20);
		if(btn_state != prev_btn_state) {
		    k_msleep(20);
			btn_state = gpio_pin_get_dt(&mybtn0);		

		    if(btn_state != prev_btn_state)
		   {
		    	mode++;
				LOG_INF("mode has changed to %d", mode);
		   }	
		   
		   switch(mode)
		   {
			case 0: 
				gpio_pin_set_dt(&myled0, 0);
		   		LOG_INF("MODE %d: LED is off", mode);
				break;
			case 1:
				gpio_pin_set_dt(&myled0,1);
		   		LOG_INF("MODE %d: LED is on", mode);
				break;

		   }
		 }
							
		if(mode > 1) 
		{
			mode = 0;
			LOG_INF("Mode has reset to %d", mode);
		}
		
	}			
	return 0;
}

