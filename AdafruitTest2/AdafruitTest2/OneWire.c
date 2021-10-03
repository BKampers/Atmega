/*
   Copyright (c) 2007, Jim Studt  (original old version - many contributors since)
*/

#include "OneWire.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>


#define DS18B20_PORT PORTD
#define DS18B20_DDR DDRD
#define DS18B20_PIN PIND
#define DS18B20_DQ PORTD2

#define DIRECT_MODE_INPUT DS18B20_DDR &= ~_BV(DS18B20_DQ)
#define DIRECT_MODE_OUTPUT DS18B20_DDR |= _BV(DS18B20_DQ)
#define DIRECT_READ (DS18B20_PIN & _BV(DS18B20_DQ))
#define DIRECT_WRITE_LOW DS18B20_PORT &= ~_BV(DS18B20_DQ)
#define DIRECT_WRITE_HIGH DS18B20_PORT |= _BV(DS18B20_DQ)


#define interrupts() ;
#define noInterrupts() ;


void initOneWire()
{
	DIRECT_MODE_INPUT;
#if ONEWIRE_SEARCH
	reset_search();
#endif
}


// Perform the onewire reset function.  Wait up to 250 micro for
// the bus to come high, if it doesn't then it is broken or shorted.
//
// Returns true if a device asserted a presence pulse, false otherwise.
//
bool reset(void)
{
	uint8_t r;
	uint8_t retries = 125;
	noInterrupts();
    DIRECT_MODE_INPUT;
	interrupts();
    
	// wait until the wire is high... just in case
	do {
		if (--retries == 0) return 0;
		_delay_us(2);
	} while (! DIRECT_READ);

	noInterrupts();
	DIRECT_WRITE_LOW;
	DIRECT_MODE_OUTPUT;	// drive output low
	interrupts();
	_delay_us(480);
	noInterrupts();
	DIRECT_MODE_INPUT;	// allow it to float
	_delay_us(70);
	r = ! DIRECT_READ;
	interrupts();
	_delay_us(410);
	return r != 0;
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
void write_bit(uint8_t v)
{
	if (v & 1)
    {
		noInterrupts();
		DIRECT_WRITE_LOW;
		DIRECT_MODE_OUTPUT;	// drive output low
		_delay_us(10);
		DIRECT_WRITE_HIGH;	// drive output high
		interrupts();
		_delay_us(55);
	}
    else
    {
		noInterrupts();
		DIRECT_WRITE_LOW;
		DIRECT_MODE_OUTPUT;	// drive output low
		_delay_us(65);
		DIRECT_WRITE_HIGH;	// drive output high
		interrupts();
		_delay_us(5);
	}
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
uint8_t read_bit(void)
{
	uint8_t r;
	noInterrupts();
	DIRECT_MODE_OUTPUT;
	DIRECT_WRITE_LOW;
	_delay_us(3);
	DIRECT_MODE_INPUT;	// let pin float, pull up will raise
	_delay_us(10);
	r = DIRECT_READ;
	interrupts();
	_delay_us(53);
	return r != 0;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
void write(uint8_t value, bool power) {
    uint8_t bitMask;
    for (bitMask = 0x01; bitMask != 0; bitMask <<= 1)
    {
        write_bit(((bitMask & value) != 0) ? 1 : 0);
    }
    if (! power)
    {
        noInterrupts();
        DIRECT_MODE_INPUT;
        DIRECT_WRITE_LOW;
        interrupts();
    }
}

void write_bytes(const uint8_t *buf, uint16_t count, bool power)
{
    for (uint16_t i = 0 ; i < count ; i++)
    {
        write(buf[i], 0);
    }
    if (! power)
    {
        noInterrupts();
        DIRECT_MODE_INPUT;
        DIRECT_WRITE_LOW;
        interrupts();
    }
}

//
// Read a byte
//
uint8_t read()
{
    uint8_t bitMask;
    uint8_t r = 0;
    for (bitMask = 0x01; bitMask != 0; bitMask <<= 1)
    {
        if (read_bit())
        {
            r |= bitMask;
        }
    }
    return r;
}

void read_bytes(uint8_t *buf, uint16_t count)
{
  for (uint16_t i = 0 ; i < count ; i++)
    buf[i] = read();
}

//
// Do a ROM select
//
void select(const uint8_t rom[8])
{
    uint8_t i;

    write(0x55, 0);           // Choose ROM

    for (i = 0; i < 8; i++) write(rom[i], 0);
}

//
// Do a ROM skip
//
void skip()
{
    write(0xCC, 0);           // Skip ROM
}

void depower()
{
	noInterrupts();
	DIRECT_MODE_INPUT;
	interrupts();
}

#if ONEWIRE_SEARCH

//
// You need to use this function to start a search again from the beginning.
// You do not need to do it for the first search, though you could.
//
void reset_search()
{
  // reset the search state
  LastDiscrepancy = 0;
  LastDeviceFlag = false;
  LastFamilyDiscrepancy = 0;
  for(int i = 7; ; i--) {
    ROM_NO[i] = 0;
    if ( i == 0) break;
  }
}

// Setup the search to find the device type 'family_code' on the next call
// to search(*newAddr) if it is present.
//
void target_search(uint8_t family_code)
{
   // set the search state to find SearchFamily type devices
   ROM_NO[0] = family_code;
   for (uint8_t i = 1; i < 8; i++)
      ROM_NO[i] = 0;
   LastDiscrepancy = 64;
   LastFamilyDiscrepancy = 0;
   LastDeviceFlag = false;
}

//
// Perform a search. If this function returns a '1' then it has
// enumerated the next device and you may retrieve the ROM from the
// address variable. If there are no devices, no further
// devices, or something horrible happens in the middle of the
// enumeration then a 0 is returned.  If a new device is found then
// its address is copied to newAddr.  Use reset_search() to
// start over.
//
// --- Replaced by the one from the Dallas Semiconductor web site ---
//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
bool search(uint8_t *newAddr, bool search_mode)
{
   uint8_t id_bit_number;
   uint8_t last_zero, rom_byte_number;
   bool    search_result;
   uint8_t id_bit, cmp_id_bit;

   unsigned char rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = false;

   // if the last call was not the last one
   if (!LastDeviceFlag) {
      // 1-Wire reset
      if (!reset()) {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = false;
         LastFamilyDiscrepancy = 0;
         return false;
      }

      // issue the search command
      if (search_mode) {
        write(0xF0, 0);   // NORMAL SEARCH
      } else {
        write(0xEC, 0);   // CONDITIONAL SEARCH
      }

      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = read_bit();
         cmp_id_bit = read_bit();

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break;
         } else {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit) {
               search_direction = id_bit;  // bit write value for search
            } else {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < LastDiscrepancy) {
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               } else {
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);
               }
               // if 0 was picked then record its position in LastZero
               if (search_direction == 0) {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
              ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
              ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            // serial number search direction write bit
            write_bit(search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0) {
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!(id_bit_number < 65)) {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0) {
            LastDeviceFlag = true;
         }
         search_result = true;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !ROM_NO[0]) {
      LastDiscrepancy = 0;
      LastDeviceFlag = false;
      LastFamilyDiscrepancy = 0;
      search_result = false;
   } else {
      for (int i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
   }
   return search_result;
  }

#endif
