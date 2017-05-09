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

/**
 * @brief Channel status is the status of the individual catheter coil.
 *
 * The status includes the following:
 * - enable Whether or not the channel is active.
 * - dir The current direction of the channel.
 * - DAC_val The set current (12 bits).
 * - ADC_val The sensed current (12 bits).
 */
struct channelStatus
{
  int enable;
  int dir;
  uint16_t DAC_val;
  uint16_t ADC_val;
};

/**
 * @brief Individual channel statuses.
 */
channelStatus channelList[NCHANNELS];

/**
 * @brief Enable or disable an H-bridge channel.
 *
 * @param channel The H-bridge channel to be enable/disable.
 * @param en This is set to 1 if the H-bridge is to be enable, set to 0 otherwise.
 */
void toggle_enable(int channel, int en)
{
 if (en == 0)
  {
    digitalWrite(H_Enable_pins[channel], !H_EN);
  }
  else
  {
    digitalWrite(H_Enable_pins[channel], H_EN);
  }
}
/**
 * @brief Write the camera_counter to the camera GPIO.
 *
 * This function uses the Arduino digital write.
 *
 * @param counter Counter passed in from the main function.
 */
int camera_write(int counter)
{
  static unsigned long mriStartTime(0);

  // We use two bits of the camera's GPIO for the counter.
  counter = counter % 4;

  // Set CAMERA_PINS according to the counter.
  if (counter % 2 == 1)
  {
    digitalWrite(CAMERA_PINS[0], HIGH);
  }
  else
  {
    digitalWrite(CAMERA_PINS[0], LOW);
  }

  if ((counter >> 1) % 2 == 1)
  {
    digitalWrite(CAMERA_PINS[1], HIGH);
  }
  else
  {
    digitalWrite(CAMERA_PINS[1], LOW);
  }

  // This checks the MRI imaging status pin.
  // This pin is guaranteed to be on for at least 20 ms.
  int mriStatus = digitalRead(mriPin);

  // Set CAMERA_PINS[2] according to the MRI status.
  if (mriStatus)
  {
    mriStartTime = millis();
    digitalWrite(CAMERA_PINS[2], HIGH);
  }
  else
  {
    if (mriStartTime != 0)
    {
      unsigned long currentTime(millis());
      // minimum on time for the MRI pin (20 ms).
      unsigned long minOn(20);

      // turn off the pin after the on time has passed.
      if ((mriStartTime + minOn) < currentTime)
      {
     digitalWrite(CAMERA_PINS[2],LOW);
      mriStartTime = 0;
      }

      // turn off the mri pin after the on time has passed in case there was
      // a zero wrapparound. (This happens every ~52 days)
      if (mriStartTime > currentTime && currentTime > minOn )
      {
        digitalWrite(CAMERA_PINS[2],LOW);
      mriStartTime = 0;
       }
    }
    else
    {
      digitalWrite(CAMERA_PINS[2],LOW);
      mriStartTime = 0;
    }
  }
  return mriStatus;
}


/**
 * @brief Initialize pin modes and states.
 */
