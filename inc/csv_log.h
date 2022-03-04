#ifndef CSV_LOG_H_
#define CSV_LOG_H_


struct csv_log_data{
    double internal_temperature;
    double external_temperature;
    double reference_tempetature;
    double fan_speed;
    double resistor_power;
};


void csv_create_log(void);
void csv_add_log(struct csv_log_data log);


#endif /* CSV_LOG_H_ */