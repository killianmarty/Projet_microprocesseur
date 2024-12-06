#ifndef INIT_COMM_H
#define INIT_COMM_H
#include <stdio.h>
#include "stm32f0xx.h"

int USART2_write (int ch);
int USART2_read(void);
int fgetc(FILE *f);
int fputc(int c, FILE *f);
void init_Comm(void);
	
#endif
