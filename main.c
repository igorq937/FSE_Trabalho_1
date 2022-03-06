#include "inc/bme280_driver.h"
#include "inc/pid.h"
#include "inc/csv.h"
#include "inc/esp32_control.h"
#include "inc/lcd_16x2_driver.h"


#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdio.h>    
#include <softPwm.h>
#include <unistd.h>


#define MODE_TERMINAL       0x00
#define MODE_UART			0x01

#define UART_MODE_POTENTIOMETER 0x00
#define UART_MODE_CURVE         0x01

#define CSV_LINES_MAX       50 
#define CSV_START_LINE      1      


struct csv_log_data log_data;
float reference_temperature = 0;

int mode = 0;
int uart = 0;

const int RESISTOR_PIN =    4;  /* GPIO23 */
const int FAN_PIN =         5;	/* GPIO24 */

long curve_lines = 0;
float curve_temperature[CSV_LINES_MAX];
long curve_time[CSV_LINES_MAX];
long curve_line = CSV_START_LINE;
long curve_clock = 0;


void sig_handler(){
    printf("### DESLIGANDO! ###\n");

    softPwmWrite(RESISTOR_PIN, 0);
    softPwmWrite(FAN_PIN, 0);
    bme280_driver_close();

    lcd_clear();
    lcd_type_line("    SISTEMA");
    lcd_set_line(LCD_LINE2);
    lcd_type_line("   DESLIGADO!");

    send_on_off(ESP_SEND_OFF);
    exit(0);
}

void control_temperature(){

    double control_temperature = pid_controle(log_data.internal_temperature);
    if(control_temperature > 0){
        log_data.resistor_power = control_temperature;
        softPwmWrite(RESISTOR_PIN, control_temperature);
        log_data.fan_speed = 0;
        softPwmWrite(FAN_PIN, 0);
    }else if(control_temperature >= -40){
        log_data.fan_speed = 40;
        softPwmWrite(FAN_PIN, 40);
        log_data.resistor_power = 0;
        softPwmWrite(RESISTOR_PIN, 0);
    }else{
        log_data.fan_speed = -control_temperature;
        softPwmWrite(FAN_PIN, (int)-control_temperature);
        log_data.resistor_power = 0;
        softPwmWrite(RESISTOR_PIN, 0);
    }
    send_control_tempetature(control_temperature);

    printf("Temperatura de referencia: %2.1lf C'\n", reference_temperature);
    printf("Temperatura interna:       %2.1lf C'\n", log_data.internal_temperature);
    printf("Sinal de controle:         %2.1lf%%\n", control_temperature);

}

void lcd_routine(){
    lcd_clear();
    if(mode == MODE_TERMINAL)
        lcd_type_line("TERMINAL Tr:");
    else
        if(uart == UART_MODE_POTENTIOMETER)
            lcd_type_line("UARTPOT  Tr:");
        else
            lcd_type_line("UARTCURV Tr:");
    lcd_type_float(log_data.reference_temperature);

    lcd_set_line(LCD_LINE2);
    lcd_type_line("Ti:");
    lcd_type_float(log_data.internal_temperature);
    lcd_type_line("  Te:");
    lcd_type_float(log_data.external_temperature);
}

void user_routine(){
    Byte command;
    request_user_command(&command);

    switch(command){
    case ESP_USER_COMMAND_ON:
        printf("### PRESSIONADO LIGAR! ###\n");
        send_on_off(ESP_SEND_ON);
        break;
    case ESP_USER_COMMAND_OFF:
        printf("### PRESSIONADO DESLIGAR! ###\n");
        sig_handler();
        break;
    case ESP_USER_COMMAND_POTENTIOMETER:
        printf("### PRESSIONADO PONTECIOMETRO! ###\n");
        uart = UART_MODE_POTENTIOMETER;
        send_control_mode(ESP_SEND_MODE_POTENTIOMETER);
        break;
    case ESP_USER_COMMAND_CURVE:
        printf("### PRESSIONADO CURVA REFLOW! ###\n");
        uart = UART_MODE_CURVE;
        send_control_mode(ESP_SEND_MODE_CURVE);
        break;
    }
}

