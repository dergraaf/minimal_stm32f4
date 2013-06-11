
#include <stdint.h>
#include <stdbool.h>

#include <stm32f4xx.h>

extern void delay_us(uint32_t us);
extern void clock_init(void);

/**
 * Stub required by newlibc.
 * 
 * E.g. for malloc()
 */
void
_sbrk(void)
{
}

/**
 * Stub required by newlibc.
 *
 * Used for static constructors in C++
 */
void
_init(void)
{
}

static volatile uint32_t time = 0;

/**
 * Systick Interrupt handler
 */
void
SysTick_Handler(void)
{
	time++;
}

void
systick_init(void)
{
	// Lower systick interrupt priority to lowest level
	NVIC_SetPriority(SysTick_IRQn, 0xf);
	
	// Set interrupt frequency to 1000 Hz
	SysTick->LOAD = (F_CPU / 1000) - 1;
	
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
	                SysTick_CTRL_ENABLE_Msk |
	                SysTick_CTRL_TICKINT_Msk;
}

int
main (void)
{
	clock_init();
	
	systick_init();	
	
	// Struct of PIO Port D
	GPIO_TypeDef *PD = GPIOD;

	// Orange LED is PD13
	// Blue LED is PD15
	// Set mode to 01 -> Output
	PD->MODER = (1 << (13*2)) | (1 << (15*2));
	
	// Switch LED on (is connected between IO and GND => active high)
	PD->ODR = (1 << 13) | (1 << 15);
	
	uint32_t count_led1 = 0;
	uint32_t count_led2 = 0;
	while(1)
	{
		// Toggle LED1 every 500 ms
		if ((time - count_led1) >= 500) {
			count_led1 = time;

			if (PD->ODR & (1 << 13)) {
				PD->BSRRH = (1 << 13);
			}
			else {
				PD->BSRRL = (1 << 13);
			}
		}
		
		// Toggle LED2 every 300 ms
		if ((time - count_led2) >= 300) {
			count_led2 = time;

			if (PD->ODR & (1 << 15)) {
				PD->BSRRH = (1 << 15);
			}
			else {
				PD->BSRRL = (1 << 15);
			}
		}
	}
}



