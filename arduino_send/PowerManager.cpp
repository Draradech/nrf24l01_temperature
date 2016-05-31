#include "PowerManager.h"

#include <avr/sleep.h>
#include <avr/wdt.h>

#include <avr/interrupt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

static volatile bool f_wdt = 0;

void PowerManager::setup(uint8_t waketime)
{
  uint8_t bb;

  cbi( SMCR,SE );      // sleep enable, power down mode
  cbi( SMCR,SM0 );     // power down mode
  sbi( SMCR,SM1 );     // power down mode
  cbi( SMCR,SM2 );     // power down mode 

  if (waketime > 9) waketime = 9;
  bb = waketime & 7;
  if (waketime > 7) bb|= (1<<5);
  bb|= (1 << WDCE);
  MCUSR &= ~(1<<WDRF);

  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  // set new watchdog timeout value
  WDTCSR = bb;

  WDTCSR |= _BV(WDIE);
}

bool PowerManager::sleep(void)
{
  bool wdt_wake;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System sleeps here
  sleep_disable();                     // System continues execution here when watchdog timed out 
  wdt_wake = f_wdt;
  f_wdt = 0;
  return wdt_wake;
}

void PowerManager::sleep(uint8_t cycles)
{
  for(;cycles !=0; cycles -= sleep());
}

ISR(WDT_vect) {
  f_wdt = 1;
}

