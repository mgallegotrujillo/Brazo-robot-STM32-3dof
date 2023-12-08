/* Posiciones de Home 
	Y (Primer eslavon): 1704 
	X (Segundo eslavon): 
*/

// ********************************* LIBRERIAS *************************
		#include <stdio.h>
		#include "STM32f7XX.h"
// *********************************************************************

// ********************************* VARIABLES GLOBALES ****************
		// Variables UART
			char caracter_frase='1'; // Variable donde le guardare los caracteres que vaya recorriendo en una frase
			short caracter1=0;
			short caracter2=0;  // Variables donde guardare la fragmentacion del valor a enviar
			short caracter3=0;
			short caracter4=0; 
			short dato_recibido=0;
		// Variables Timers 
			short comparacion_1=0;
			short comparacion_2=0;
			short posicion_relativa_1=0;
			short posicion_relativa_2=0;
			bool bandera_presionar=0;
			
		// Variables ADC
			short valor_adc_X=0;
			short valor_adc_Y=0;
			
		// Variables filtro
			float alpha_adc=0.05;
			float resultado_filtro_X=0;
			float resultado_filtro_Y=0;
			
		// Variables Systick
			short contador_delay=0;
// *********************************************************************

// ********************************* FUNCIONES *************************
		void delay(short tiempo){
			contador_delay=0;
			while(contador_delay<=tiempo){}
		}
		void enviar_caracter(char caracter){ // Inicio de enviar uncaracter 
			UART4->TDR =caracter;
			while((UART4->ISR &=0x80)==0){}  
	} // Fin funcion de enviar un caracter 
		void enviar_frase(char frase[]){ // Inicio funcion enviar frase 
		for(int i=0;caracter_frase!='*';i++){ // Inicio for para enviar
			caracter_frase=frase[i]; // Lo que hago es que a una variable externa le mando el caracter i de la frase 
			UART4->TDR = caracter_frase; // El contenido de la variable externa la mando al registro de transmision para mandar por el serial 
			while((UART4->ISR &=0x80)==0){} // While de tiempo para una correcta transmision de datos  
		} // Fin for para enviar 
		caracter_frase='1';
	} // Fin funcion enviar frase 
		void dividir_datos(short resultado){ // Inicio de funcion de dividir datos 
				caracter1 = resultado/1000;
				caracter2 = (resultado/100)%10;
				caracter3 = (resultado%100)/10;
				caracter4 = (resultado%10);
		} // Fin funcion de dividir numeros
		void convercion_analogica(){ // Inicio de la funcion convertir ADC 
				ADC3->CR2 |=0x40000000; // Activo SWSTART que es el pin para empezar a hacer la conversion ADC
				while((ADC3->SR &=0x02)==1){} // Si la bandera que se ubica en el pin 1 esta en '1' la conversion acabo 
		} // Fin de la funcion de covertir ADC
// *********************************************************************

// ********************************* INTERRUPCIONES ******************** 
		extern "C"{
			void ADC_IRQHandler(void){
				if(ADC3->SQR3 == 0x09){
					valor_adc_X=ADC3->DR; 
				}else if(ADC3->SQR3 == 0x0E){
					valor_adc_Y=ADC3->DR; 
				}
			}
			void UART4_IRQHandler(void){
				if(UART4->ISR & 0x20){ // Si el dato esta completamente recibido 
					dato_recibido = UART4->RDR; // Guardo el dato 
				}
			}
			void SysTick_Handler(void){
				// Delay por systick y no por for
				contador_delay++;
				if(contador_delay>2000){
					contador_delay=0;
				} // Fin delay 
			
			}
			void TIM3_IRQHandler(void){
				TIM3->SR &=~(0x01); // Apago la bandera de la interrupcion colocando un '0' en el bit 0
				
			}
		}
// *********************************************************************

