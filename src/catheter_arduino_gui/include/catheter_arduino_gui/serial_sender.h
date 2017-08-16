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
#ifndef CATHETER_ARDUINO_GUI_SERIAL_SENDER_H
#define CATHETER_ARDUINO_GUI_SERIAL_SENDER_H

#include <boost/asio.hpp>

#include <vector>
#include <string>
#include <serial/serial.h>

#include "catheter_arduino_gui/catheter_commands.h"


#define PAD_CMDS false
// ms to wait maximum before declaring timeout, i.e. for sending reset
#define MAX_PAUSE_MS 2000

// This file defines the serial port interface. (mid-level)
void printComStat(const comStatus& statIn);

struct commandInformation
{
  uint8_t commandIndex_;
  uint32_t expectedResponse_;

  commandInformation(uint8_t commandIndex, uint32_t expectedResponse):
  commandIndex_(commandIndex),
  expectedResponse_(expectedResponse)
  {}
};



class CatheterSerialSender
{
private:
  std::string port_name;
  // The serial port pointer of the interface?
  serial::Serial *sp;
  std::vector< unsigned char> bytesAvailable;
std::vector<commandInformation> commandHistory_;

  bool dataChange_;

  int responseLength_;

  comStatus returnAndWipe();

public:
  CatheterSerialSender();
  ~CatheterSerialSender();

  void getAvailablePorts(std::vector<std::string>& ports);
  void setPort(const std::string port);
  std::string getPort();
  bool start(const serial::PortInfo &port);
  bool stop();
  void serialReset();
  bool resetStop();

  bool sendReset();

  bool connected();

  bool dataAvailable();
  int processData(std::vector< CatheterChannelCmd > &);

  int sendCommand(const CatheterChannelCmdSet &, int);


  /**
   * @brief probe the packet to see if a valid response is available.
   *
   * 
   */
  comStatus probePacket();
};


std::string comStat2String(const comStatus&);


#endif  // CATHETER_ARDUINO_GUI_SERIAL_SENDER_H
