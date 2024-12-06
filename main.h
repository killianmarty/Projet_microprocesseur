#ifndef maindef
#define maindef

#include "stm32f0xx.h"

#define MASK(x) (1UL << (x))
#define MODIFY_FIELD(reg, field, value) ((reg) = ((reg) & ~(field##_Msk)) | (((uint32_t)(value) << field##_Pos) & field##_Msk))

#define MIN_DISTANCE_VALUE 2900
#define MAX_DISTANCE_VALUE 800
#define CENTER_DISTANCE_VALUE 1350

#define ENGINE_IDLE_CCR 1200
#define ENGINE_RANGE_CCR 800

#define POTENTIOMETRE_VALUES 4096

// Commandes spécifiques au SerLCD
#define LCD_CLEAR_CMD 0x01
#define LCD_SET_CURSOR_CMD 0x80




struct potentiometreValues {
	float Cp;
	float Kp;
	float Ki;
	float Kd;
};

void wait(int ms);
void configClocks(void);
void configRunningButton(void);
void configEngine(void);
void configSensor(void);
void configPotentiometres(void);
float sensorToDistance(unsigned int x);
unsigned int getSensorValue(void);
struct potentiometreValues getPotentiometreValue(void);
unsigned int PID(float distance);
void EXTI4_15_IRQHandler(void);


//Screen functions
void SPI1_init(void);
void SPI1_write(uint8_t data);
void SPI1_write_string(const char* string);
void LCD_init(float p, float i, float d);
void LCD_write_command(uint8_t command);
void LCD_write_data(uint8_t data);
void LCD_clear(void);
void delay_ms(uint32_t ms);

#endif
