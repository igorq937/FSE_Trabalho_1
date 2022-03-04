#ifndef MODBUS_H_
#define MODBUS_H_


#include "uart.h"


#define FUNCTION_COD_SEND 0x16
#define FUNCTION_COD_RESQUEST 0x23
#define SUB_COD_SEND_INT 0xB1
#define SUB_COD_SEND_FLOAT 0xB2
#define SUB_COD_SEND_STRING 0xB3
#define SUB_COD_REQUEST_INT 0xA1
#define SUB_COD_REQUEST_FLOAT 0xA2
#define SUB_COD_REQUEST_STRING 0xA3


void modbus_init(Byte address, Byte func_cod, Byte sub_cod);
int modbus_read(Byte *message, Byte message_size);
int modbus_write(const Byte *message, Byte message_size);
void modbus_close();


#endif /* MODBUS_H_ */