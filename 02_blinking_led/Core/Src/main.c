#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"

#define CPU_FREC 8000000
#define BUTTON_OFF 0
#define BUTTON_ON 1

void setUpHSE();
void setUpLed();

uint32_t is_button_clicked = 0;
void SysTick_Handler(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}

int main(void)
{
//    setUpHSE();
    setUpLed();

    // Запускаю тактирование порта C (доступ к кнопке)
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    // На всякий случай жду (где-то вычитал, что запуск
    // порта может занимать до трех тактов)
    while(__HAL_RCC_GPIOC_IS_CLK_DISABLED());

    while (1)
    {
        // Проверяю, нажата ли в данный момент кнопка
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
        {
            // Сбрасываю флаг
            is_button_clicked = BUTTON_OFF;
        }
        else
        {
            // Условие сработает при нажатии только один раз
            // пока не будет отпущена кнопка
            if (is_button_clicked == BUTTON_OFF)
            {
                is_button_clicked = BUTTON_ON;

                // Если таймер был включен, отключаю таймер и led
                // в противном случае запускаю таймер
                if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk))
                {
                    SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                }
                else
                {
                    SysTick_Config(CPU_FREC);
                }
            }
        }
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

