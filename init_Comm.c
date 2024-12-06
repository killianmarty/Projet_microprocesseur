#include "init_Comm.h"
#include <stdio.h>
#include "stm32f0xx.h"

#define MASK(x) (1UL << (x))

#define UNUSED(x) (void)(x)

#define MODIFY_FIELD(reg, field, value) \
((reg) = ((reg) & ~(field ## _Msk)) | \
(((uint32_t)(value) << field ## _Pos) & field ## _Msk))

void init_Comm(void){
	/* Configuration du timer 3 pour delais */
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 8-1;
	TIM3->ARR = 1000-1;
	TIM3->CNT = 0;
	TIM3->CR1 = TIM_CR1_CEN;
	
	/*Configuration de PA2 en UART2_TX*/
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER2, 2); 
	MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL2, 1);
	
	// Configuration de PA3 en UART2_RX
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER3, 2); 
	MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL3, 1);
	
	// Configuration de UART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	USART2->BRR = (uint32_t)(8E6/9600);				// baud rate = 9600
	USART2->CR1 &= ~USART_CR1_M1; 					// 8 data bits, 1 stop bits
	USART2->CR1 &= ~USART_CR1_M0;
	USART2->CR1 &= ~USART_CR1_OVER8; 				// oversampling = 16
	USART2->CR1 |= USART_CR1_RE; 					// active la reception
	USART2->CR1 |= USART_CR1_TE; 					// active la transmission
	MODIFY_FIELD(USART2->CR2, USART_CR2_STOP, 0); 	// 1 stop bit
	USART2->CR3 = 0; 								// no flow control
	USART2->CR1 |= USART_CR1_UE; 					// active UART2
}
	

/* fonction pour ecrire un caractere avec USART2 */
int USART2_write (int ch) 
{
	while (!(USART2->ISR & USART_ISR_TXE)) 
	{
		/* Attend que le buffer de transmission soit vide */
	}   
	USART2->TDR = (ch & 0xFF);
	return ch;
}

/* Read a character from USART2 */
int USART2_read() 
{
	while (!(USART2->ISR & USART_ISR_RXNE)) 
	{
		/* attend de recevoir un caractere */
	}
	return USART2->RDR;
}


/* The code below is the interface to the C standard I/O library.
 * All the I/O are directed to the console, which is UART2.
 */

/* Les 4 lignes qui suivent causent une erreur avec compilateur V6
//struct __FILE 
//{ 
//	int handle; 
*/

FILE __stdin  = {0};
FILE __stdout = {1};
FILE __stderr = {2};

/* Called by C library console/file input
 * This function echoes the character received.
 * If the character is '\r', it is substituted by '\n'.
 */
int fgetc(FILE *f) 
{
	
	UNUSED(f);
	
	int c;

	c = USART2_read();      /* read the character from console */

	if (c == '\r') {        /* if '\r', after it is echoed, a '\n' is appended*/
			USART2_write(c);    /* echo */
			c = '\n';
	}

	USART2_write(c);        /* echo */

	return c;
}

/* Called by C library console/file output */
int fputc(int c, FILE *f) 
{
	UNUSED(f);
	return USART2_write(c);  /* write the character to console */
}
