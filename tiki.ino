// MCP23017 pins 15~17 to GND, I2C bus address is 0x20
#include <TinyWireM.h>
#include <stdlib.h>
#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

#define F_CPU 8000000  // This is used by delay.h library
// Routines to set and claer bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define INTERNAL2V56NC (5)  // We use the internal voltage reference

//************ USER PARAMETERS***********************
/* Three PWM Outputs */
volatile uint8_t* Port[] = {&OCR0B, &OCR1A, &OCR1B};
// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;
int mode = 1;
bool set = 1;
int i = 253;

struct settings_t
{
  int set;
  int mode;
  
} settings;

// the setup routine runs once when you press reset:
void setup()  {
  // Set up IO pins
  TinyWireM.begin(); // wake up I2C bus
  // set I/O pins to outputs
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x00); // IODIRA register
  TinyWireM.write(0x00); // set all of port A to outputs
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x01); // IODIRB register
  TinyWireM.write(0x00); // set all of port B to outputs
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x00); // IODIRA register
  TinyWireM.write(0x00); // set all of port A to outputs
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x01); // IODIRB register
  TinyWireM.write(0x00); // set all of port B to outputs
  TinyWireM.endTransmission();
  delay(50);
  
  setup_watchdog(5);
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
  //
  // read eeprom for previous mode.
  eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
  // change set to opposite
  if      (settings.set == 0){settings.set = 1; flash2i2c(255,0);}
  else if (settings.set == 1){settings.set = 0; flash2i2c(0,255);}
  else                       {settings.set = 1; flash2i2c(255,0);}
  // read last mode from previous run.
  eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
  // flash the state of set
}

// the loop routine runs over and over again forever:
void loop()
{
  mode = settings.mode;
  set = settings.set;
  if      (mode == 0) {setup_watchdog(1); tiki();         }
  else if (mode == 1) {setup_watchdog(4); flip_flop(0);  }
  else if (mode == 2) {setup_watchdog(4); flip_flop(1);  }
  else if (mode == 3) {setup_watchdog(4); tic_toc();     }
  else if (mode == 4) {setup_watchdog(3); sweep();       }
  else if (mode == 5) {setup_watchdog(2); binary_count();}
  else if (mode == 6) {setup_watchdog(2); knight_rider();}
  else if (mode == 7) {setup_watchdog(4); grey_counter();}
  else if (mode == 8) {setup_watchdog(3); rand_flash();  }
  else if (mode == 9) {setup_watchdog(3); rand_on();     }
  else if (mode == 10){setup_watchdog(3); loop_roof();   }
  else if (mode == 11){setup_watchdog(4); flash_all();   }
  else if (mode == 12){setup_watchdog(7); all_on();      }

  if(settings.set == 1)
  {     
    mode++;
    if(mode == 1){mode = 0;}
    settings.mode = mode;
    eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
  }
}


ISR(TIMER1_COMPA_vect) {
  if (!bitRead(TIFR,TOV1)) bitSet(PORTB, 3);
}

ISR(TIMER1_OVF_vect) {
  bitClear(PORTB, 3);
}

void tiki()
{
  int fire[9] = {16,48,112,240,0,8,12,14,15};// {15,7,3,1,31,63,127,255,16,32,64,128,8,4,2,1};
  write2bothi2c(0,255); //make eyes glow ,  mouth off
  for(int a = 0; a < 20; a++)
  {
    int r2 = random(0,4);
    int r1 = random(4,8);
    int flame = fire[r1]+fire[r2];
    //write_no_sleep_a_i2c(r1);
    write_a_i2c(flame);
    //write_no_sleep_b_i2c(0);
  }
}
void loop_roof()
{
  for(int a = 0; a < 10; a++)
  {
    write_a_i2c(1);
    write_a_i2c(2);
    write_a_i2c(4);
    write_a_i2c(8);
    write_a_i2c(16);
    write_a_i2c(32);
    write_a_i2c(64);
    write_a_i2c(128);
    write_no_sleep_a_i2c(0);
    write_b_i2c(1);
    write_b_i2c(2);
    write_b_i2c(4);
    write_b_i2c(8);
    write_b_i2c(16);
    write_b_i2c(32);
    write_b_i2c(64);
    write_b_i2c(128);
    write_no_sleep_b_i2c(0);
  }
}

void rand_flash()
{
  for(int a = 0; a < 25; a++)
  {
    int r1 = random(0,255);
    int r2 = random(0,255);
    flash2i2c(r1,r2);
  }
  write_no_sleep_a_i2c(0);
  write_no_sleep_b_i2c(0);
}

void rand_on()
{
  for(int a = 0; a < 25; a++)
  {
    int r1 = random(0,255);
    int r2 = random(0,255);
    write2bothi2c(r1,r2);
  }
  write_no_sleep_a_i2c(0);
  write_no_sleep_b_i2c(0);
}

