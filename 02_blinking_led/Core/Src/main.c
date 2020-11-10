#include "stdint.h"

int main(void)
{
    uint32_t Reset_and_Clock_Control = 0x40021000UL;
    uint32_t APB2ENR = 0x18UL;
    uint32_t *RCC_APB2ENR = Reset_and_Clock_Control + APB2ENR;

    uint32_t PORTX_ENABLE = 0x1;
    uint32_t IOPAEN = 0x2;
    uint32_t PORTA_ENABLE = PORTX_ENABLE << IOPAEN;
    *RCC_APB2ENR |= PORTA_ENABLE;

    while((*RCC_APB2ENR & PORTA_ENABLE) == 0);

    uint32_t GPIOA__ = 0x40010800;
    uint32_t BSRR__ = 0x10;
    uint32_t *GPIOA_BSRR__ = GPIOA__ + BSRR__;
    uint32_t *GPIOA_CRL__ = GPIOA__ + 0;

    *GPIOA_CRL__ &= ~(0b11 << 22);
    *GPIOA_CRL__ |= 0b1 << 20;

    *GPIOA_BSRR__ |= 0x20;
  while (1)
  {
  }
}