void pin_init()
{
  pinMode(mriPin, INPUT);

  for (int i = 0; i < NCHANNELS; i++)
  {
    pinMode(ADC_CS_pins[i], OUTPUT);
    pinMode(DAC_CS_pins[i], OUTPUT);
    pinMode(H_Enable_pins[i], OUTPUT);
    pinMode(H_Neg_pins[i], OUTPUT);
    pinMode(H_Pos_pins[i], OUTPUT);
    pinMode(DAC_LDAC_pins[i], OUTPUT);
    digitalWrite(ADC_CS_pins[i], !CS_EN);
    digitalWrite(DAC_CS_pins[i], !CS_EN);
    digitalWrite(DAC_LDAC_pins[i], LOW);
    digitalWrite(H_Enable_pins[i], !H_EN);
    digitalWrite(H_Neg_pins[i], !DIR_ON);
    digitalWrite(H_Pos_pins[i], DIR_ON);
  }

  for (int i = 0; i < 3; i++)
  {
    pinMode(CAMERA_PINS[i], OUTPUT);
    digitalWrite(CAMERA_PINS[i], LOW);
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


/* calculate 8-bit fletcher checksum using blocksize=4 */
uint8_t fletcher8(uint8_t len, const uint8_t* data)
{
  uint8_t sum1 = 0, sum2 = 0;
  int i;
  for (i = 0; i < len; i++)
  {
    // first 4 bits
    sum1 += (data[i] >> 4);
    sum2 += sum1;

    // last 4 bits
    sum1 += (data[i] & 15);
    sum2 += sum1;

    // modulo 15
    sum1 %= 16;
    sum2 %= 16;
  }
  return ((sum2) << 4) + (sum1);
}

// Encodes an error response based on the input packet index
void errorEncode(uint8_t packetIndex, uint8_t *reply, uint8_t *replyLen)
{
  reply[0] = (packetIndex & 16);
  *replyLen = 1;
}

// Compact the valid response using a channel status with polling
uint8_t compactCmdResponsePoll(const channelStatus& local_)
{
  uint8_t cmd(0);
  cmd |= (1 << POL_B);
  if (local_.enable) cmd |= (1 << ENA_B);
  if (local_.dir) cmd |= (1 << DIR_B);
  return cmd;
}

// Compact the valid response using a channel status with out polling
uint8_t compactCmdResponse(const channelStatus& local_, bool update)
{
  uint8_t cmd(0);
  if (update) cmd |= (1 << UPD_B);
  if (local_.enable) cmd |= (1 << ENA_B);
  if (local_.dir) cmd |= (1 << DIR_B);
  return cmd;
}

// This function processes a single channel for the arduino.
uint8_t processSingleChannel(int i, uint8_t cmdVal, uint16_t cmdData, uint8_t *responseBytes, uint8_t *responseIndex)
{
  bool poll = (cmdVal >> 3) & 1;
  bool en = (cmdVal >> 2) & 1;
  bool upd = (cmdVal >> 1) & 1;
  bool dir = (cmdVal >> 0) & 1;
  // poll disables all other action.
  if (poll)
  {
    // get the channel index
    uint8_t channel((i+1) << 4);
    uint8_t cmdVal(compactCmdResponsePoll(channelList[i]));
    uint8_t response = channel | cmdVal;

    // get the DAC value;
    uint8_t DACU = (0b00111111 & (channelList[i].DAC_val >> 6));
    uint8_t DACL = (0b00111111 & channelList[i].DAC_val);
    // get the ADC settings:
    uint16_t  ADCm(ADC_read(i));
    // this is saved for now but does very little.
    channelList[i].ADC_val = ADCm;

    // channel number and channel commands
    responseBytes[*responseIndex] = response;
    // DAC upper 6 bits
    responseBytes[*responseIndex+1] = DACU;
    // DAC lower 6 bits
    responseBytes[*responseIndex+2] = DACL;
    // ADC upper 6 bits
    responseBytes[*responseIndex+3] = (uint8_t) ((ADCm >> 8));
    // ADC lower 6 bits
    responseBytes[*responseIndex+4] = (uint8_t) (0b11111111 & ADCm);
    *responseIndex += 5;
  return 1;
  }  // if(poll)
  else
  {
    // enable or disable channel
    toggle_enable(i, en);
    // saved but useless.
    channelList[i].enable = en;
    // set DAC value if necessary
    if (upd)
    {
      DAC_write(i, cmdData);
      channelList[i].DAC_val = cmdData;
    }
    // set H-brige direction
    set_direction(i, dir);
    channelList[i].dir = dir;

    // assign the DAC values (upper and lower)
    uint8_t DACU = (0b00111111 & (channelList[i].DAC_val >> 6));
    uint8_t DACL = (0b00111111 & channelList[i].DAC_val);

    uint8_t channel((i+1) << 4);
    uint8_t cmdVal(compactCmdResponse(channelList[i], upd));
    uint8_t response = channel | cmdVal;

    // channel number and channel commands
    responseBytes[*responseIndex] = response;
    // DAC upper 6 bits
    responseBytes[*responseIndex+1] = DACU;
    // DAC lower 6 bits
    responseBytes[*responseIndex+2] = DACL;
    *responseIndex += 3;
    return 0;
  }  // if (poll) else
}
