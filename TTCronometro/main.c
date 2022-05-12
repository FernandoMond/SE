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


// RTC_TSR Registro con el segundo
// RTC_SR TCE sTATUS REGISTER TIME COUNTER ENABLE
// RTC_SR TRTC_CR OSCE OSCILATOR ENABLE 
//  


//Con el selector controlamos en que modo esta, pudiendo ser la selección de un dígito, en pausa o en modo cuenta atrás
  int selector = 0; 	// 0 -> primer digito 
  			//1->segundo digito 
  			//2->tercer digito
  			//3->cuarto digito
  			// 4->Pausa
  			// 5->Cuenta atrás
  int minutos = 0;
  int segundos = 0;

  uint32_t tiempo = 0;
  
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


void actualizar_tiempo(){
	if (segundos == 0){
		if (minutos ==0){
			selector = 0;}
		else {
			segundos = 59;
			minutos--;
		}
	}
	else{
		segundos--;
	}
}

//cuando cambia valor, llamamos acualización

void sclock_init(){
  SIM->SCGC6 |= SIM_SCGC6_RTC_MASK;
  SIM->SOPT1 |= 0x00000000;
  RTC->CR = RTC_CR_OSCE(1); //Activamos el oscilador
  delay();
  RTC->CR = RTC_CR_CLKO(1);
  RTC->SR |= (1 << 4); //Activamos el reloj 
}



void ini_clock()
{
  SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK; 
  SIM->SOPT2 |= SIM_SOPT2_TPMSRC(0x03); //Le damos señal a 32kHz
  TPM0->SC = TPM_SC_CMOD(0x00); //Lo desactivamos
  TPM0->SC |= TPM_SC_CPWMS(0x00); 
  TPM0->SC |= TPM_SC_PS(0x00); // Seleccionamos que no divida en el Preescaler 
  TPM0->MOD = TPM_MOD_MOD(0x7CFF); // Cuenta hasta 31999
  TPM0->SC |= TPM_SC_TOF_MASK; // Se resete ael bit del TOF 
  TPM0->SC |= TPM_SC_CMOD(0x01); // Se vuelve a activar
}




// RIGHT_SWITCH (SW1) = PTC3
void sw1_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3);
  PORTC->PCR[3] |= PORT_PCR_IRQC(0xA);    
}

// LEFT_SWITCH (SW2) = PTC12
void sw2_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 12);
  PORTC->PCR[12] |= PORT_PCR_IRQC(0xA);   
}

void PORTDIntHandler(void) {
  int pressed_switch = PORTC->ISFR;
  PORTC->ISFR = 0xFFFFFFFF; // Clear IRQ

  // SW1
  if(pressed_switch == (0x8)) {
	//Codigo botón izquierdo
	if (selector == 3) {
		minutos = (minutos + 10)%100;
	}
	if (selector == 2) {
		minutos = minutos/10 + ((minutos+ 1) %10);
	}
	if (selector == 1) {
		segundos = (segundos + 10)%60;
	}
	if (selector == 0) {
		segundos = segundos/10 + ((segundos+ 1) %10);
	}
	if (selector == 4) {
	  selector = 5; //Activamos el reloj 
	 //SwapCountdown();
	}
	else if (selector == 5) {
	  selector = 4;
	}
  }
  // SW2
  if(pressed_switch == (0x1000)) {
  	//código boton derecho
  	
	  	if (selector == 4){
		  	minutos = 0;
		  	segundos = 0;
		  	selector = 0;
	  	}
	  	else if(selector<4 ){
			selector++;
		}
  }
  
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



void sws_ini(){
  SIM->COPC = 0;            
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  NVIC_SetPriority(31, 0);  
  NVIC_EnableIRQ(31);  
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





int main(void)
{
  irclk_ini(); // Enable internal ref clk to use by LCD
  ini_clock(); //Inicializa el reloj que usaremos para contar.
  leds_ini();
  sws_ini();
  sw1_ini();
  sw2_ini();
  lcd_ini();

  while(1){
  lcd_display_time(minutos,segundos);
  
  //Miramos si el contador ha sufrido overflow. Si ha ocurrido es que ha pasado 1 segundo.
  if ((TPM0->SC & TPM_SC_TOF_MASK) == TPM_SC_TOF_MASK){
  
  	//Si está en modo de funcionamiento normal, actualizamos el lcd
  	if (selector == 5){
  		actualizar_tiempo();
  		lcd_display_time(minutos,segundos);
  	}
  	
  	//Reseteamos el contador
  	TPM0->SC |= TPM_SC_TOF_MASK;
  	}
  }
  return 0;
}
