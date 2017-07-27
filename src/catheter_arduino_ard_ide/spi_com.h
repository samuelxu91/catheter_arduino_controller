/* Copyright 2017 Russell Jackson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// @TODO(RCJ) If this header file is ever used outside of the arduino IDE,
// then add header guards

#include <SPI.h>

/*
 * The Chip select is 
#define CS_EN LOW

/*
 * The preprocessor definitions below are for the DAC control bits.
 * Currently, most control functionality is unused so these are constants
 * Full documentation is available online inside the MCP4921 datasheet.
 */
#define DAC_SELECT  0b00000000    /* write to DAC A;          bit 15 */
#define INPUT_BUF   0b00000000    /* Vref input not buffered; bit 14 */
#define GAIN_SELECT 0b00100000    /* output gain = 1;         bit 13 */
#define PWR_DOWN    0b00010000    /* output power-down ctrl;  bit 12 */

#define CS_EN LOW

/** 
 * @brief  read a 12-bit value from the MCP3201 ADC on a specified channel 
 * (0 through NCHANNELS-1).
 *
 * All the data lines are shared across the SPI bus except for the chip select
 * pin (CS).
 *
 * @param The index of the ADC channel being read. The range is [0, NCHANNELS-1].
 * 
 * @return the 16 bit unsigned value of the ADC.
 * Note that there is a pull down bit to indicate the start of the valid data.
 * There should be 12 valid bits.
 */
uint16_t ADC_read(uint8_t channel)
{
  // 1. Enable the ADC chip
  // 2. Read the 16 bits using the SPI bus
  // 3. Disable the ADC chip
  digitalWrite(ADC_CS_pins[channel], CS_EN);
  uint16_t ret1(SPI.transfer16(0x0000));
  digitalWrite(ADC_CS_pins[channel], !CS_EN);
  return ret1;
}

/**
 * @brief Write a 12-bit value to the MCP4921 DAC on a specified channel 
 * (0 through NCHANNELS-1).
 *
 * While there are only 12 bits of information, data is transfered in 16 bit 
 * packets. The first 4 bits are control bits for the ADC. Their function is
 * available in the MCP4921 documentation.
 *
 * @param channel, the channel index with a range of [0, NCHANNELS-1].
 * @param the 12 bit MSB DAC data. The first 4 bits are ignored.
 */
void DAC_write(uint8_t channel, uint16_t to_dac)
{
  // First construct the 16 DAC data.
  // 1. break the data into 2 8 bit words,
  // 2. add the control bits to the MSB data,
  // 3. recombine the MSB and LSB to 1 16 bit wide data packet.
  byte dataMSB = highByte(to_dac);
  byte dataLSB = lowByte(to_dac);

  dataMSB &= 0b00001111;
  dataMSB = dataMSB | DAC_SELECT | INPUT_BUF | GAIN_SELECT | PWR_DOWN;

  uint16_t outputData(dataMSB << 8 | dataLSB);

  // 1. Enable the DAC chip
  // 2. Write the 16 bits using the SPI bus
  // 3. Disable the DAC chip
  digitalWrite(DAC_CS_pins[channel], CS_EN);
  SPI.transfer16(outputData);
  digitalWrite(DAC_CS_pins[channel], !CS_EN);
}

/**
 * @brief initialize the SPI bus. This function is called once during startup.
 * 
 * This ensures that all transfers are MSB first and that the SPI clock is not
 * too fast.
 */
void SPI_init()
{
  // Arduino specific SPI start up.
  // Also slow the clock down for the Due.
  SPI.begin();
#ifdef DUE
  // Sets the SPI clock to ~1.3 MHz on the Due
  SPI.setClockDivider(64);
#endif
  SPI.setBitOrder(MSBFIRST);

  // set all ADC outputs to zero.
  for (int i = 0; i <= NCHANNELS; i++) {
    DAC_write(i, (uint16_t)0);
  }
}
