/*
  Copyright 2017 Russell Jackson

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#pragma once

#ifndef CATHETER_ARDUINO_GUI_DIGITAL_ANALOG_CONVERSIONS_H
#define CATHETER_ARDUINO_GUI_DIGITAL_ANALOG_CONVERSIONS_H

#include <cstdint>
#include <cstdlib>
#include "catheter_arduino_gui/catheter_commands.h"

// This file declares the functions capable of converting byte data to analog current
// I.E. ADC data to mA and mA to DAC data.


/**
 * @brief This function converts up to a 16 bit adc input to a milliAmp current.
*/
double adc2MilliAmp(uint16_t dataIn);

/**
* @brief This function converts up to a milliAmp current into a dac bit value.
*/
uint16_t milliAmp2Dac(double mA);

/**
* @brief This function converts the dac bit value to a milliAmp current.
*/
double dac2MilliAmp(uint16_t dacVal, dir_t dir);


#endif  // CATHETER_ARDUINO_GUI_DIGITAL_ANALOG_CONVERSIONS_H
