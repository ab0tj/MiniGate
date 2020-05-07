#ifndef INC_MCU_H
#define INC_MCU_H

void initSpi();
void initMcu();
void resetMcu();
void initPtt();
float read_adc_mcu(int a, int scale);
void get_ptt_status(unsigned char p);
int readGpio(int num);

#endif