#include "stdint.h"
#include "cmsis_gcc.h"
//#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"

void setUpHSE();
void setUpLed();
void setLight();
void resetLight();

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
        setLight();
        Delay(55560);
        resetLight();
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
    // Адрес RCC
    uint32_t Reset_and_Clock_Control = 0x40021000UL;
    // Регистр шины APB2
    uint32_t APB2ENR = 0x18UL;
    // Получаю адрес регистра
    uint32_t *RCC_APB2ENR = Reset_and_Clock_Control + APB2ENR;

    uint32_t PORTX_ENABLE = 0x1;
    uint32_t IOPAEN = 0x2;
    uint32_t PORTA_ENABLE = PORTX_ENABLE << IOPAEN;
    // Запускаю тактирование порта A
    *RCC_APB2ENR |= PORTA_ENABLE;

    // Жду, пока тактирование запустится
    while((*RCC_APB2ENR & PORTA_ENABLE) == 0);

    // Адрес GPIO A
    uint32_t GPIOA__ = 0x40010800;

    // Регситр CRL
    uint32_t CRL__ = 0x0;
    uint32_t *GPIOA_CRL__ = GPIOA__ + CRL__;

    // Регистр CNF5
    uint32_t CNF5__ = 22;
    uint32_t CNF5_mask__ = 0b11;
    // Сбрасываю биты. Установится режим push-pull
    *GPIOA_CRL__ &= ~(CNF5_mask__ << CNF5__);

    // Регситр MODE5
    uint32_t MODE5__ = 20;
    // Режим скорости
    uint32_t max_speed_10 = 0b1;
    // Устанавливаю
    *GPIOA_CRL__ |= max_speed_10 << MODE5__;
}

void setLight()
{
    // Адрес GPIO A
    uint32_t GPIOA__ = 0x40010800;

    // Регистр BSRR
    uint32_t BSRR__ = 0x10;
    // Полный адрес GPIOx_BSRR
    uint32_t *GPIOA_BSRR__ = GPIOA__ + BSRR__;

    // Бит set 5 pin
    uint32_t BS5__ = 5;
    uint32_t set_bit = 0b1;
    *GPIOA_BSRR__ |= set_bit << BS5__;
}

void resetLight()
{
    // Адрес GPIO A
    uint32_t GPIOA__ = 0x40010800;

    // Регистр BSRR
    uint32_t BSRR__ = 0x10;
    // Полный адрес GPIOx_BSRR
    uint32_t *GPIOA_BSRR__ = GPIOA__ + BSRR__;

    // Бит reset 5 pin
    uint32_t BR5__ = 21;
    uint32_t set_bit = 0b1;
    *GPIOA_BSRR__ |= set_bit << BR5__;
}