void control_temperature_routine(){
    
    if(mode == MODE_UART){
        if(uart == UART_MODE_POTENTIOMETER){
            request_potentiometer(&reference_temperature);
        }else{
            if(curve_clock == curve_time[curve_line])
                reference_temperature = curve_temperature[curve_line++];
            
            curve_clock++;
            if(curve_clock == curve_lines){
                uart = UART_MODE_POTENTIOMETER;
                send_control_mode(ESP_SEND_MODE_POTENTIOMETER);
                printf("### CURVE REFLOW TERMINOU! ###\n");
                printf("### MODO POTENCIOMETRO HABILITADO! ###\n");
            }
        }
    }

    log_data.reference_temperature = reference_temperature;
    pid_atualiza_referencia(reference_temperature);
	send_reference_tempetature(reference_temperature);

    bme280_get_temperature(&log_data.external_temperature);

    control_temperature();

    float internal_temperature;
    request_internal_temperature(&internal_temperature);
    log_data.internal_temperature = internal_temperature;
}

void loop_routine(){
    printf("\n### Executando rotina! ###\n");
    user_routine();
    control_temperature_routine();
    csv_append_log(log_data);
    lcd_routine();
}

long long timeInMicroseconds(void){

    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((((long long)tv.tv_sec)*1000000)+(tv.tv_usec));
}

void loop(long long microseconds_tick){

    long long wait = 0;
    long long currentTime = 0;
    long long lastTime = timeInMicroseconds();

    while(1){
        currentTime = timeInMicroseconds();
        wait += microseconds_tick - (currentTime - lastTime);
        
        loop_routine();

        if(wait > 0){
            usleep(wait);
		    wait = 0;
        }
        lastTime = timeInMicroseconds();
    }
}

void scan_mode(){

    char option;

    printf("\nSelecione o modo de temperatura de referencia:\n\n");
    do{
        printf("\n\t(1) Terminal.\n");
        printf("\t(2) UART.\n");
        printf("=======> ");
        scanf(" %c", &option);

        switch(option){
        case '1':
			printf("\nDigite a temperatura de referencia desejada:\n\n");
			printf("=======> ");
			scanf(" %f", &reference_temperature);
			mode = MODE_TERMINAL;
            break;

        case '2':
			mode = MODE_UART;
            break;

        default:
            printf("\nEntrada invalida! Selecione novamente:\n");
            option = '\0';
        }
    }while(option == '\0');
    send_control_mode(mode);
}

void scan_pid_consts(){

	double Kp, Ki, Kd;
	char option;

	printf("\nDeseja utilizar um predefinicao para configurar as constantes do PID?\n");
    do{
        printf("\n\t(1) Predefinicao rasp42 -> Kp=30.0 Ki=0.2 Kd=400.0.\n");
        printf("\t(2) Predefinicao rasp43 -> Kp=20.0 Ki=0.1 Kd=100.0.\n");
        printf("\t(3) Predefinicao customizada.\n\n");
        printf("=======> ");
        scanf(" %c", &option);

        switch(option){
        case '1':
            Kp = 30.0, Ki = 0.2, Kd = 400.0;
            break;

        case '2':
            Kp = 20.0, Ki = 0.1, Kd = 100.0;
            break;

        case '3':
            printf("\nInforme a cosntantes Kp Ki Kd para o PID:\n\n");
            printf("=======> ");
            scanf("%lf %lf %lf", &Kp, &Ki, &Kd);
            break;

        default:
            printf("\nEntrada invalida! Selecione novamente:\n");
            option = '\0';
        }
    }while(option == '\0');
    pid_configura_constantes(Kp, Ki, Kd);
}

int main(void){

    //setando estado inicial do sistema
    send_on_off(ESP_SEND_OFF);
    send_control_mode(ESP_SEND_MODE_POTENTIOMETER);

    printf("### INICIALIZANDO! ###\n");

    if(wiringPiSetup() == -1) exit(1);
    if(lcd_init() == -1) exit(1);
    if(bme280_driver_init() != BME280_OK) exit(1);  
    signal(SIGINT, sig_handler);
    
    pinMode(RESISTOR_PIN, OUTPUT);
	pinMode(FAN_PIN, OUTPUT);
	softPwmCreate(RESISTOR_PIN, 1, 100);
    softPwmCreate(FAN_PIN, 1, 100);

	scan_pid_consts();
	scan_mode();
    csv_create_log();

    if(mode == MODE_TERMINAL){
        send_control_mode(ESP_SEND_MODE_CURVE);
    }else{
        send_control_mode(ESP_SEND_MODE_POTENTIOMETER);
        curve_lines = csv_read_csv_curve(curve_temperature, curve_time);    
    }

    send_on_off(ESP_SEND_ON);
    printf("### SISTEMA LIGADO! ###\n");

    loop(1000000);

    return 0;
}
