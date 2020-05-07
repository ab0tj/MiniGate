#ifndef INC_MCU_H
#define INC_MCU_H

void initSpi();
void initMcu();
void resetMcu();
void initPtt();
uint read_mcu_adc(int a);
void get_ptt_status(unsigned char p);
int readGpio(int num);

#endif