// ********************************* MAIN ******************************
int main(void){
		// ***************************** PUERTOS ***************************
				RCC->AHB1ENR |=0x27; // A, B y F
		// *****************************************************************
	
		// ***************************** PINES *****************************
				GPIOA->MODER |=0xA002; // ALTERNATIVO pin 0, 6 y 7
				GPIOA->AFR[0]=0x22000008; // FUNCION ALTERNA 8 pin 0, FUNCION ALTERNA 2 pin 6 y 7
	
				GPIOB->MODER |=0x02; // ALTERNATIVO pin 0
				GPIOB->AFR[0] =0x2; // FUNCION ALTERNA 2´pin 0
				GPIOC->MODER &=0xFFFFFFC; // INPUT pin 0
			
				GPIOF->MODER |=0xFC0; // ANALOGICO pin 3, 4 y 5   
		// *****************************************************************
	
		// ***************************** SYSTICK ***************************
				SystemCoreClockUpdate();
				SysTick_Config(SystemCoreClock/1000); // 1ms
		// *****************************************************************
	
		// ***************************** UART ******************************
				RCC->APB1ENR |=0x80000; // Activo el reloj del UART 4 
				UART4->BRR =0x683; // 9600 Baudios
				UART4->CR1 |=0x2C; // Activo Rx, Tx y la interrupcion por Rx
				UART4->CR1 |=0x01; // Habilito el modulo UART
				NVIC_EnableIRQ(UART4_IRQn); 
		// *****************************************************************
	
		// ***************************** ADC *******************************
				RCC->APB2ENR |=0x400; // Activo el reloj del ADC3
				ADC3->CR1 |=0x20; // Activo la interrupcion EOC
				ADC3->CR2 |=0x400; // Activo la conversion seguida, no por paquetes
				ADC3->CR2 |=0x01; // Habilito la conversion ADC3
				NVIC_EnableIRQ(ADC_IRQn); // Activo el vector de interrupciones para el ADC
		// *****************************************************************	
	
		// ***************************** TIMERS ****************************
				RCC->APB1ENR |=0x02; // Activo el reloj del Timer 3 
				TIM2->EGR |=0x01; // Habilito la actualizacion de los registros cuando cambie CCRx
				TIM3->PSC =15; // Esto para tener frecuencua de 1MHz cuyo periodo es de 1us 
				TIM3->DIER |=0x01; // Activo la interrupcion por actualizacion de conteo 
				TIM3->ARR =20000; // El periodo del conteo sera de 10us para conseguir el pulso de dicho ancho
				TIM3->CCER |=0x1111; // Conecto la señal PWM a su pin fisico en el canal 1, 2 Y 3
				TIM3->CCMR1 = 0x6060; // PWM moodo 1 canal 1 y 2 
				TIM3->CCMR2 =0x6060; // PWM modo 1 canal 3
				TIM3->CCR1 =510; // Señal de comparacion 1
				TIM3->CCR2 =510; // Señal de comparacion 2
				TIM3->CCR3 =510; // Señal de comparacion 3
				TIM3->CR1 |=0x01;  // Activo el conteo 
				NVIC_EnableIRQ(TIM3_IRQn); // Activo la interrupcion 
		// *****************************************************************	
	
		// ***************************** BUCLE *****************************
				while(true){
					// Lectura del Eje X
					ADC3->SQR3 =0x09; 
					convercion_analogica();
					
					// Lectura del Eje Y
					ADC3->SQR3 =0x0E;
					convercion_analogica();
					
					// Lectura del boton 
					if((GPIOC->IDR &=0x01)==0x00){
						delay(500);
						bandera_presionar=bandera_presionar^1; 
						if(bandera_presionar==1){
							TIM3->CCR3 =2867;
						}else{
							TIM3->CCR3 =1653;
						}
					}
					
					// Filtrado de las señales 
					resultado_filtro_X=(alpha_adc*valor_adc_X)+((1-alpha_adc)*resultado_filtro_X);
					resultado_filtro_Y=(alpha_adc*valor_adc_Y)+((1-alpha_adc)*resultado_filtro_Y);
					posicion_relativa_1=resultado_filtro_X*2000/4095; // Escalo el resultado en el rango de 0 a 20000
					comparacion_1=posicion_relativa_1*1538/1017; 
					comparacion_1=posicion_relativa_1;
					posicion_relativa_2=resultado_filtro_Y*2000/4095; // Escalo el resultado en el rango de 0 a 20000
					comparacion_2=posicion_relativa_2*1683/1010;
					// Generacion de PWM
					TIM3->CCR1=1538/1017;
					TIM3->CCR2=1538/1077;
					TIM3->CCR1=1738/1017;
					TIM3->CCR2=1738/1057;
					TIM3->CCR1=1238/1017;
					TIM3->CCR2=1238/1047;
					TIM3->CCR1=1138/1017;
					TIM3->CCR2=1138/1037;
					TIM3->CCR1=1938/1017;
					TIM3->CCR2=1938/1027;
					
					// Envio de las señales
					enviar_caracter('X');
					enviar_caracter(':');
					enviar_caracter(' ');
					dividir_datos(comparacion_1);
					enviar_caracter(caracter1+0x30);
					enviar_caracter(caracter2+0x30);
					enviar_caracter(caracter3+0x30);
					enviar_caracter(caracter4+0x30);
					enviar_caracter(',');
					
					enviar_caracter('Y');
					enviar_caracter(':');
					enviar_caracter(' ');
					dividir_datos(comparacion_2);
					enviar_caracter(caracter1+0x30);
					enviar_caracter(caracter2+0x30);
					enviar_caracter(caracter3+0x30);
					enviar_caracter(caracter4+0x30);
					enviar_caracter('\n');
				}
		// *****************************************************************	
}
// *********************************************************************