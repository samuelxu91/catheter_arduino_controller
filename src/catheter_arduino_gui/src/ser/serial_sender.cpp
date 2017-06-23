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
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "catheter_arduino_gui/pc_utils.h"
#include "catheter_arduino_gui/serial_sender.h"
#include "catheter_arduino_gui/simple_serial.h"

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


CatheterSerialSender::CatheterSerialSender():
commandHistory_(),
dataChange_(false),
responseLength_(-1)
{
  port_name = "";
  sp = new SerialPort();
}


CatheterSerialSender::~CatheterSerialSender()
{
  // sp->stop();
  delete sp;
}


void CatheterSerialSender::getAvailablePorts(std::vector<std::string>& ports)
{
  ports.clear();
  ports = sp->get_port_names();
}


void CatheterSerialSender::setPort(const std::string port)
{
  this->port_name = port;
}


std::string CatheterSerialSender::getPort()
{
  return port_name;
}


// design plan:
// don't want to keep calling get_port_names() every time the serial port needs to be re-opened;
// should do this once when CatheterSerialSender is initialized, and again when the user requests
// the serial connection to be refreshed. Instead of a port_name field, keep a vector<string> of
// discovered ports, as well as a port_id (default 0) to index into ports.
bool CatheterSerialSender::start()
{
  commandHistory_.clear();
  if (!sp->isOpen())
  {
    if (port_name.empty())
    {
      std::vector<std::string> ports = sp->get_port_names();
      if (!ports.size())
      {
        return false;
      }
      port_name = ports[0];
    }
    return sp->start(port_name.c_str());
  }
  else
  {
    return true;
  }
}


bool CatheterSerialSender::stop()
{
  commandHistory_.clear();
  sp->stop();
  return true;
}


void CatheterSerialSender::serialReset()
{
  if (sp->isOpen())
  {
    sp->flushData();
    boost::this_thread::sleep(boost::posix_time::milliseconds(300));
    sp->stop();
    boost::this_thread::sleep(boost::posix_time::milliseconds(300));
  }
  start();
}


bool CatheterSerialSender::resetStop()
{
  return stop();
}


bool CatheterSerialSender::dataAvailable()
{
  if (sp->hasData())
  {
    std::vector<uint8_t> temp = sp->flushData();
    bytesAvailable.insert(bytesAvailable.end(), temp.begin(), temp.end());
    this->dataChange_ = true;
  }
  return this->dataChange_;
}


int CatheterSerialSender::processData(std::vector<CatheterChannelCmd> &cmd)
{
  cmd.clear();
  this->dataChange_ = true;
  return parseBytes2Cmds(this->responseLength_, bytesAvailable, cmd);
}


bool CatheterSerialSender::connected()
{
  if (!sp->isOpen())
  {
    return false;
  }
  else return true;
}


int CatheterSerialSender::sendCommand(const CatheterChannelCmdSet & outgoingData, int pseqnum)
{
  // parse the command:
  std::vector< uint8_t > bytesOut(encodeCommandSet(outgoingData, pseqnum));
  int expectedResponse(estResponseSize(outgoingData));
  commandHistory_.push_back(commandInformation(pseqnum, expectedResponse));
  if (connected())
  {
    // send it through the serial port:
    printf("connected and sending\n");
    return static_cast<int> (sp->write_some_bytes(bytesOut));
  }
  else
  {
    printf("not connected");
    return 0;
  }
}


comStatus CatheterSerialSender::probePacket()
{
  this->dataChange_ = false;
  // probe the current incoming packet to see if it is a valid response with a matching
  // preamble index.
  printData(bytesAvailable);
  if (bytesAvailable.size() > 1)
  {
    int index(parseFirstSecondByte(bytesAvailable));
    printf("%d, %d\n", bytesAvailable[0], bytesAvailable[1]);
    printf("Index: <%d> \n ", index);
    if (index < 0)
    {
      return returnAndWipe();
    }
    else
    {
      if (bytesAvailable.size() > 2)
      {
        int length(parseThirdByte(bytesAvailable));
        printf("Size compare: <%d, %d>\n", static_cast<int> (bytesAvailable.size()), length);
        if (length > bytesAvailable.size())
        {
           printf("incomplete: <%d, %d>\n", static_cast<int> (bytesAvailable.size()), length);
          return incomplete;
        }
        else
        {
          bool fletcherVal(checkFletcher(length, bytesAvailable));
          if (fletcherVal)
          {
            this->responseLength_ = length;
            return valid;
          }
          else
          {
            printf("failed fletcher\n");
            return returnAndWipe();
          }
        }
      }
      else
      {
        return incomplete;
      }
    }
  }
  return none;
}

comStatus CatheterSerialSender::returnAndWipe()
{
  this->bytesAvailable.erase(bytesAvailable.begin());
  this->responseLength_ = -1;
  this->dataChange_ = true;
  return invalid;
}

void printComStat(const comStatus& statIn)
{
  switch (statIn)
  {
  case invalid:
    printf("Invalid\n");
    break;
  case valid:
    printf("valid\n");
    break;
  case none:
    printf("none\n");
    break;
  case incomplete:
    printf("incomplete\n");
    break;
  default:
    printf("unknown status\n");
  }
}


