#include "main.h"
#include "stm32f1xx_hal.h"

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    while(__HAL_RCC_GPIOA_IS_CLK_DISABLED());

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LD2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  while (1)
  {
  }
}
