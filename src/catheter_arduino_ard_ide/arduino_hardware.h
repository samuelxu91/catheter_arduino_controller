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


/*
 * @brief delayMaker toggles itself inside a for loop that is used to create time delay
 * 
 * Since this program implements interruption, it is undesirable to use delayMicroseconds(), which disables interruption when executed.
 * A for loop that toggles delayMaker during each iteration is used instead to create delays of microseconds. 
 * 
 */
bool delayMaker;


/**
 * @brief Enable or disable an H-bridge channel.
 *
 * @param channel The H-bridge channel to be enable/disable.
 * @param en This is set to H_EN if the H-bridge is to be enable, set to !H_EN otherwise.
 */
void set_enable(int channel, int en)
{
  digitalWrite(H_Enable_pins[channel], en);
}


/**
 * @brief Set the direction of an H-bridge channel.
 * @param channel The H-bridge channel whose direction is to be set.
 * @param direction The direction of the H-bridge channel. direction=0 indicates negative direction (current flows from Load_N to Load_P) and direction=1 indicates positive direction (current flows from Load_P to Load_N)
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

