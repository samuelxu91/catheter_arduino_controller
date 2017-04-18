/**************************************************************************
   Copyright {2016} {Dr. Russell C Jackson}

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

#include <SPI.h>
#include "ard_due_defs.h"
#include "pin_defs.h"
#include "spi_com.h"
#include "serial_com_ard.h"
#include "cmd_support.h"
#include "cmd_parse.h"

/* structs to represent channel status, channel commands, and serial packets */

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
 * @brief Bytes read in by the Arduino serial bus.
 */
uint8_t inputBytes[512];

/**
 * @brief Bytes returned from the Arduino serial bus.
 */
uint8_t outputBytes[512];

/**
 * @brief Used for writing data to the camera GPIO.
 *
 * Indexed from 0 to 3. The GPIO bus width is 2 bits.
 */
unsigned int camera_counter;

/**
 * @brief The previous MRI imaging status.
 *
 * The MRI indicates imaging by setting an input pin.
 */
bool mriStatOld;

/**
 * @brief The Arduino time at which the MRI begins scanning.
 */
unsigned long tof = 0;

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
    
  // make sure counter is integer 0 - 3 
  // The integer is written to a 2 bit wide bus using MSB.
  counter = counter % 4;
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
    
  // This checks the MRI imaging status pin and writes that pin to 
  // the 3rd of the camera bus.
  // This pin is guaranteed to be on for at least 20 ms.
  int mriStatus = digitalRead(mriPin);
  if (mriStatus)
  {
    mriStartTime = millis();
    digitalWrite(CAMERA_PINS[2],HIGH);
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


/* set pins to output mode and put them in defined state */
void pin_init() {
	for (int i = 0; i<NCHANNELS; i++) {
		pinMode(ADC_CS_pins[i], OUTPUT);
		pinMode(DAC_CS_pins[i], OUTPUT);
		pinMode(H_Enable_pins[i], OUTPUT);
		pinMode(H_Neg_pins[i], OUTPUT);
		pinMode(H_Pos_pins[i], OUTPUT);
		pinMode(DAC_LDAC_pins[i], OUTPUT);
		if (i < 3) {
			pinMode(CAMERA_PINS[i], OUTPUT);
			digitalWrite(CAMERA_PINS[i], LOW);
		}
    
    pinMode(mriPin,INPUT);
    
		digitalWrite(ADC_CS_pins[i], !CS_EN);
		digitalWrite(DAC_CS_pins[i], !CS_EN);
		digitalWrite(DAC_LDAC_pins[i], LOW);
		digitalWrite(H_Enable_pins[i], !H_EN);
		digitalWrite(H_Neg_pins[i], !DIR_ON);
		digitalWrite(H_Pos_pins[i], DIR_ON);
	}
}

/* enable or disable the H-bridge on a given channel (Active LOW) */
void toggle_enable(int channel, int en) {
	if (en == 0) {  // disable 
		digitalWrite(H_Enable_pins[channel], !H_EN);
	}
	else {  // enable 
		digitalWrite(H_Enable_pins[channel], H_EN);
	}
}

//* set the direction of the H-bridge for a given channel */
void set_direction(int channel, int direction) {
	if (direction == 0) {
		digitalWrite(H_Neg_pins[channel], DIR_ON);
		digitalWrite(H_Pos_pins[channel], !DIR_ON);
	}
	else {
		digitalWrite(H_Pos_pins[channel], DIR_ON);
		digitalWrite(H_Neg_pins[channel], !DIR_ON);
	}
}

/* ************ */
/* main program */
/* ************ */

void setup() {
	pin_init();
	SPI_init();

	serial_init();

  
  for ( int i = 0; i < 512; i++)
  {
    inputBytes[i] = 0;
    outputBytes[i] = 0;
  }     
	camera_counter = 0;
	delay(START_DELAY);
  mriStatOld = false;
}

void loop() {
        if(serial_available())
        {
          uint8_t counter = (uint8_t) serial_available();
          uint8_t packetSize = read_bytes( inputBytes, counter);
          uint8_t packetIndex(0);
          uint8_t cmdCount(0);
          
          // when data is available parse it.
          //If the parsing passes the checksum, then it is acted on.
          if(cmd_check(inputBytes, packetSize, &packetIndex, &cmdCount))
          {
            camera_counter = camera_counter + 1;
	          
            uint8_t outputLength(0);
            // This function no longer actually changes 
            outputLength = cmd_parse(inputBytes, packetSize, cmdCount, outputBytes, packetIndex);
            write_bytes(outputBytes, outputLength);
          }
          else writeError(packetIndex);
        }
        int mriStat(camera_write(camera_counter));
        if (mriStat && !mriStatOld)
        {
          mriStatOld = true;
          tof = millis();
          // set the axial coil (channel 3) to be at 75 ma.
          // do NOT change directions.
          DAC_write(0, 0);
          DAC_write(1, 0);
          if ( channelList[2].enable == 0)
          {
            // renable channel 3 (toindexed as 2).
            // disable channel2
            toggle_enable(2, 0);
          }
          uint16_t maSettings(00); // This is the 75 ma (estimated value for the MRI).
          DAC_write(2, maSettings);
        }
        if (!mriStat && mriStatOld)
        {
          if ((tof+40) <  (millis()))
          { 
            mriStatOld = false;
            // resets the coil currents to their previous values.
            // do NOT change directions.
            DAC_write(0, channelList[0].DAC_val);
            DAC_write(1, channelList[1].DAC_val);
            // reset channel 3 (toindexed as 2) to desired value.
            toggle_enable(2, channelList[2].enable);
            set_direction(2, channelList[2].dir);
            //and direction.
          
            DAC_write(2, channelList[2].DAC_val);
          }
        }
        
}  //end loop




