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

/* **************** */
/* Serial functions */
/* **************** */

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

// initialize the arduino serial bus.
void serial_init()
{
#ifdef DUE
  // set millisecond timeout so commands can be entered
  SerialUSB.setTimeout(2);
  SerialUSB.begin(BAUD);
#else
  Serial.setTimeout(2);
  Serial.begin(BAUD);
#endif
}

// check for available serial data and returns the number of available bytes.
int serial_available(void)
{
#ifdef DUE
  return SerialUSB.available();
#else
  return Serial.available();
#endif
}

// Empty the serial input buffer of the first count bytes.
void flush_bytes(int count)
{
  int i;
  for (i = 0; i < count; i++)
  {
#ifdef DUE
    SerialUSB.read();
#else
    Serial.read();
#endif
  }
}

// writes a single byte to the serial bus.
void write_byte(uint8_t b)
{
#ifdef DUE
  SerialUSB.write(b);
#else
  Serial.write(b);
#endif
}

// writes multiple bytes to the serial bus
void write_bytes(uint8_t bytes[], uint8_t nb)
{
  uint8_t i;
  for (i = 0; i<nb; i++)
  {
    write_byte(bytes[i]);
  }
}

// reads and returns a single serial byte.
uint8_t read_byte()
{
#ifdef DUE
  return (uint8_t)(SerialUSB.read());
#else
  return (uint8_t)(Serial.read());
#endif
}

// waits for the outgoing data to be written
void serial_flush()
{
  #ifdef DUE
    SerialUSB.flush();
    #else
    Serial.flush();
    #endif
}

// attempt to read count bytes and returns the number of successfully read bytes.
uint8_t read_bytes(uint8_t charBuffer[], int count)
{
#ifdef DUE
    uint8_t i(SerialUSB.readBytes(charBuffer, count));
#else
    uint8_t i(Serial.readBytes(charBuffer, count));
#endif
  return i;
}

/* @TODO(rcj) The two functions below may have a better home somewhere else */
// byte 1: bit 1 set indicates that this is the beginning of a message
// byte 1: bit 2 unset indicates that there was an error with the packet
// byte 1: bits 4-7: packet index
// bytes 2-3: don't-cares
// write a bad response to the serial bus
void writeError(uint8_t packetIndex = 0)
{
  uint8_t badResp[5];
  badResp[0] = (8 << 4) + (packetIndex & 7);
  badResp[1] = badResp[0];
  badResp[2] = 0;
  badResp[3] = fletcher8(3, badResp);
  badResp[4] = 0;
  write_bytes(badResp, 5);
}

// Write a good response to the serial bus.
/*void writeGood(uint8_t packetIndex)
{
  write_byte((12 << 4) + (packetIndex & 15));
}*/
