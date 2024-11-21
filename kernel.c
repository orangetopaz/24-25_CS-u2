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
#define GPFSET1        0x20
#define GPFCLR1        0x2c
#define GPIOVAL        0x200000

#define I2C_BASE 0x3F804000  // Adjust based on Pi version
#define I2C_C    *(volatile uint32_t *)(I2C_BASE + 0x00)  // all different regesters for reading and writing different things.
#define I2C_S    *(volatile uint32_t *)(I2C_BASE + 0x04)  // C= Control Regester
#define I2C_DLEN *(volatile uint32_t *)(I2C_BASE + 0x08)  // S= Status Regester
#define I2C_A    *(volatile uint32_t *)(I2C_BASE + 0x0C)  // DLEN= Control Regester
#define I2C_FIFO *(volatile uint32_t *)(I2C_BASE + 0x10)  // FIFO= Control Regester
volatile uint32_t *gpio_base = (volatile uint32_t *)GPIO_BASE;

enum {  // Probobly not going to use this as I can't find a ttc to usb, but good just in case

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

void pinFunc(unsigned int pinN, uint32_t funcSet){
  // function to bits:
    // setToOutput: 0b000
    // setToIutput: 0b001
    // setToAltFuc0: 0b100
    // setToAltFuc1: 0b101
    // setToAltFuc2: 0b110
    // setToAltFuc3: 0b111
    // setToAltFuc4: 0b011
    // setToAltFuc5: 0b010
  // pin function group regesters (each group of 3 of the 30-some bits of the regester corrispond to the function set for each pin)
    // pins 0-9: GPFSEL0
    // pins 10-19: GPFSEL1
    // pins 20-29: GPFSEL2
    // pins 30-39: GPFSEL3
    // pins 40-49: GPFSEL4
    // pins 50-53: GPFSEL5
  unsigned int bitPos = 3*(pinN%10);
  volatile uint32_t *GPFSEL = *gpio_base + (uint32_t *)((pinN / 10)*4);  // intager division
  // declare a pointer with a * before the name at any point, refrence the target of the pointer and not just the pos by including the defferance operator, which is conincedentaly also an astrix before a pointer var
  *GPFSEL &= ~(0b111 << bitPos);  // clears the position of the funcset area, by putting ...000000011100000000... at the pos, then inverting to only and (clear) the necessary 3 bits, and set everything else to 1, so that it preserves the original setting when anded.
  *GPFSEL |= (funcSet << bitPos);  // gets the full 32 bit func like ...00000000"001"000000... then oring it to make sure not to clear anytning else by setting it to 0

}

// set/clear regesters for different pins
  // pins 0-31: CLR0/SET0
  // pins 32-53: CLR1/SET1
void pinOn(unsigned int pinN){  // 1 leftshifted by the pin number into the regester
  if (pinN <= 31){
    
  }
  else if (pinN >= 32){

  }
}
void pinOff(unsigned int pinN){
  if (pinN <= 31){
    
  }
  else if (pinN >= 32){

  }
}

int kernel_main(){
  volatile uint32_t *gpio_base = (volatile uint32_t *)GPIO_BASE;  // turn gpio base into an address (pointer) to be writen to with a new versoin of the variable that is now lowercase
  pinFunc(21, 0b000);
  return 0;
}