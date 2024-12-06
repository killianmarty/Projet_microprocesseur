#include "stm32f0xx.h"
#include "init_Comm.h"
#include "utilities.h"

int running = 1;

//Valeurs par défaut
float center = 15.7f;
float Kp = 8.8f;
float Ki = .05f;
float Kd = 23.5f;

void delay_ms(int ms){
	for(volatile int i = 0; i < ms * 8000; i++);
}

void configClocks(){
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
}

void configRunningButton(){
	//Interupteur foncionnement/configuration sur PC10, 5V
	MODIFY_FIELD(GPIOC->MODER,GPIO_MODER_MODER10,0);
	MODIFY_FIELD(GPIOC->PUPDR,GPIO_PUPDR_PUPDR10,1);
}


void configEngine(){
	//Le moteur est branché sur la pin PC7, 5V
	MODIFY_FIELD(GPIOC->MODER,GPIO_MODER_MODER7, 2); 
	MODIFY_FIELD(GPIOC->AFR[0], GPIO_AFRL_AFSEL7, 0);
	

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 9;
	TIM3->ARR = 15999;  //50Hz
	TIM3->CCR2 = ENGINE_IDLE_CCR;
	TIM3->CNT = 0;
	
	//Active le mode output compare sur le canal 2.
	MODIFY_FIELD(TIM3->CCMR1, TIM_CCMR1_CC2S, 0);
	//Met le mode PWM à 1 sur le canal 2.		
	MODIFY_FIELD(TIM3->CCMR1, TIM_CCMR1_OC2M, 6);
	//Active l'ouput compare sur le canal 2.
	TIM3->CCER |= TIM_CCER_CC2E;
	//Active TIM3				
	TIM3->CR1 |= TIM_CR1_CEN; 							
}

void configADC(){
		MODIFY_FIELD(ADC1->SMPR, ADC_SMPR_SMP, 0);
    if ((ADC1->CR & ADC_CR_ADEN) != 0) {
        ADC1->CR &= ~ADC_CR_ADEN;
    }
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
		MODIFY_FIELD(ADC1->CFGR1, ADC_CFGR1_RES, 0);
}


void configSensor(){
		//Le capteur est branché sur la pin PB0, 3.3V
		MODIFY_FIELD(GPIOB->MODER, GPIO_MODER_MODER0, 3);
}

void configPotentiometres(){		
		//KD sur la pin PC1, 5V
		MODIFY_FIELD(GPIOC->MODER, GPIO_MODER_MODER1, 3);
		
		//KP sur la pin PB1, 5V
		MODIFY_FIELD(GPIOB->MODER, GPIO_MODER_MODER1, 3);
		
		//KI sur la pin PC5, 5V
		MODIFY_FIELD(GPIOC->MODER, GPIO_MODER_MODER5, 3);
	
		//C sur le pin PC2, 5V
		MODIFY_FIELD(GPIOC->MODER, GPIO_MODER_MODER2, 3);
}

float sensorToDistance(unsigned int x){
		long double v = (long double)x;
	
		//On applique une fonction interpolée à partir de mesures pour convertir les données du capteur distance
		float res = (float)((0.0037 * v*v - 25.439 * v + 44483)/1000);
		if(res < 0){
			return 0;
		}
		else if(res > 25){
			return 25;
		}
		else{
			return res;
		}
}

unsigned int getSensorValue(){
		// Selectionne l'entree du capteur
		ADC1->CHSELR |= MASK(8);
	
		// Demarre la conversion			
		ADC1->CR |= ADC_CR_ADSTART;			
		while(!(ADC1->ISR & ADC_ISR_ADRDY));
		unsigned int res = ADC1->DR;
	
		// Deselectionne l'entree du capteur
		ADC1->CHSELR &= ~MASK(8);
	
		return res;
}

struct potentiometreValues getPotentiometreValue(){
		struct potentiometreValues res;
	
		//Lecture de Cp
		ADC1->CHSELR |= MASK(12);
		ADC1->CR |= ADC_CR_ADSTART;
		while(!(ADC1->ISR & ADC_ISR_ADRDY));
		ADC1->CHSELR &= ~MASK(12);
		res.Cp = ((float)ADC1->DR) * 30 /POTENTIOMETRE_VALUES;
	
		//Lecture de Kp
		ADC1->CHSELR |= MASK(11);
		ADC1->CR |= ADC_CR_ADSTART;
		while(!(ADC1->ISR & ADC_ISR_ADRDY));
		ADC1->CHSELR &= ~MASK(11);
		res.Kp = ((float)ADC1->DR) * 50 /POTENTIOMETRE_VALUES;
	
		//Lectrure de Ki
		ADC1->CHSELR |= MASK(9);
		ADC1->CR |= ADC_CR_ADSTART;
		while(!(ADC1->ISR & ADC_ISR_ADRDY));
		ADC1->CHSELR &= ~MASK(9);
		res.Ki = ((float)ADC1->DR) * (float)0.5 /POTENTIOMETRE_VALUES;
	
		//Lecture de Kd
		ADC1->CHSELR |= MASK(15);
		ADC1->CR |= ADC_CR_ADSTART;
		while(!(ADC1->ISR & ADC_ISR_ADRDY));
		ADC1->CHSELR &= ~MASK(15);
		res.Kd = ((float)ADC1->DR) * 50 /POTENTIOMETRE_VALUES;
	
		return res;
}

int PID(float distance){
		static float E = 0;
		static float lastE = 0;
	
		float e = center - distance;
		float de = (e - lastE);
		E += e;
	
		float u = Kp * e + Ki * E + Kd * de;

		lastE = e;
		return (int)u;
}

//Fonction pour convertir un float en string sans la librairie stdio.h
char * floatToString(float x, char *p) {
    char *s = p + CHAR_BUFF_SIZE;
    int decimals;
    int units;
    if (x < 0) {
        decimals = (int)(x * -100) % 100;
        units = (int)(-1 * x);
    } else {
        decimals = (int)(x * 100) % 100;
        units = (int)x;
    }
    *--s = (char)(decimals % 10) + '0';
    decimals /= 10;
    *--s = (char)(decimals % 10) + '0';
    *--s = '.';
    while (units > 0) {
        *--s = (char)(units % 10) + '0';
        units /= 10;
    }
    if (x < 0) *--s = '-';
    return s;
}
