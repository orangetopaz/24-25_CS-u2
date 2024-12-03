#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

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
  pinOff(_enable_pin, LOW);
  if (_rw_pin != 255){ 
    pinOff(_rw_pin, LOW);
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

void setup(){
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