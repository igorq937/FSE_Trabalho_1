#include "esp32_control.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MESSAGE_MAX_SIZE        128
#define ESP_ADDRESS_DEVICE      0x01
#define ESP_REQUEST             0x23
#define ESP_SEND                0x16

#define TEMPERATURE_ERROR       0.5f
#define TEMPERATURE_MIN         0.0f
#define TEMPERATURE_MAX         70.0f

#define RETURN_MESSAGE_SIZE     4
#define ESP_TIMEOUT             2


static Byte message_buffer[MESSAGE_MAX_SIZE];
static float last_internal_temperature = 0;
static float last_potentiometer_temperature = 0;


int request_esp(void *return_message, Byte sub_code){

    modbus_init(ESP_ADDRESS_DEVICE, ESP_REQUEST, sub_code);
    
    int size = 0;
    for(int i = 0; size <= 0 && i < ESP_TIMEOUT; i++){
        modbus_write(NULL, 0);
        size = modbus_read(&message_buffer[0], MESSAGE_MAX_SIZE);
        if(size <= 0) continue;
    }

    memcpy(return_message, &message_buffer[3], RETURN_MESSAGE_SIZE);
    modbus_close();

    return ESP_R_SUCCESS;
}

int send2esp(void *data, Byte data_size, void *return_message, Byte sub_code){

    modbus_init(ESP_ADDRESS_DEVICE, ESP_SEND, sub_code);
    int size = modbus_write(data, data_size);
    if(size == -1) return ESP_R_FAIL;

    if(return_message != NULL){
        size = modbus_read(&message_buffer[0], MESSAGE_MAX_SIZE);
        if(size == -1) return ESP_R_FAIL;

        memcpy(return_message, &message_buffer[3], RETURN_MESSAGE_SIZE);
    }
    
    modbus_close();
    return ESP_R_SUCCESS;
}

int request_internal_temperature(float *temperature){
    
    int error = request_esp(temperature, 0xC1);
    for(int i = 0; (*temperature <= TEMPERATURE_ERROR && *temperature >= -TEMPERATURE_ERROR) && i < ESP_TIMEOUT; i++){
        error = request_esp(temperature, 0xC1);
    }

    if(*temperature > TEMPERATURE_MAX || *temperature <= TEMPERATURE_MIN){
        *temperature = last_internal_temperature;
    }else{
        last_internal_temperature = *temperature;
    }

    return error;
}

int request_potentiometer(float *temperature){
    
    int error = request_esp(temperature, 0xC2);
    for(int i = 0; (*temperature <= TEMPERATURE_ERROR && *temperature >= -TEMPERATURE_ERROR) && i < ESP_TIMEOUT; i++){
        error = request_esp(temperature, 0xC2);
    }

    if(*temperature > TEMPERATURE_MAX || *temperature <= TEMPERATURE_MIN){
        *temperature = last_potentiometer_temperature;
    }else{
        last_potentiometer_temperature = *temperature;
    }

    return error;
}

int request_user_command(Byte *return_byte){
    Byte bytes[4];
    int error = request_esp(&bytes[0], 0xC3);
    *return_byte = bytes[0];
    return error;
}

int send_control_tempetature(int control){
    return send2esp(&control, sizeof(int), NULL, 0xD1);
}

int send_reference_tempetature(float temperature){
    return send2esp(&temperature, sizeof(float), NULL, 0xD2);
}

int send_on_off(Byte on_off){
    Byte bytes[4];
    send2esp(&on_off, sizeof(Byte), bytes, 0xD3);
    if(bytes[0] != on_off) return ESP_R_FAIL;
    return ESP_R_SUCCESS;
}

int send_control_mode(Byte mode){
    Byte bytes[4];
    send2esp(&mode, sizeof(Byte), bytes, 0xD4);
    if(bytes[0] != mode) return ESP_R_FAIL;
    return ESP_R_SUCCESS;
}
