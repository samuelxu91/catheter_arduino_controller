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

/* *************** */
/* pin definitions */
/* *************** */

// chip select (part of SPI) (DAC)
int DAC_CS_pins[NCHANNELS] = {2, 3, 4, 5, 6, 7};

// chip select (part of SPI) (ADC)
int ADC_CS_pins[NCHANNELS] = {8, 9, 10, 11, 12, 13};

// h-bridge enable pins
int H_Enable_pins[NCHANNELS] = {22, 23, 24, 25, 26, 27};

// H_ neg pins
int H_Neg_pins[NCHANNELS] = {34, 35, 36, 37, 38, 39};

// H_pos pins
int H_Pos_pins[NCHANNELS] = {28, 29, 30, 31, 32, 33};

// DAC buffer load pins
// (held low auto loads input buffer) (high to low loads input buffer).
int DAC_LDAC_pins[NCHANNELS] = {46, 47, 48, 49, 50, 51};

// Camera data pins (outputs)
int CAMERA_PINS[3] = {64, 63, 62};

// MRI pin (Input)
int mriPin = 65;
