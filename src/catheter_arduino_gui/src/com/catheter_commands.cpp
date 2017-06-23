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
#include "catheter_arduino_gui/catheter_commands.h"
#include "catheter_arduino_gui/digital_analog_conversions.h"
#include <vector>

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
  #ifndef DBG_NEW
    #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
    #define new DBG_NEW
  #endif
#endif  // _DEBUG
#endif  // __MSC_VER


/* calculate 8-bit fletcher checksum using blocksize=4 */
// This is for error correction.
uint8_t fletcher8(int len, const std::vector<uint8_t> &data)
{
  if (len <= data.size())
  {
    // The 8-bit fletcher checksum is using blocksize of 4.
    uint8_t sum1(0), sum2(0);
    for (int i(0); i < len; i++)
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
  return 0;
}


CatheterChannelCmdSet resetCmd()
{
  CatheterChannelCmdSet resetCmdSet;
  resetCmdSet.commandList.push_back(CatheterChannelCmd());
  return resetCmdSet;
}


CatheterChannelCmd emptyCommand()
{
  CatheterChannelCmd cmd;
  cmd.channel = 0;
  cmd.currentMilliAmp = 0;
  cmd.currentMilliAmp_ADC = 0;
  cmd.poll = false;
  return cmd;
}


std::vector<uint8_t> encodeCommandSet(const CatheterChannelCmdSet& cmds, int pseqnum)
{
  std::vector<uint8_t> encodedSet;
  std::vector<uint8_t> tempEncoding;

  encodedSet.clear();

  int n(cmds.commandList.size());

  tempEncoding = encodePreamble(pseqnum, n);


  encodedSet.insert(encodedSet.end(), tempEncoding.begin(), tempEncoding.end());

  tempEncoding.clear();

  for (int ind(0); ind < n; ind++)
  {
    tempEncoding = encodeSingleCommand(cmds.commandList[ind]);
    encodedSet.insert(encodedSet.end(), tempEncoding.begin(), tempEncoding.end());
    tempEncoding.clear();
  }

  tempEncoding = encodePostamble(pseqnum);
  encodedSet.insert(encodedSet.end(), tempEncoding.begin(), tempEncoding.end());
  uint8_t chksum = fletcher8(encodedSet.size(), encodedSet);
  encodedSet.insert(encodedSet.end(), 1, chksum);
  // encode the preamble.
  return encodedSet;
}


std::vector<uint8_t> encodePreamble(int pseqnum, int ncmds)
{
  std::vector<uint8_t> bytes;
  int i;
  for (i = 0; i < PRE_LEN; i++)
  {
    if (i == 0)
    {
      bytes.push_back(PCK_OK << 7);          /* ok1 */
      bytes[i] |= (pseqnum & 7) << 4;    /* index3 */
      bytes[i] |= (ncmds & 15);           /* cmdCnt4 */
    }
  }
  return bytes;
}


std::vector<uint8_t> encodeSingleCommand(const CatheterChannelCmd& cmd)
{
  std::vector<uint8_t> bytes;


  uint8_t encodedByte = 0;
  if (cmd.poll)   encodedByte |= (1 << POL_BIT);
  if (cmd.enable)     encodedByte |= (1 << ENA_BIT);
  // always update.
  encodedByte |= (1 << UPD_BIT);
  // if (cmd.update)   encodedByte |= (1 << UPD_BIT);

  encodedByte |= (cmd.currentMilliAmp > 0.0) ? (DIR_POS << DIR_BIT) : (DIR_NEG << DIR_BIT);

  // bit 1-4 is the channel number
  // bit 5-8 is the encoded channel command information.
  uint8_t firstByte(cmd.channel << 4 | (encodedByte & 15));

  uint16_t dacSetting(milliAmp2Dac(cmd.currentMilliAmp));

  for (int i(0); i < CMD_LEN; i++)
  {
    if (i == 0)
    {
      // bits 1-4
      bytes.push_back(firstByte);
    }
    else if (i == 1)
    {
      // bits 8-15  (first 6 bits of DAC data (in the lower 6) (modified to match the arduin0
      bytes.push_back((dacSetting >> 6) & 63);
    }
    else if (i == 2)
    {
      // bits 16-23 (last 6 bits of DAC data)
      bytes.push_back((dacSetting & 63));
    }
  }
  return bytes;
}


std::vector<uint8_t> encodePostamble(int pseqnum)
{
  std::vector<uint8_t> bytes;
  int i;
  for (i = 0; i < POST_LEN; i++)
  {
    if (i == 0)
    {
      bytes.push_back(pseqnum << 5);  // index3
      bytes[i] |= PCK_OK & 1;     // packet OK bit appended to beginning and end of packet
    }
  }
  return bytes;
}


CatheterChannelCmd parseSingleCommand(const std::vector<uint8_t>& cmdBytes, int & index)
{
  CatheterChannelCmd result;
  // byte 1
  result.channel = cmdBytes[index] >> 4;
  // expandCmdVal(, &poll, &en, &update, &dir);
  result.poll = (cmdBytes[index] >> POL_BIT) & 1;
  result.enable = (cmdBytes[index] >> ENA_BIT) & 1;
  result.update = (cmdBytes[index] >> UPD_BIT) & 1;

  result.dir = ((cmdBytes[index] >> DIR_BIT) & 1) ? DIR_POS : DIR_NEG;


  // bytes 2 and 3 (last 4 bits reserved)
  // pull off the DAC value:
  uint16_t cmdData(((uint16_t)(cmdBytes[index + 1]) << 6) + (cmdBytes[index + 2] % 64));

  // convert the dac value to a double.
  result.currentMilliAmp = dac2MilliAmp(cmdData, result.dir);

  index += 3;
  // If the Poll bit is true, pull off the adc value
  if (result.poll)
  {
    result.poll = true;
    uint16_t adcd1(static_cast<uint16_t> (cmdBytes[index]));
    if ((adcd1 >> 6) == 1)
    {
      result.currentMilliAmp_ADC = -500.0;
    }
    else
    {
      uint16_t adcd1a(adcd1 << 6);
      uint16_t adcd2(static_cast<uint16_t> (cmdBytes[index+1]));
      uint16_t adcData(adcd1a+adcd2);
    //  convert adc bits to a double.
    result.currentMilliAmp_ADC = adc2MilliAmp(adcData);
    }
    index += 2;
  }
  return result;
}


// generate, populate, and return a reset command
CatheterChannelCmdSet resetCommand()
{
  CatheterChannelCmd cmd;
  cmd.channel = 0;
  cmd.currentMilliAmp = 0;
  cmd.currentMilliAmp_ADC = 0;
  cmd.poll = false;
  CatheterChannelCmdSet resetCmdSet;
  resetCmdSet.commandList.resize(1, cmd);
  resetCmdSet.delayTime = 0;
  return resetCmdSet;
}


void printData(const std::vector< uint8_t >& bytesRead)
{
  printf("Input bytes:\n");
  for (int i(0); i < bytesRead.size(); i++)
  {
    printf("%d ", static_cast<int> (bytesRead[i]));
  }
  printf("\n");
}


int parseBytes2Cmds(int byteCount, std::vector<uint8_t>& bytesRead, std::vector<CatheterChannelCmd>& cmds)
{
  int byteIndex(3);
  while (byteIndex + 1 < byteCount)
  {
    cmds.push_back(parseSingleCommand(bytesRead, byteIndex));
  }
  bytesRead.erase(bytesRead.begin(), bytesRead.begin() + byteCount);
  return 1;
}


int estResponseSize(const CatheterChannelCmdSet &commands)
{
  int respSize(4);
  for (int i(0); i < commands.commandList.size(); i++)
  {
    int chanCount(1);
    if (commands.commandList[i].channel == 0)
      chanCount = 6;

    if (commands.commandList[i].poll)  chanCount *= 5;
    else  chanCount *= 3;

    respSize += chanCount;
  }
  return respSize;
}


int parseFirstSecondByte(const std::vector < uint8_t > &inputBytes)
{
  // verify that the first two bytes are equal:
  if (inputBytes[0] != inputBytes[1])
  {
    return -1;
  }

  uint8_t ok_1((inputBytes[0] >> 7) % 2);
  uint8_t ok_2((inputBytes[0] >> 6) % 2);

  bool ok(ok_1 & ok_2);

  if (ok)
  {
    int index_((inputBytes[0]) % 8);
    return index_;
  }
  else return -1;
}


int parseThirdByte(const std::vector < uint8_t > &inputBytes)
{
  uint8_t cmdCount(inputBytes[2] >> 4);
  uint8_t pollCount(inputBytes[2] & 15);

  int totalSize = static_cast<int> (cmdCount)*3 + static_cast<int> (pollCount)*2 + 4;
  return totalSize;
}


bool checkFletcher(int length, const std::vector<uint8_t> &bytePacket)
{
  uint8_t checksum(fletcher8(length-1, bytePacket));

  if (bytePacket[length-1] == checksum) return true;
  else
  {
    printf("%d, %d\n", checksum, bytePacket[length-1]);
    return false;
  }
}


CatheterChannelCmdSet pollCmd()
{
  CatheterChannelCmdSet pollCmdSet;
  pollCmdSet.commandList.push_back(CatheterChannelCmd());
  pollCmdSet.commandList[0].poll = true;
  pollCmdSet.commandList[0].channel = 0;
  return pollCmdSet;
}
