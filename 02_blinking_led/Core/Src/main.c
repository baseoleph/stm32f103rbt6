#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"

#define CPU_FREC 8000000

void setUpHSE();
void setUpLed();

void EXTI15_10_IRQHandler(void)
{
    // Проверка, включен ли SysTick
    if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk))
    {
        // Отключение SysTick
        SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        // Отключаю тактирование порта А
        RCC->APB2ENR &= ~RCC_APB2ENR_IOPAEN;
    }
    else
    {
        setUpLed();
        SysTick_Config(CPU_FREC/2);
    }

    // Сбрасываю pending register
    EXTI->PR |= EXTI_PR_PR13;
}

void SysTick_Handler(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}

int main(void)
{
    setUpHSE();

    // Запускаю тактирование порта C (доступ к кнопке)
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    while(__HAL_RCC_GPIOC_IS_CLK_DISABLED());

    // Включение тактирования AFIO
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    while(__HAL_RCC_AFIO_IS_CLK_DISABLED());

    // EXTICR[] отвечает за выбор вывода
    AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI13_PC;


    // Falling trigger selection register
    EXTI->FTSR |= EXTI_FTSR_TR13;

    // Разрешаю прерывания в NVIC
    NVIC_EnableIRQ (EXTI15_10_IRQn);

    // Разрешаю прерывания в периферии
    EXTI->IMR |= EXTI_IMR_MR13;


    // Функцию увидел у DI HALT, но без нее тоже все работает.
    // Не понимаю, почему
//    __enable_irq();

    while (1)
    {
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
        RCC->CR |= RCC_CR_PLLON;
        while ((RCC->CR & RCC_CR_PLLON) != RCC_CR_PLLON);

        RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_DIV2;
        // Сбрасываю System clock switch
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        // Устанавливаю на режим тактирования от HSE
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

        // На всякий случай жду
//        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    }
}

void setUpLed()
{
    // Запускаю тактирование порта А
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    while(__HAL_RCC_GPIOA_IS_CLK_DISABLED());

    // Устанавливаю состояние для output mode push-pull
    GPIOA->CRL &= ~GPIO_CRL_CNF5;
    // output mode, max speed 50MHz
    GPIOA->CRL |= GPIO_CRL_MODE5_Msk;
}

