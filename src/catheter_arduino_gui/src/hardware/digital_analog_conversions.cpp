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


#include "catheter_arduino_gui/digital_analog_conversions.h"


double adc2MilliAmp(uint16_t dataIn)
{
  // The ADC currently uses 12 bits (0-4095) to measure the range of 0-5 V
  // The voltage is sampled from a 1 ohm resistor with a 10x gain amplifier.


  double data_d(static_cast<double> (dataIn));

  data_d /= 4095.0;  // convert to digital samples (12 bits)
  data_d *= 5.0;     // scale by the 5 V voltage range
  data_d /= 10.0;    // divide by the gain amplifier (10x)
  data_d *= 1000.0;  // convert from Volts to mA through a 1 Ohm resisitor. (1000x)
  // if speed is necessary, the net coefficient is 0.1221001221 mA/step
  return data_d;
}


uint16_t milliAmp2Dac(double milliAmp)
{
  double dacVal(abs(milliAmp));
  // @TODO(rcj) Break down the 12.8 multiplier into its constituent components.

  dacVal *= 12.8;  // this constant is from previous code (needs to be defined)

  // cast the variable as a uint16_t and return.
  uint16_t dacValInt(static_cast<uint16_t> (dacVal));
  return dacValInt;
}

double dac2MilliAmp(uint16_t dacVal, dir_t dir)
{
  double dacOut(static_cast<double> (dacVal));
  dacOut /= 12.8;  // this constant is from previous code (needs to be defined)
  dacOut *= (dir) ? (1.0) : (-1.0);
  return dacOut;
}
