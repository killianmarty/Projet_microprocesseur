
#include "utilities.h"
#include "screen.h"

int main(void) 
{
	configClocks();
	configRunningButton();
	configEngine();
	configADC();
	configSensor();
	configPotentiometres();
	
	SPI1_init();
	delay_ms(100);
	LCD_init(center, Kp, Ki, Kd);
	
	TIM3->CCR2 = ENGINE_IDLE_CCR;
	
	while(1) 
	{
		running = GPIOC->IDR & MASK(10);
		if(running){
			//MODE FONCTIONNEMENT
			unsigned int ADC_value = getSensorValue();
			float distance = sensorToDistance(ADC_value);

			int pid = PID(distance);

			unsigned int newCCR2 = (unsigned int)(ENGINE_IDLE_CCR + pid);
			
			//Assurance de ne pas dépasser les valeurs min et max pour le moteur.
			if(newCCR2 < ENGINE_IDLE_CCR - ENGINE_RANGE_CCR) newCCR2 = ENGINE_IDLE_CCR - ENGINE_RANGE_CCR;
			else if (newCCR2 > ENGINE_IDLE_CCR + ENGINE_RANGE_CCR) newCCR2 = ENGINE_IDLE_CCR + ENGINE_RANGE_CCR;
			
			TIM3->CCR2 = newCCR2;
			
			delay_ms(20); //Ralentit la cadence de la clock pour permettre au PID d'avoir une valeur de dérivée non-nulle.
		}else{
			//MODE CONFIGURATION
			struct potentiometreValues val = getPotentiometreValue();
			center = (float)val.Cp;
			Kp = val.Kp;
			Ki = val.Ki;
			Kd = val.Kd;
			LCD_update(center, Kp, Ki, Kd);
		}
	}
}
