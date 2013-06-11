
#include <stdint.h>
#include <stdbool.h>

#include <stm32f4xx.h>

extern void delay_us(uint32_t us);

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

bool
clock_init(void)
{
	// Use external 8 MHz crystal on the STM32F4Discovery board
	RCC->CR |= RCC_CR_HSEON;
	
	uint32_t t = 1500;
	while (!(RCC->CR & RCC_CR_HSERDY)) {
		if (!(--t)) {
			// External crystal couldn't be enabled
			return false;
		}
	}
	
	// Enable PLL
	uint32_t cfgr;
	
	// Read reserved values and clear all other values
	cfgr  = RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM
			| RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP | RCC_PLLCFGR_PLLQ);
	
	// PLLSRC source for pll and for plli2s
	cfgr |= RCC_PLLCFGR_PLLSRC_HSE;
	
	// PLLM (0) = factor is user defined VCO input frequency must be configured to 2MHz
	cfgr |= ((uint32_t) 4) & RCC_PLLCFGR_PLLM;
	
	// PLLN (6) = factor is user defined
	cfgr |= (((uint32_t) 168) << 6) & RCC_PLLCFGR_PLLN;
	
	// VCO output frequency must be configured to 336MHz
	// PLLP (16) = 0 (factor = 2) for CPU frequency = 168MHz
	// PLLQ (24) = 7 (factor = 7) for 48MHz
	cfgr |= (7UL << 24) & RCC_PLLCFGR_PLLQ;
		
	RCC->PLLCFGR = cfgr;
	
	// enable pll
	RCC->CR |= RCC_CR_PLLON;
	
	// Wait until PLL is ready
	t = 1500;
	while (!(RCC->CR & RCC_CR_PLLRDY)) {
		if (!(--t)) {
			return false;
		}
	}
	
	// APB2 84MHz, APB1 42MHz, AHB 168MHz, select PLL as source
	RCC->CFGR =
			(RCC->CFGR & 0xffff0000) | // Try to generate a halfword write
			((RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_SW_PLL) & 0x0000ffff);
	
	// Wait till the main PLL is used as system clock source
	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	{
	}

	return true;
}

int
main (void)
{
	clock_init();
	
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
		count_led1++;
		if (count_led1 >= 500) {
			count_led1 = 0;

			if (PD->ODR & (1 << 13)) {
				PD->BSRRH = (1 << 13);
			}
			else {
				PD->BSRRL = (1 << 13);
			}
		}
		
		count_led2++;
		if (count_led2 >= 300) {
			count_led2 = 0;

			if (PD->ODR & (1 << 15)) {
				PD->BSRRH = (1 << 15);
			}
			else {
				PD->BSRRL = (1 << 15);
			}
		}
		
		delay_us(1000);
	}
}



