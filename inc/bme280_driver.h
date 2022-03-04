#ifndef BME280_DRIVER_H_
#define BME280_DRIVER_H_


#include "bme280.h"


int32_t bme280_driver_init(void);
int8_t bme280_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
int8_t bme280_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
void bme280_delay_us(uint32_t period, void *intf_ptr);
int8_t bme280_get_temperature(double *temperature);
void bme280_driver_close(void);
char * bme280_str_error(int error);


#endif /* BME280_DRIVER_H_ */