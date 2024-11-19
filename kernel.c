#include <stddef.h>
#include <stdint.h>

// The GPIO registers base address.
#define GPIO_BASE      0x3f200000  // for raspi2 & 3, 0x20200000 for raspi1
#define GPFSEL0        0x00
#define GPFSEL1        0x04
#define GPFSEL2        0x08
#define GPFSEL3        0x0c
#define GPFSEL4        0x10
#define GPFSEL5        0x14
#define GPFSET0        0x1c
#define GPFCLR0        0x28
#define GPIOVAL        0x200000

#define I2C_BASE 0x3F804000  // Adjust based on Pi version
#define I2C_C    *(volatile uint32_t *)(I2C_BASE + 0x00)  // all different regesters for reading and writing different things. C= Control Regester
#define I2C_S    *(volatile uint32_t *)(I2C_BASE + 0x04)  // C= Control Regester
#define I2C_DLEN *(volatile uint32_t *)(I2C_BASE + 0x08)  // S= Status Regester
#define I2C_A    *(volatile uint32_t *)(I2C_BASE + 0x0C)  // DLEN= Control Regester
#define I2C_FIFO *(volatile uint32_t *)(I2C_BASE + 0x10)  // FIFO= Control Regester

enum
{

  GPPUD = (GPIO_BASE + 0x94),
  GPPUDCLK0 = (GPIO_BASE + 0x98),

  // The base address for UART.
  UART0_BASE = 0x3F201000, // for raspi2 & 3, 0x20201000 for raspi1

  UART0_DR     = (UART0_BASE + 0x00),
  UART0_RSRECR = (UART0_BASE + 0x04),
  UART0_FR     = (UART0_BASE + 0x18),
  UART0_ILPR   = (UART0_BASE + 0x20),
  UART0_IBRD   = (UART0_BASE + 0x24),
  UART0_FBRD   = (UART0_BASE + 0x28),
  UART0_LCRH   = (UART0_BASE + 0x2C),
  UART0_CR     = (UART0_BASE + 0x30),
  UART0_IFLS   = (UART0_BASE + 0x34),
  UART0_IMSC   = (UART0_BASE + 0x38),
  UART0_RIS    = (UART0_BASE + 0x3C),
  UART0_MIS    = (UART0_BASE + 0x40),
  UART0_ICR    = (UART0_BASE + 0x44),
  UART0_DMACR  = (UART0_BASE + 0x48),
  UART0_ITCR   = (UART0_BASE + 0x80),
  UART0_ITIP   = (UART0_BASE + 0x84),
  UART0_ITOP   = (UART0_BASE + 0x88),
  UART0_TDR    = (UART0_BASE + 0x8C),
};

int kernel_main(){
  return 0;
}