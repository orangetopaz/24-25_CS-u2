#include <stddef.h>
#include <stdint.h>

// The GPIO registers base address.

#define I2C_BASE 0x3F804000  // Adjust based on Pi version  // all different regesters for reading and writing different things.
#define I2C_C    *(volatile uint32_t *)(I2C_BASE + 0x00)  // C= Control Regester
#define I2C_S    *(volatile uint32_t *)(I2C_BASE + 0x04)  // S= Status Regester
#define I2C_DLEN *(volatile uint32_t *)(I2C_BASE + 0x08)  // DLEN= Data Length
#define I2C_A    *(volatile uint32_t *)(I2C_BASE + 0x0C)  // A= Slave regester, the 1st 7 bits of which contain the address of the I2C component (unique to each device, it depends.) for LCD displays, usualy 0x27, but can be 0x3F rarely
#define I2C_FIFO *(volatile uint32_t *)(I2C_BASE + 0x10)  // FIFO= first in first out, very small amount of storage

/* enum {  // Probobly not going to use this as I can't find a ttc to usb, but good just in case

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
}; */

#define GPIO_BASE      0x3F200000  // for raspi2 & 3, 0x20200000 for raspi1
  // turn gpio base into an address (pointer) to be writen to with a new versoin of the variable that is now lowercase
#define GPFSEL0        0x00
#define GPFSEL1        0x04
#define GPFSEL2        0x08
#define GPFSEL3        0x0c
#define GPFSEL4        0x10
#define GPFSEL5        0x14
#define GPFSET0        *(volatile uint32_t *)(GPIO_BASE + 0x1c)
#define GPFCLR0        *(volatile uint32_t *)(GPIO_BASE + 0x28)
#define GPFSET1        *(volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPFCLR1        *(volatile uint32_t *)(GPIO_BASE + 0x2c)

void pinFunc(unsigned int pinN, uint32_t funcSet){
  // function to bits:
    // setToInput: 0b000
    // setToOutput: 0b001
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
  volatile uint32_t* GPFSEL = (volatile uint32_t *)(GPIO_BASE + ((pinN / 10) * 4));  // intager division
  // declare a pointer with a * before the name at any point, refrence the target of the pointer and not just the pos by including the defferance operator, which is conincedentaly also an astrix before a pointer var
  *GPFSEL &= ~(0b111 << bitPos);  // clears the position of the funcset area, by putting ...000000011100000000... at the pos, then inverting to only and (clear) the necessary 3 bits, and set everything else to 1, so that it preserves the original setting when anded.
  *GPFSEL |= (funcSet << bitPos);  // gets the full 32 bit func like ...00000000"001"000000... then oring it to make sure not to clear anytning else by setting it to 0

}

// set/clear regesters for different pins
  // pins 0-31: CLR0/SET0
  // pins 32-53: CLR1/SET1
void pinOn(unsigned int pinN){  // 1 leftshifted by the pin number into the regester
  if (pinN <= 31){
    GPFSET0 = (1 << pinN);  // assignes to the regester its pointing at. deference operator * not neccessary as it's baked into the definition at the top
  }
  else if (pinN >= 32){
    GPFSET1 = (1 << (pinN-32));
  }
}
void pinOff(unsigned int pinN){
  if (pinN <= 31){
    GPFCLR0 = (1 << pinN);
  }
  else if (pinN >= 32){
    GPFCLR1 = (1 << (pinN-32));
  }
}

void OSDelay(int reps){
  while (reps--) {
    asm volatile("nop"); // empty loop to create delay, compiler might optomize but I don't trust macos that much
  }
}

void I2CInitalize() {
  // Set the I2C clock rate, address, and enable I2C mode
  I2C_C = 0;  // Disable I2C temporarily for configuration
  I2C_A = 0x27;  // Address for your LCD, adjust as needed. only the 1st 7 bits of the 32 of this regester matter/are written to, it is the address. in this case, it's address 39, or 0b100111, or 0x27
  // Set clock rate by configuring I2C_C (Clock Control)
  // Enable the interface after configuration
  I2C_C |= (1 << 15);  // Enable I2C. Bit 15 is the BSC (Broadcom (manufacturer of raspi2 processor) Serial Controller) enable/disable (1/0) (page 29)
}

void I2CByteSend(uint8_t cmdOrData){
  I2C_DLEN = 1;  // DLEN has 16 bits to store the length. it counts through this to send the bits one at a time
  I2C_FIFO = cmdOrData;  // there are some commands that I can send to the screen to do certain things. EG, 0x01 clears, 0x0C turns on w/o cursor, 0x06 set auto increment for cursor, 0x30 wake up (send multiple times), 0x28 turn on 4 bit mode and 2 line display 
  I2C_C |= (1 << 7);  // sets the start (7) bit to 1, to start transmission of command
  while (!(I2C_S & (1 << 1)));  // checks if the DONE (1) bit, anded with 1 = 1. if it equals 0, then invertend and the loop continues, but if 1, inverted is 0, loop ends
  I2C_S |= (1 << 1); // clear the DONE bit 
}

/*
the FIFO regester sends data to the LCD. the bits work out like this:
bits 0:3 - Char/Command
bit 4 - Regester select (RS) (0 for sending command, 1 for sending char/data)
bit 5 - read/write (R/W) (0 for writing to LCD, 1 for reading from LCD)
bit 6 - enable (E), rising edge signal (0 -> 1), then falling edge (1 -> 0) (send command twice with same data, exept change enable from 1 to 0) latches then sends data to LCD
bit 7 - backlight control (BL) (if appliccable). usualy 1 for backlight on, 0 for backlight off
*/
unsigned short backlight = 1;  // bool backlight = true;
void LCDSendCmd(uint8_t cmd){  
  uint8_t settings = 0b00000000;  // & 0b[BL]0[RS = 0][R/W = 0]0000, *sends* a *command*
  settings |= (backlight << 7);  //0b[backlight]0000000
  
}
void LCDSendChar(char ch){  
  
}

void kernel_main() {
  I2CInitalize();
}