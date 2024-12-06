#include "screen.h"
#include "utilities.h"

void LCD_printFloat(float x){
	char str[5];
	for(int i = 0; i < 4; i++) str[i] = '\0';
	char * res = floatToString(x, str);
	for(int i = 0; i < 4; i++){
		if(res[i] != '\0'){
			LCD_write_data(res[i]);
		}else{
			break;
		}
	}
}

void LCD_init(float cp, float p, float i, float d)
{
    LCD_clear(); // Clear LCD
    delay_ms(1); // Attente après clear
    // Pour le centre
    LCD_write_command(0x80); // Position cursor to 1st row, position 0
    LCD_write_data('C');
    LCD_write_data('p');
    LCD_write_data(':');
	  LCD_printFloat(cp);
	

    // Pour le Paramètre : P
    LCD_write_command(0x89); // Position cursor to 2nd row, position 0
		LCD_write_data('K');
    LCD_write_data('p');
    LCD_write_data(':');
		LCD_printFloat(p);

    // Pour le Paramètre : I
    LCD_write_command(0xC0); // Position cursor to 2nd row, position 9
    LCD_write_data('K');
		LCD_write_data('i');
    LCD_write_data(':');
		LCD_printFloat(i);

    // Paramètre : D
    LCD_write_command(0xC9); // Position cursor to 1st row, position 9
    LCD_write_data('K');
		LCD_write_data('d');
    LCD_write_data(':');
		LCD_printFloat(d);
}

void LCD_update(float cp, float p, float i, float d){
		LCD_write_command(0x83); // Position cursor to 1st row, position 0
	  LCD_printFloat(cp);
	

    // Pour le Paramètre : P
    LCD_write_command(0x8C); // Position cursor to 2nd row, position 0
		LCD_printFloat(p);

    // Pour le Paramètre : I
    LCD_write_command(0xC3); // Position cursor to 2nd row, position 9
		LCD_printFloat(i);

    // Paramètre : D
    LCD_write_command(0xCC); // Position cursor to 1st row, position 9
		LCD_printFloat(d);
}

void SPI1_init(void) {
    // Activation de l'horloge pour GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Configuration des broches SPI : PA4 (NSS), PA5 (SCK), PA7 (MOSI)
    MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER5, 2); // PA5 en AF  SCK
    MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER7, 2); // PA7 en AF  MOSI
    MODIFY_FIELD(GPIOA->MODER, GPIO_MODER_MODER4, 2); // PA4 en AF  /CS

    MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL5, 0); // PA5 AF0
    MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL7, 0); // PA7 AF0
    MODIFY_FIELD(GPIOA->AFR[0], GPIO_AFRL_AFSEL4, 0); // PA4 AF0

    // Activation de l'horloge pour SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Configuration de SPI1 en mode maître
    MODIFY_FIELD(SPI1->CR1, SPI_CR1_MSTR, 1); // Mode maître
    MODIFY_FIELD(SPI1->CR1, SPI_CR1_BR, 3); // Baud rate divisé par 16
    MODIFY_FIELD(SPI1->CR1, SPI_CR1_CPOL, 0); // Polarité 0
    MODIFY_FIELD(SPI1->CR1, SPI_CR1_CPHA, 0); // Phase 0

    MODIFY_FIELD(SPI1->CR2, SPI_CR2_SSOE, 1); // Activation de NSS
    MODIFY_FIELD(SPI1->CR2, SPI_CR2_DS, 7); // Taille des données 8 bits

    MODIFY_FIELD(SPI1->CR1, SPI_CR1_SPE, 1); // Activer SPI
		
		LCD_set_contrast();
}

void SPI1_write(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE)); // Attendre que le buffer TX soit prêt
    *(volatile uint8_t *)&SPI1->DR = data; // Écrire dans le registre
    while (SPI1->SR & SPI_SR_BSY); // Attendre que la transmission soit terminée
    delay_ms(1); // Délai après envoi des données
}

void SPI1_write_string(const char* string) {
    while (*string) {
        SPI1_write(*string++);
    }
}

void LCD_set_contrast(){
		GPIOA->BSRR = MASK(4) << 16; // NSS à LOW
    SPI1_write(0x7C); // Préfixe de commande pour SerLCD
    SPI1_write(0x18); // Commande pour régler le contraste
    SPI1_write(150); // Valeur de contraste entre 0 et 255
    GPIOA->BSRR = MASK(4); // NSS à HIGH
}

void LCD_write_command(uint8_t command) {
    GPIOA->BSRR = MASK(4) << 16; // NSS à LOW
    SPI1_write(0xFE); // Préfixe de commande pour SerLCD
    SPI1_write(command);
    GPIOA->BSRR = MASK(4); // NSS à HIGH
}

void LCD_write_data(uint8_t data) {
    GPIOA->BSRR = MASK(4) << 16; // NSS à LOW
    SPI1_write(data); // Envoyer un caractère
    GPIOA->BSRR = MASK(4); // NSS à HIGH
}

void LCD_clear(void) {
    LCD_write_command(LCD_CLEAR_CMD);
}
