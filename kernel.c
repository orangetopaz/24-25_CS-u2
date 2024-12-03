#include <stddef.h>
#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
//#include <inttypes.h>
#include <time.h>

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

#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00
#define LCD_FUNCTIONSET 0x20

#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

#define HIGH 0x1
#define LOW 0x0

uint8_t _rs_pin = 26; // LOW: command. HIGH: character.
uint8_t _rw_pin = 255;
uint8_t _enable_pin = 19; // activated by a HIGH pulse.
uint8_t _data_pins[8] = {27, 6, 22, 5, 0, 0, 0, 0};
uint8_t _row_offsets[4];

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;

uint8_t _numlines;

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
  while (reps--){
    asm volatile("nop"); // empty loop to create delay, compiler might optomize but I don't trust macos that much
  }
}

void flash(unsigned int pinN){
  pinFunc(pinN, 0b001);
  pinOn(pinN);
  OSDelay(0x8000);
  pinOff(pinN);
}

void bianaryLEDout(unsigned int pinN0, unsigned int pinN1, uint32_t data, unsigned int reps){
  pinFunc(pinN0, 0b001);
  pinFunc(pinN1, 0b001);
  uint32_t tester = 0b00000000000000000000000000000000 | (1 << reps);
  for (unsigned int i = 0; i < reps; i++){
    if (data & (tester >> i)){
      pinOn(pinN1);
      OSDelay(5000000);
      pinOff(pinN1);
      OSDelay(500000);
    }
    else {
      pinOn(pinN0);
      OSDelay(5000000);
      pinOff(pinN0);
      OSDelay(500000);
    }
  }
}

void I2CInit(){
  // Set the I2C clock rate, address, and enable I2C mode
  I2C_C = 0;  // Disable I2C temporarily for configuration
  I2C_A = 0x4E;  // Address for your LCD, adjust as needed. only the 1st 7 bits of the 32 of this regester matter/are written to, it is the address. in this case, it's address 39, or 0b100111, or 0x27
  // Set clock rate by configuring I2C_C (Clock Control)
  // Enable the interface after configuration
  I2C_C |= (1 << 15);  // Enable I2C. Bit 15 is the BSC (Broadcom (manufacturer of raspi2 processor) Serial Controller) enable/disable (1/0) (page 29)
}

void I2CByteSend(uint8_t cmdOrData){
  I2C_C |= (1 << 4); // clear the FIFO
  I2C_DLEN = 1;  // DLEN has 16 bits to store the length. it counts through this to send the bits one at a time
  I2C_FIFO = cmdOrData;  // there are some commands that I can send to the screen to do certain things. EG, 0x01 clears, 0x0C turns on w/o cursor, 0x06 set auto increment for cursor, 0x30 wake up (send multiple times), 0x28 turn on 4 bit mode and 2 line display 
  I2C_C |= (1 << 7);  // sets the start (7) bit to 1, to start transmission of command
  while (!(I2C_S & (1 << 1)));  // checks if the DONE (1) bit, anded with 1 = 1. if it equals 0, then invertend and the loop continues, but if 1, inverted is 0, loop ends
  if (I2C_S & (1 << 8)){ // ERR bit set
    flash(16);
    I2C_C |= (1 << 4); // Clear FIFO as part of error recovery
  }
  I2C_S |= (1 << 1) | (1 << 8); // clear the DONE and ERR (error) bits
  //I2C_C |= (1 << 4); // Clear FIFO as part of error recovery
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
  uint8_t highNybble = cmd >> 4;  // Extract MSB
  uint8_t lowNybble = cmd & 0x0F;  // Extract LSB
  uint8_t settings = 0b00000000 | (backlight << 7); // & 0b[BL][E][RS = 0][R/W = 0]0000, *sends* a *command*
  // send first nibbyl
  I2CByteSend((settings | (1 << 6)) | highNybble);  // send it by 1ing Enable bit
  OSDelay(1); // Small delay to allow signals to stabilize
  I2CByteSend((settings & ~(1 << 6)) | highNybble);  // latch it by 0ing (NOT 1) Enable bit
  OSDelay(1); // Small delay to allow signals to stabilize
  // send second nibbyl
  I2CByteSend((settings | (1 << 6)) | lowNybble);  // send it by 1ing Enable bit
  OSDelay(1); // Small delay to allow signals to stabilize
  I2CByteSend((settings & ~(1 << 6)) | lowNybble);  // latch it by 0ing (NOT 1) Enable bit
}

