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

#pragma once

#ifndef CATHETER_ARDUINO_GUI_CATHETER_COMMANDS_H
#define CATHETER_ARDUINO_GUI_CATHETER_COMMANDS_H

#include "catheter_arduino_gui/communication_definitions.h"

#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>


/**
 * @brief This enumerated type is meant to list the parsing results of an incoming command.
 */
enum comStatus
{
  incomplete = -3, invalid = -1, valid = 0, none = -2, noPrompt = 1
};


/**
 * @brief This is the catheter command structure.
 *
 * It contains a channel, poll information, an enable flag,
 * And update flag, and both a DAC and ADC setting.
 */
struct CatheterChannelCmd
{
  int channel;
  bool poll;
  bool enable;
  bool update;
  dir_t dir;
  double currentMilliAmp;
  double currentMilliAmp_ADC;

  // default constructor:
  CatheterChannelCmd() : channel(0), poll(false), currentMilliAmp(0), currentMilliAmp_ADC(0)
  {}
};


/**
 * @brief This struct contains a list of commands
 * 
 * This list is meant to be sent to the arduino as a single command group
 * the time delay is meant for after the command is sent.
 */
struct CatheterChannelCmdSet
{
  std::vector < CatheterChannelCmd > commandList;
  int64_t delayTime;

  CatheterChannelCmdSet() : delayTime(0), commandList()
  {}
};


/**
 * @brief Create a global reset command
 *
 * @returns A command set that can be sent to the arduino as a reset.
 */
CatheterChannelCmdSet resetCmd();


/**
 * @brief Create a global poll command
 *
 * The command polls the arduino to obtain its current state.
 * 
 * @returns A command that can be used to poll the arduino
 */
CatheterChannelCmdSet pollCmd();


/**
 * @brief generates a byte stream from a catheter command set for sending to the arduino.
 * 
 * @param The set of catheter commands to set.
 * @param The index assigned to this catheter commands.
 * 
 * @returns std::vector<uint8_t> The byte list.
 */
std::vector<uint8_t> encodeCommandSet(const CatheterChannelCmdSet&, int pseqnum);


/**
 * @brief encode the preamble bytes.
 * 
 * The preamble encodes the number of commands as well as the sequence number into it.
 * @param The index of this command.
 * @param The number of commands being sent over.
 */
std::vector<uint8_t> encodePreamble(int pseqnum, int ncmds);


/**
 * @brief encode the postamble. 
 * 
 * @param the index of the command being encoded.
 */
std::vector<uint8_t> encodePostamble(int pseqnum);


/** 
 * \brief std::vector<uint8_t> encodeSingleCommand(const catheterChannelCmd):
 * compacts a single arduino command into a 3 byte packet.
 * 
 * This function is used in the encode command set.
 */
std::vector<uint8_t> encodeSingleCommand(const CatheterChannelCmd& cmd);


/** 
 * @brief  compute the 8-bit blocksize 4 fletcher checksum for a given command list.
 * 
 * compute the fletcher checksum of an array of bytes of length 'len' using blocksize=8.
 * ('len' <= the actual length of the array, since we may not want to include all elements
 * of the array in the computation.) 
 *
 * @param the length of the command set
 * @param The constant vector of command bytes
 * @return a uint8_t byte representing the checksum.
 */
uint8_t fletcher8(int len, const std::vector<uint8_t> &);


/**
 * @brief validates and processes returned bytes for a packet from the arduino.
 *
 * This function validates the length, checksum, and matching indecis of the index number.
 * This function also populates a vector of Catheter Channel Cmds to represent the response.
 * 
 * @param The incoming byte packet
 * @param The CatheterChannelCmd vector list.
 * @returns the channel data parsed from the return values (or -1 on error)
 */
int parseBytes2Cmds(int byteCount, std::vector<uint8_t>& reply, std::vector<CatheterChannelCmd>& cmds);


/**
 * @brief  Parses the preamble of an incoming byte list to validate it. parsePreamble(const std::vector < uint8_t > &)
 * 
 * parses the incoming data from the arduino in order to validate that the command packet was valid.
 *
 * @param the incoming byte list.
 * @returns the integer value of success.
 */
// int parsePreamble(const std::vector < uint8_t > &, int &);

bool checkFletcher(int length, const std::vector<uint8_t> &bytePacket);


int parseFirstSecondByte(const std::vector < uint8_t > &inputBytes);


int parseThirdByte(const std::vector < uint8_t > &inputBytes);

int estResponseSize(const CatheterChannelCmdSet &);

void printData(const std::vector< uint8_t >& bytesRead);

#endif  // CATHETER_ARDUINO_GUI_CATHETER_COMMANDS_H
