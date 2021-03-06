/**
  * Marlin 3D Printer Firmware
 * Copyright (C) 2017 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
/**
 * Support routines for Re-ARM board
 */


 
typedef struct {
  int8_t pin;
  bool output;
  bool analog;
  uint8_t resistors;
  bool open_drain;
  char function_string[15];
} pin_info;

pin_info pin_Re_ARM;
 
void get_pin_info(int8_t pin) {
  pin_Re_ARM.analog = 0;
  pin_Re_ARM.pin = pin;
  int8_t pin_port = pin_map[pin].port;
  int8_t pin_port_pin = pin_map[pin].pin;
 
  // active ADC function/mode/code values for PINSEL registers
  int8_t ADC_pin_mode = pin_port == 0 && pin_port_pin == 2  ? 2 :
                        pin_port == 0 && pin_port_pin == 3  ? 2 :
                        pin_port == 0 && pin_port_pin == 23 ? 1 : 
                        pin_port == 0 && pin_port_pin == 24 ? 1 :
                        pin_port == 0 && pin_port_pin == 25 ? 1 :
                        pin_port == 0 && pin_port_pin == 26 ? 1 :
                        pin_port == 1 && pin_port_pin == 30 ? 3 :
                        pin_port == 1 && pin_port_pin == 31 ? 3 : -1;       
  //get appropriate PINSEL register                                         
  volatile uint32_t * pinsel_reg = (pin_port == 0 && pin_port_pin <= 15) ? &LPC_PINCON->PINSEL0 :
                                   (pin_port == 0)                       ? &LPC_PINCON->PINSEL1 :
                                   (pin_port == 1 && pin_port_pin <= 15) ? &LPC_PINCON->PINSEL2 :
                                    pin_port == 1                        ? &LPC_PINCON->PINSEL3 :
                                    pin_port == 2                        ? &LPC_PINCON->PINSEL4 :
                                    pin_port == 3                        ? &LPC_PINCON->PINSEL7 : &LPC_PINCON->PINSEL9;                                                    

  uint8_t pinsel_start_bit = pin_port_pin > 15 ? 2 * (pin_port_pin - 16) : 2 * pin_port_pin;
  uint8_t pin_mode = (uint8_t) ((*pinsel_reg >> pinsel_start_bit) & 0x3);
  
  uint32_t * FIO_reg[5] PROGMEM = {(uint32_t*) 0x2009C000,(uint32_t*)  0x2009C020,(uint32_t*)  0x2009C040,(uint32_t*)  0x2009C060,(uint32_t*)  0x2009C080};
  pin_Re_ARM.output = (*FIO_reg[pin_map[pin].port] >> pin_map[pin].pin) & 1; //input/output state except if active ADC

  if (pin_mode) {  // if function/mode/code value not 0 then could be an active analog channel
    if (ADC_pin_mode == pin_mode) {  // found an active analog pin
      pin_Re_ARM.output = 0;
      pin_Re_ARM.analog = 1;
    }  
  }
}

/**
 * translation of routines & variables used by pinsDebug.h
 */
 
#define GET_PIN_INFO(pin) get_pin_info(pin)
#define IS_ANALOG(P) (DIGITAL_PIN_TO_ANALOG_PIN(P) >= 0 ? 1 : 0)
#define GET_PINMODE(pin) pin_Re_ARM.output
#define digitalRead_mod(p)  digitalRead(p)
#define digitalPinToPort_DEBUG(p)  0
#define digitalPinToBitMask_DEBUG(pin) 0
#define PRINT_PORT(p) SERIAL_ECHO_SP(10);
#define GET_ARRAY_PIN(p) pin_array[p].pin
#define NAME_FORMAT(p) PSTR("%-##p##s")
//  #define PRINT_ARRAY_NAME(x)  do {sprintf_P(buffer, NAME_FORMAT(MAX_NAME_LENGTH) , pin_array[x].name); SERIAL_ECHO(buffer);} while (0)
#define PRINT_ARRAY_NAME(x)  do {sprintf_P(buffer, PSTR("%-35s") , pin_array[x].name); SERIAL_ECHO(buffer);} while (0)
#define GET_ARRAY_IS_DIGITAL(x)  !pin_Re_ARM.analog