void grey_counter()
{
  int g = 0;
  int b = 0;
  for(; b < 255; b++)
  {
    g = bintogray(b);
    write2i2c(g);
  }
  for(b = 255; b > 0; b--)
  {
    g = bintogray(b);
    write2i2c(g);
  }
}

void knight_rider()
{
  for (int a = 1; a < 8; a++)
  {
    write_b_i2c(1);
    write_b_i2c(2);
    write_b_i2c(4);
    write_b_i2c(8);
    write_b_i2c(16);
    write_b_i2c(32);
    write_b_i2c(64);
    write_b_i2c(128);
    write_b_i2c(64);
    write_b_i2c(32);
    write_b_i2c(16);
    write_b_i2c(8);
    write_b_i2c(4);
    write_b_i2c(2);
    write_b_i2c(1);
    write_no_sleep_b_i2c(0);
  }
    write_no_sleep_b_i2c(0);
    write_a_i2c(1);
    write_a_i2c(2);
    write_a_i2c(4);
    write_a_i2c(8);
    write_a_i2c(16);
    write_a_i2c(32);
    write_a_i2c(64);
    write_a_i2c(128);
    write_a_i2c(64);
    write_a_i2c(32);
    write_a_i2c(16);
    write_a_i2c(8);
    write_a_i2c(4);
    write_a_i2c(2);
    write_a_i2c(1);
    write_no_sleep_a_i2c(0);
}

void flash_all()
{
  for (int a = 0; a < 25; a++)
  {
    flash2i2c(255,255);
  }
  write_no_sleep_a_i2c(0);
  write_no_sleep_b_i2c(0);
}

void all_on()
{
  write2i2c(255);
}

void binary_count()
{
  for (int a = 0; a < 255; a++)
  {
     write2i2c(a);
  }
}

void flip_flop(int f)
{
  if(f == 0){
    for (int a = 1; a < 25; a++ ) {
      write2bothi2c(85,85);   // 01100101 , 01010101
      write2bothi2c(170,170);  // 10011010 , 10101010
    }
  }
  else{
    for (int a = 1; a < 25; a++ ) {
      flash2i2c(85,85);   // 01010101 , 01010101
      flash2i2c(170,170);  // 10101010 , 10101010
    }
  }
}

void sweep()
{
  for (int a = 3; a < 256; a = a << 1)
  {
      write2i2c(a);
      write2i2c(0);
  }
  for (int b = 192; b > 1; b = b >> 1)
  {
      write2i2c(b);
      write2i2c(0);
  }
}

void tic_toc()
{
  for (int a = 1; a < 255; a = (a << 1) | 1)
  {
      write2i2c(a);
      write2i2c(0);
  }
  for (int b = 255; b > 1; b = b >> 1)
  {
      write2i2c(b);
      write2i2c(0);
  }
}

int bintogray(int a)
{
  return a ^ (a >> 1);
}

void flash2i2c(int a, int b)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(b); // port B
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(b); // port B
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(0); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(0); // port B
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(0); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(0); // port B
  TinyWireM.endTransmission();
  if (f_wdt == 1) { // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt = 0;     // reset flag
    system_sleep();  // Send the unit to sleep
  }
}

void write2bothi2c(int a, int b)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(b); // port B
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(b); // port B
  TinyWireM.endTransmission();
  if (f_wdt == 1) { // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt = 0;     // reset flag
    system_sleep();  // Send the unit to sleep
  }
}

void write2i2c(int a)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(a); // port B
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  TinyWireM.beginTransmission(0x21);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(a); // port B
  TinyWireM.endTransmission();
  if (f_wdt == 1) { // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt = 0;     // reset flag
    system_sleep();  // Send the unit to sleep
  }
}

void write_a_i2c(int a)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
  if (f_wdt == 1) { // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt = 0;     // reset flag
    system_sleep();  // Send the unit to sleep
  }
}

void write_no_sleep_a_i2c(int a)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x12); // GPIOA
  TinyWireM.write(a); // port A
  TinyWireM.endTransmission();
}

void write_b_i2c(int b)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(b); // port B
  TinyWireM.endTransmission();
  if (f_wdt == 1) { // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt = 0;     // reset flag
    system_sleep();  // Send the unit to sleep
  }
}

void write_no_sleep_b_i2c(int b)
{
  TinyWireM.beginTransmission(0x20);
  TinyWireM.write(0x13); // GPIOB
  TinyWireM.write(b); // port B
  TinyWireM.endTransmission();
}

// set system into the sleep state
// system wakes up when wtchdog is timed out
void system_sleep() {
  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System actually sleeps here
  sleep_disable();                     // System continues execution here when watchdog timed out
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int i) {
  byte b;
  int w;
  if (i > 9 ) i = 9;
  b = i & 7;
  if (i > 7) b |= (1 << 5);
  b |= (1 << WDCE);
  w = b;

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCR = b;
  WDTCR |= _BV(WDIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt = 1; // set global flag
}
