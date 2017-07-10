/**************************************************************************
   Copyright {2017} Russell C Jackson

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


/* enable or disable the H-bridge on a given channel (Active LOW) */
/**
 * @brief Enable or disable an H-bridge channel.
 *
 * @param channel The H-bridge channel to be enable/disable.
 * @param en This is set to 1 if the H-bridge is to be enable, set to 0 otherwise.
 */
void toggle_enable(int channel, int en) {
  if (en == 0) {  // disable 
    digitalWrite(H_Enable_pins[channel], !H_EN);
  }
  else {  // enable 
    digitalWrite(H_Enable_pins[channel], H_EN);
  }
}


/**
 * @brief Set the direction of an H-bridge channel.

 * @param channel The H-bridge channel whose direction is to be set.
 * @param direction The direction of the H-bridge channel.
 */
void set_direction(int channel, int direction)
{
  
  if (direction == 0)
  {
    digitalWrite(H_Neg_pins[channel], DIR_ON);
    digitalWrite(H_Pos_pins[channel], !DIR_ON);
  }
  else
  {
    digitalWrite(H_Pos_pins[channel], DIR_ON);
    digitalWrite(H_Neg_pins[channel], !DIR_ON);
  }

}

// zeros out the channel completely
void zero(int channel)
{
  
  digitalWrite(H_Neg_pins[channel], DIR_ON);
  digitalWrite(H_Pos_pins[channel], DIR_ON);

}
/** 
 * @brief translate the 16 bit SPI data received from the MCP3201 ADC to the 12 bits sent to the PC 
 *
 * The bits are split so that the 6 MSB are in the upper byte (Right justified) 
 * While the 6 LSB are in the lower byte (also right justified)
 * 
 * @param The raw 2-bytes read by the arduino.
 * 
 * @return the 16 bit split value of the ADC.
 * There should be 12 valid bits.
 * This function returns 0x4000 when there is a problem.
 */
uint16_t processADC(uint16_t inputWord)
{
  // find the null bit:
  uint16_t tempWord(inputWord);
  int shiftInd = 0;
  while (tempWord > 32768)
  {
    shiftInd++;
    tempWord <<= 1;
  }
  if (shiftInd > 3)
  {
    return(0x4000);
  }
  else
  {
    // since bit 2-13 are part of the data stream, move them to be
    // bits 5-16 (shift by 3)
    tempWord >>= 3;
    uint16_t MSB((tempWord >> 6) << 8);
    uint16_t LSB((tempWord & 0b00111111));
    uint16_t outputWord(MSB + LSB);
    return outputWord;
  }
}

