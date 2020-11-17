#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"

#define CPU_FREC 8000000U

void initHSE();
void initLED();
void initRTC();
void setCounterToRTC(uint32_t cnt);

// Флаг для кнопки
uint32_t is_blink = 0;

// Включает и отключает лампочку каждую секунду
void RTC_IRQHandler(void)
{
    // Если прерывание вызвано SEC
    if (RTC->CRL & RTC_CRL_SECF)
    {
        // Запускаю SysTick, чтобы по его прерыванию затушить LED
        SysTick_Config(CPU_FREC/4);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
//        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // Убираю бит из SECF
        RTC->CRL &= ~RTC_CRL_SECF;
    }

    // Сбрасываю pending register
    EXTI->PR |= EXTI_PR_PR17;
}

// Реагирует на нажатие кнопки
void EXTI15_10_IRQHandler(void)
{
    if (is_blink)
    {
        // Отключаю отправку события по флагу SEC
        RTC->CRH &= ~RTC_CRH_SECIE;
        is_blink = 0;

        // Перестаю ждать собития на 17 линии
        EXTI->PR &= ~EXTI_PR_PR17;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
    else
    {
        // Обнуление счетчика в RTC
        setCounterToRTC(0);
        // Включаю отправку собитий по флагу SEC
        RTC->CRH |= RTC_CRH_SECIE;
        // Жду событий от RTC по линии 17
        EXTI->PR |= EXTI_PR_PR17;
        is_blink = 1;
    }

    // Сбрасываю pending register
    EXTI->PR |= EXTI_PR_PR13;
}

void SysTick_Handler(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    // Отключаю SysTick
    SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
}

int main(void)
{
    initHSE();
    initRTC();
    initLED();

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
    EXTI->FTSR &= ~EXTI_FTSR_TR17;

    // Rising trigger selection register
    EXTI->RTSR |= EXTI_RTSR_TR17;


    // Разрешаю прерывания в периферии
    EXTI->IMR |= EXTI_IMR_MR13;
    EXTI->IMR |= EXTI_IMR_MR17;

    EXTI->EMR &= ~EXTI_EMR_EM17;

    // Разрешаю прерывания в NVIC
    NVIC_EnableIRQ (EXTI15_10_IRQn);
    NVIC_EnableIRQ (RTC_IRQn);

    while (1)
    {
    }
}

void initHSE()
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
        // Устанавливаю на режим тактирования от HSE*PLL
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

        // На всякий случай жду
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    }
}

void initLED()
{
    // Запускаю тактирование порта А
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    while(__HAL_RCC_GPIOA_IS_CLK_DISABLED());

    // Устанавливаю состояние для output mode push-pull
    GPIOA->CRL &= ~GPIO_CRL_CNF5;
    // output mode, max speed 50MHz
    GPIOA->CRL |= GPIO_CRL_MODE5_Msk;
}

void initRTC()
{
    // Включаю тактирование PWR и Backup
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    // Разрешаю доступ к Backup области
    PWR->CR |= PWR_CR_DBP;
    // Сбрасываю Backup область
    RCC->BDCR |= RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    // Выбираю LSE
    RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;
    // Запускаю LSE
    RCC->BDCR |= RCC_BDCR_LSEON;
    while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){}

    // Проверка, что в програме никто не меняет регистры RTC в данный момент
    while (!(RTC->CRL & RTC_CRL_RTOFF));

    // Разрешение записи в регистры RTC
    RTC->CRL  |=  RTC_CRL_CNF;

    // Делитель 32656+1
    RTC->PRLL  = 0x7FFF;

    // Запрет записи в регистры RTF
    RTC->CRL  &=  ~RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF));

    // Синхронизация RTC
    RTC->CRL &= (uint16_t)~RTC_CRL_RSF;
    while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF);

    // Запрет доступа в Backup область
    PWR->CR &= ~PWR_CR_DBP;
}

void setCounterToRTC(uint32_t cnt)
{
    // Включаю тактирование PWR и Backup
    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    // Разрешаю доступ к Backup области
    PWR->CR |= PWR_CR_DBP;

    // Проверка, что в програме никто не меняет регистры RTC в данный момент
    while (!(RTC->CRL & RTC_CRL_RTOFF));

    // Разрешение записи в регистры RTC
    RTC->CRL  |=  RTC_CRL_CNF;

    RTC->CNTL = cnt >> 16;
    RTC->CNTH = cnt;

    // Запрет записи в регистры RTF
    RTC->CRL  &=  ~RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF));

    // Запрет доступа в Backup область
    PWR->CR &= ~PWR_CR_DBP;
}
