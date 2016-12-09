/*gpio*/
#include "drivers/gpio.h"
int platform_gpio_mode(unsigned int pin, unsigned int mode)
{
	int err;
	if(err=gpio_request(pin))return err; 
	gpio_set_direction(pin, mode);
	gpio_release(pin);
	return 0;
}
int platform_gpio_read(unsigned int pin)
{
	int err,status;
	if(err=gpio_request(pin))return err;
	status=gpio_get_value( pin);
	gpio_release(pin);
	return status;
}
int platform_gpio_write( unsigned int pin, int level)
{
	int err,status;
	if(err=gpio_request(pin))return err;
	gpio_set_value(pin, level);
	gpio_release(pin);
	return 0;
}


