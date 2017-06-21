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

#include <iostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "catheter_arduino_gui/digital_analog_conversions.h"


TEST(ard_hardware_conv, ADC_DAC)
{
  // for each potential value of current, validate that the ADC then back to DAC is valid.
  for (uint16_t index(0); index < 4096; index++)
  {
    double amperageVal(adc2MilliAmp(index));
    uint16_t index_(milliAmp2Dac(amperageVal));
    // ASSERT_EQ(index, index_);
  }
}

TEST(ard_hardware_conv, DAC_ADC)
{
  // for each potential value of current, validate that the ADC then back to DAC is valid.
  double di(0.000001);
  for (double i(0.0); i < 300.0; i+=di)
  {
    uint16_t dacVal(milliAmp2Dac(i));
    double i_(adc2MilliAmp(dacVal));
    double e_(abs(i-i_));
    // ASSERT_LT(e_, 1/12.8);
  }
}


int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
