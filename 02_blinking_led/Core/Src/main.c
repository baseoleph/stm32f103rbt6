#include "stdint.h"
#include "cmsis_gcc.h"
//#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"

void setUpHSE();
void setUpLed();
void Delay(uint32_t val)
{
    for (; val != 0; val--)
    {
        __NOP();
    }
}

int main(void)
{

    setUpHSE();
    setUpLed();

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        Delay(55560);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        Delay(55560);
    }
}

void setUpHSE()
{
    __IO uint32_t cnt = 0;
    __IO uint32_t HSEStatus = 0;

    RCC->CR = ((uint32_t)RCC_CR_HSEON);

    do
    {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        ++cnt;
    } while (HSEStatus == 0 && cnt != HSE_STARTUP_TIMEOUT);

    if ((RCC->CR & RCC_CR_HSERDY) != RESET)
    {
        HSEStatus = 0x1;
    }
    else
    {
        HSEStatus = 0x0;
    }

    if (HSEStatus == 0x1)
    {
        FLASH->ACR |= FLASH_ACR_PRFTBE;

        FLASH->ACR &= (uint32_t)((uint32_t) ~FLASH_ACR_LATENCY);
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;


        // AHB prescaler
        RCC->CFGR |= RCC_CFGR_HPRE_DIV512;

        // Как я понял это множитель на шину APB. Светодиод там.
        // Настраивать другие множители не нужно
        // APB2 prescaler
        RCC->CFGR |= RCC_CFGR_PPRE2_DIV16;

        // Как я это понимаю --> сбрасываем настройки pll модулей
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

        RCC->CR |= RCC_CR_PLLON;


        while ((RCC->CR & RCC_CR_PLLRDY) == 0);

        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (0x08));
    }
}

void setUpLed()
{
    // Запускаю тактирование порта А
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    // На всякий случай жду, пока запустится
    while(__HAL_RCC_GPIOA_IS_CLK_DISABLED());

    // Устанавливаю состояние для output mode push-pull
    GPIOA->CRL &= ~GPIO_CRL_CNF5;
    // output mode, max speed 50MHz
    GPIOA->CRL |= GPIO_CRL_MODE5_Msk;
}