void LCDSendChar(char ch){  
  uint8_t highNybble = ch >> 4;  // Extract MSB (most significant bit(s))
  uint8_t lowNybble = ch & 0x0F;  // Extract LSB
  uint8_t settings = 0b10010000 | (backlight << 7); // & 0b[BL][E][RS = 1][R/W = 0]0000, *sends* a *command*  //0b[backlight]0000000
  // send first nibbyl
  I2CByteSend((settings | (1 << 6)) | highNybble);  // send it by 1ing Enable bit
  OSDelay(1); // Small delay to allow signals to stabilize
  I2CByteSend((settings & ~(1 << 6)) | highNybble);  // latch it by 0ing (NOT 1) Enable bit
  OSDelay(1); // Small delay to allow signals to stabilize
  // send second nibbyl
  I2CByteSend((settings | (1 << 6)) | lowNybble);  // send it by 1ing Enable bit
  OSDelay(1); // Small delay to allow signals to stabilize
  I2CByteSend((settings & ~(1 << 6)) | lowNybble);  // latch it by 0ing (NOT 1) Enable bit
}

void cycleLEDS(){
  for (int i = 0; i < 5; i++){
    pinOn(16);  // Turn on red LED
    OSDelay(500000);
    pinOff(16); // Turn off red LED
    pinOn(20);  // Turn on green LED
    OSDelay(500000);
    pinOff(20); // Turn off green LED
    pinOn(21);  // Turn on blue LED
    OSDelay(500000);
    pinOff(21); // Turn off blue LED
  }
}

void delayMicroseconds(int number_of_microseconds)
{
  // Converting time into milli_seconds
  double milli_seconds = number_of_microseconds/1000;

  // Storing start time
  clock_t start_time = clock();

  // looping till required time is not achieved
  while (clock() < start_time + milli_seconds);
}

void begin(uint8_t cols, uint8_t lines, uint8_t dotsize){
  if (lines > 1){
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)){
    _displayfunction |= LCD_5x10DOTS;
  }

  pinFunc(_rs_pin, 0b001);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  if (_rw_pin != 255){ 
    pinFunc(_rw_pin, 0b001);
  }
  pinFunc(_enable_pin, 0b001);
  
  // Do these once, instead of every time a character is drawn for speed reasons.
  for (int i=0; i<((_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i){
    pinFunc(_data_pins[i], 0b001);
  } 

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40 ms after power rises above 2.7 V
  // before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
  delayMicroseconds(50000); 
  // Now we pull both RS and R/W low to begin commands
  pinOff(_rs_pin);
  pinOff(_enable_pin);
  if (_rw_pin != 255){ 
    pinOff(_rw_pin);
  }
  
  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)){
    // this is according to the Hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms
    
    // third go!
    write4bits(0x03); 
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02); 
  } else {
    // this is according to the Hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1 ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

void setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void display(){
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/*********** mid level commands, for sending data/cmds */

inline void command(uint8_t value){
  send(value, LOW);
}

inline size_t write(uint8_t value){
  send(value, HIGH);
  return 1; // assume success
}

/************ low level data pushing commands **********/

void pulseEnable(void){
  pinOff(_enable_pin);
  delayMicroseconds(1);    
  pinOn(_enable_pin);
  delayMicroseconds(1);    // enable pulse must be >450 ns
  pinOff(_enable_pin);
  delayMicroseconds(100);   // commands need >37 µs to settle
}

void send(uint8_t value, uint8_t mode){
  digitalWrite(_rs_pin, mode);
  
  write4bits(value>>4);
  write4bits(value);
}

void write4bits(uint8_t value){
  for (int i = 0; i < 4; i++){
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  pulseEnable();
}


void kernel_main(){
  pinFunc(16, 0b001);
  pinFunc(20, 0b001);
  pinFunc(21, 0b001);
  pinFunc(3, 0b001);
  pinFunc(5, 0b001);

  cycleLEDS();

  begin(16, 2, 0);
  write('H');
  write('e');
  write('l');
  write('l');
  write('o');
  write(' ');
  write('W');
  write('o');
  write('r');
  write('l');
  write('d');
  write('!');
  write('!');
  write('!');
  write('!');
  write('!');
  write('!');
  write('!');
  write('!');
  write('!');
  write('!');
}