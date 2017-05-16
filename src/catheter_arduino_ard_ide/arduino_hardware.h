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

