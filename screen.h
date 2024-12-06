#ifndef screendef
#define screendef

#include "stm32f0xx.h"

#define LCD_CLEAR_CMD 0x01

void SPI1_init(void);
void LCD_set_contrast(void);
void SPI1_write(uint8_t data);
void SPI1_write_string(const char* string);
void LCD_init(float cp, float p, float i, float d);
void LCD_update(float cp, float p, float i, float d);
void LCD_write_command(uint8_t command);
void LCD_write_data(uint8_t data);
void LCD_clear(void);
void LCD_printFloat(float x);

#endif
