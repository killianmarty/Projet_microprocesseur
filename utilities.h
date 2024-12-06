#ifndef utilitiesdef
#define utilitiesdef

#include "stm32f0xx.h"

#define MASK(x) (1UL << (x))
#define MODIFY_FIELD(reg, field, value) ((reg) = ((reg) & ~(field##_Msk)) | (((uint32_t)(value) << field##_Pos) & field##_Msk))

#define ENGINE_IDLE_CCR 1200
#define ENGINE_RANGE_CCR 800
#define POTENTIOMETRE_VALUES 4096
#define CHAR_BUFF_SIZE 5

extern int running;

extern float center;
extern float Kp;
extern float Ki;
extern float Kd;

struct potentiometreValues {
	float Cp;
	float Kp;
	float Ki;
	float Kd;
};

void configClocks(void);
void configRunningButton(void);
void configEngine(void);
void configADC(void);
void configSensor(void);
void configPotentiometres(void);
float sensorToDistance(unsigned int x);
unsigned int getSensorValue(void);
struct potentiometreValues getPotentiometreValue(void);
int PID(float distance);
char * floatToString(float number, char *str);
void delay_ms(int ms);

#endif
