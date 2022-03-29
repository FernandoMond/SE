#include "MKL46Z4.h"
#include "lcd.h"

// LED (RG)
// LED_GREEN = PTD5 (pin 98)
// LED_RED = PTE29 (pin 26)

// SWICHES
// RIGHT (SW1) = PTC3 (pin 73)
// LEFT (SW2) = PTC12 (pin 88)

// Enable IRCLK (Internal Reference Clock)
// see Chapter 24 in MCU doc

void irclk_ini()
{
  MCG->C1 = MCG_C1_IRCLKEN(1) | MCG_C1_IREFSTEN(1);
  MCG->C2 = MCG_C2_IRCS(0); //0 32KHZ internal reference clock; 1= 4MHz irc
}

void clear_LCD(){
  LCD->WF8B[LCD_FRONTPLANE0] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE1] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE2] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE3] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE4] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE5] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE6] = LCD_CLEAR;
  LCD->WF8B[LCD_FRONTPLANE7] = LCD_CLEAR;
}
void delay(void)
{
  volatile int i;

  for (i = 0; i < 1000000; i++);
}
void short_delay(void)
{
  volatile int i;

  for (i = 0; i < 500000; i++);
}

// RIGHT_SWITCH (SW1) = PTC3
void sw1_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3);
}

// LEFT_SWITCH (SW2) = PTC12
void sw2_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 12);
}

int sw1_check()
{
  return( !(GPIOC->PDIR & (1 << 3)) );
}

int sw2_check()
{
  return( !(GPIOC->PDIR & (1 << 12)) );
}

// RIGHT_SWITCH (SW1) = PTC3
// LEFT_SWITCH (SW2) = PTC12
void sws_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3 | 1 << 12);
}

// LED_GREEN = PTD5
void led_green_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOD->PSOR = (1 << 5);
}

void led_green_toggle()
{
  GPIOD->PTOR = (1 << 5);
}

// LED_RED = PTE29
void led_red_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOE->PDDR |= (1 << 29);
  GPIOE->PSOR = (1 << 29);
}

void led_red_toggle(void)
{
  GPIOE->PTOR = (1 << 29);
}

// LED_RED = PTE29
// LED_GREEN = PTD5
void leds_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOE->PDDR |= (1 << 29);
  // both LEDS off after init
  GPIOD->PSOR = (1 << 5);
  GPIOE->PSOR = (1 << 29);
}

// Hit condition: (else, it is a miss)
// - Left switch matches red light
// - Right switch matches green light


int hitcount(volatile unsigned int* index,int green_led_on){
	int result = 0;
	
	//Miro si hay algun boton apretado, y segun sea el caso, controlo el resultado y apago el led.
	if (sw1_check()){
		(*index)++;
		if (green_led_on){
			result = 100;
			led_green_toggle();
		}else {
			result = 1;
			led_red_toggle();
			}
	}
	else {if(sw2_check()){
			(*index)++;
			if(green_led_on){
				led_green_toggle();
				result = 1;
			}else {
				result = 100;
				led_red_toggle();
			}
		}
	}
	return result;
}

int main(void)
{
  irclk_ini(); // Enable internal ref clk to use by LCD
  leds_ini();
  sws_ini();
  lcd_ini();
  lcd_display_dec(0);

  // 'Random' sequence :-)
  volatile unsigned int sequence = 0x32B14D98,
    index = 0;
    int result = 0;
    int green_led_on = 0;
    int round_toggle = 0;
    int aux = 0;
    //led_red_toggle();// TEST BORRAR

  while (index < 32) {
    if (sequence & (1 << index)) { //odd
      //
      // Switch on green led
      // [...]
      //
      green_led_on = 1;
      //Controlo si ya se ha encendido el led antes
      if (!round_toggle){
      	led_green_toggle();
	round_toggle = 1;      
      }
      
    } else { //even
      //
      // Switch on red led
      // [...]
      //
      green_led_on = 0;
      //Controlo si ya se ha encendido el led antes
      if (!round_toggle){
      	led_red_toggle();
      	round_toggle = 1;
      }
      
    }
    // [...]
    aux = hitcount(&index,green_led_on);
    result += aux;
    //Si se detecta pulsación, hay que encender el led correspondiente en la siguiente iteración
    if (aux){
    	round_toggle = 0;
    }
    lcd_display_time(result/100,result%100);
    //lcd_display_dec(result);
    short_delay();
  }

  // Stop game and show blinking final result in LCD: hits:misses
  // [...]
  //

  while (1) {
  clear_LCD();
  delay();
  lcd_display_time(result/100,result%100);
  delay();
  }

  return 0;
}
