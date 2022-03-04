#ifndef UART_H_
#define UART_H_


typedef unsigned char Byte;


void uart_init();
int uart_read(Byte *buffer, unsigned int buffer_size);
int uart_write(Byte *buffer, unsigned int buffer_size);
void uart_close();


#endif /* UART_H_ */