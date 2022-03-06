#ifndef ESP32_CONTROL_H_
#define ESP32_CONTROL_H_


#include "modbus.h"


#define ESP_SEND_ON                 0x01
#define ESP_SEND_OFF                0x00

#define ESP_SEND_MODE_POTENTIOMETER 0x00
#define ESP_SEND_MODE_CURVE         0x01

#define ESP_R_SUCCESS               0
#define ESP_R_FAIL                  -1

#define ESP_USER_COMMAND_ON             0x01
#define ESP_USER_COMMAND_OFF            0x02
#define ESP_USER_COMMAND_POTENTIOMETER  0x03
#define ESP_USER_COMMAND_CURVE          0x04


int request_internal_temperature(float *temperature);
int request_potentiometer(float *temperature);
int request_user_command(Byte *return_byte);
int send_control_tempetature(int control);
int send_reference_tempetature(float temperature);
int send_on_off(Byte on_off);
int send_control_mode(Byte mode);


#endif /* ESP32_CONTROL_H_ */