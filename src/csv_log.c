#include "csv_log.h"
#include <stdio.h>
#include <time.h>


#define FILE_PATH "log.csv"


void csv_create_log(void){

    FILE *file = fopen(FILE_PATH, "rb");

    if(file == NULL){
        FILE *file = fopen(FILE_PATH, "wb");
        fprintf(file, "%s;%s;", "date", "time");
        fprintf(file, "%s;%s;%s;", "internal temperature (deg C)", "external temperature (deg C)", "reference tempetature (deg C)");
        fprintf(file, "%s;%s\n", "fan speed (%)", "resistor power (%)");
        fclose(file);
    }else{
        fclose(file);
    }
}

void csv_add_log(struct csv_log_data log){
    
    FILE *file = fopen(FILE_PATH, "a");

    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    fprintf(file, "%02d/%02d/%04d;", info->tm_mday, info->tm_mon, info->tm_year%100 + 2000);
    fprintf(file, "%02d:%02d:%02d;", info->tm_hour, info->tm_min, info->tm_sec);
    fprintf(file, "%02.2lf;%02.2lf;%02.2lf;", log.internal_temperature, log.external_temperature, log.reference_tempetature);
    fprintf(file, "%02.2lf;%02.2lf\n", log.fan_speed, log.resistor_power);

    fclose(file);
}