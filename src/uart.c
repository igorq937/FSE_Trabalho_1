#include "uart.h"


#include <unistd.h>
#include <fcntl.h>
#include <termios.h>


#define FILE_DESCRIPTOR "/dev/serial0"
#define UART_R_FAIL		-1


static int file_descriptor = -1;


void uart_init(){

	file_descriptor = open(FILE_DESCRIPTOR, O_RDWR | O_NOCTTY | O_NDELAY);

	if (file_descriptor == -1) return;

	struct termios options;
	tcgetattr(file_descriptor, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(file_descriptor, TCIFLUSH);
	tcsetattr(file_descriptor, TCSANOW, &options);
}

int uart_read(Byte *buffer, unsigned int buffer_size){

	if(file_descriptor != -1){
		int size = read(file_descriptor, (void*)buffer, buffer_size);
		return size;
	}
	return UART_R_FAIL;
}

int uart_write(Byte *buffer, unsigned int buffer_size){

	if(file_descriptor != -1){
		int size = write(file_descriptor, buffer, buffer_size);
		return size;
	}
	return UART_R_FAIL;
}

void uart_close(){
	close(file_descriptor);
}