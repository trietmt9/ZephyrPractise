#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#define TRANSMIT_PERIOD_MS 500
/*
*@NOTE: CONFIG_LOG_MAX_LEVEL: will also include Debug level log 
*/
LOG_MODULE_REGISTER(app_helloworld, CONFIG_LOG_MAX_LEVEL);

/*
*@NOTE: CONFIG_LOG_DEFAULT_LEVEL: will not include DEbug level initialy 
*/
int main(void)
{
	printk("Hello World sent using printk()\n");
	LOG_INF("Helloworld sent using Log INFO level");
	LOG_DBG("Helloworld sent using Log DEBUG level");
	LOG_WRN("Helloworld sent using Log WARNING level");
	LOG_ERR("Helloworld sent using Log ERROR level");
	return 0;
}

