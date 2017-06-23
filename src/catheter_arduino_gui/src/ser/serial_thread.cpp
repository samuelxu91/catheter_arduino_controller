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
#include "catheter_arduino_gui/serial_thread.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <wx/wx.h>
#include <wx/numdlg.h>
#include <string>
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

// ms to wait between consecutive probes
#define PAUSE_INC_MS 1


SerialThreadObject::SerialThreadObject():
  active_(true),
  ss_(new CatheterSerialSender),
  thread_loop_handle_(),
  textStatusData_(NULL),
  statusGridData_(NULL)
{
  // retain a handle to the thread (This way a graceful exit is possible).
  thread_loop_handle_ = boost::thread(boost::bind(&SerialThreadObject::serialLoop, this));
}


void SerialThreadObject::serialLoop()
{
  boost::posix_time::ptime t1(boost::posix_time::second_clock::local_time());
  int64_t delay(0);
  int cmdIndex(0);
  t1 = boost::posix_time::microsec_clock::local_time();
  while (active_)
  {
    if (ss_->connected())
    {
      if (ss_->dataAvailable())
      {
        comStatus newCom(ss_->probePacket());

        printComStat(newCom);

        CatheterChannelCmd incomingData;
        boost::recursive_mutex::scoped_lock lock(threadMutex_);
        // printf("recieved command: ");
        // printComStat(newCom);
        if (newCom == valid)
        {
          ss_->processData(commandFromArd.commandList);
          if (statusGridData_ != NULL)
          {
            statusGridData_->updateCmdList(commandFromArd.commandList);
          }
        }
        else
        {
          // printf("invalid\n");
        }
        lock.unlock();
      }
      // why is there a segmentation fault here?
      // This is a fifo command
      boost::recursive_mutex::scoped_lock lock(threadMutex_);
      if (queued_commands_to_arduino_.size() > 0)
      {
        printf("sending a command\n");
        boost::posix_time::time_duration diff(boost::posix_time::microsec_clock::local_time() - t1);
        int64_t diffAmt(diff.total_nanoseconds());
        if (diffAmt > delay)
        {
          // send the first command
          t1 = boost::posix_time::microsec_clock::local_time();
          boost::recursive_mutex::scoped_lock lock(threadMutex_);
          int writeOut(ss_->sendCommand(queued_commands_to_arduino_[0], cmdIndex));
          printf("Sending command index %d...", cmdIndex);
          printf("Sent %d bytes\n", writeOut);
          delay = queued_commands_to_arduino_[0].delayTime*1000000;
          queued_commands_to_arduino_.erase(queued_commands_to_arduino_.begin());

          cmdIndex++;
          cmdIndex %= 8;
        }
        lock.unlock();
      }
    }
    boost::this_thread::sleep(boost::posix_time::microseconds(1));
  }
}

void SerialThreadObject::setStatusGrid(statusData* newPtr)
{
  statusGridData_ = newPtr;
}

void SerialThreadObject::serialCommand(const ThreadCmd& incomingCommand)
{
  // check for avaiable data:
  if (incomingCommand != noCmd)
  {
    boost::recursive_mutex::scoped_lock looplock(threadMutex_);
    switch (incomingCommand)
    {
    case resetArduino:
    {
      queueCommand(resetCmd(), true);
    }
    break;
    case resetSerial:
    {
      if (ss_->connected())
      {
        ss_->stop();
      }
      if (textStatusData_ != NULL)
      {
        textStatusData_->appendText(std::string("Attempting to reset Arduino Serial Connection"));
      }
      // reset the serial bus.
      std::vector<std::string> ports;
      ss_->getAvailablePorts(ports);
      if (!ports.size())
      {
        if (textStatusData_ != NULL)
        {
          textStatusData_->appendText(std::string("No Serial Ports found."));
        }
      }
      else
      {
        if (ports.size() == 1)
        {
          ss_->setPort(ports[0]);
          if (textStatusData_ != NULL)
          {
            textStatusData_->appendText(std::string("Connecting to Port: ")+ports[0]);
          }
        }
        else
        {
          // have user select the correct port
          for (int i = 0; i < ports.size(); i++)
          {
            wxMessageBox(wxString::Format("Found Serial Port: %s (%d/%d)", wxString(ports[i]), i + 1, ports.size()));
          }
          int which_port(wxGetNumberFromUser(wxEmptyString,
            wxT("Select Serial Port Number"), wxEmptyString, 0, 1, ports.size()) - 1);
          wxMessageBox(wxString::Format("Selected Serial Port: %s", wxString(ports[which_port])));
          ss_->setPort(ports[which_port]);
          if (textStatusData_ != NULL)
          {
            textStatusData_ -> appendText(std::string("Connecting to Port: ")+ports[which_port]);
          }
        }

        ss_->start();
        if (textStatusData_ != NULL && ss_->connected())
        {
          textStatusData_->appendText(std::string("Successfully Connected!!"));
          printf("successfully connected\n");
        }
      }
    }
    break;
    case poll:
      queueCommand(pollCmd(), true);
    break;
    case connect:
      // connect to the arduino
    case disconnect:
      // disconnect from the arduino
    default:
      if (textStatusData_ != NULL)
      {
        textStatusData_->appendText(std::string("Command not recognized"));
      }
    }
    looplock.unlock();
  }
  return;
}


void SerialThreadObject::setStatusTextPtr(incomingText* textPtr)
{
  this->textStatusData_ = textPtr;
}


void SerialThreadObject::stopThreads()
{
  boost::recursive_mutex::scoped_lock lock(threadMutex_);
  active_ = false;
  lock.unlock();
  thread_loop_handle_.join();
  return;
}

SerialThreadObject::~SerialThreadObject()
{
  // lock the mutex
  stopThreads();
  ss_->stop();
  boost::this_thread::sleep(boost::posix_time::milliseconds(300));
  delete ss_;
  return;
}


void SerialThreadObject::flushCommandQueue()
{
  queued_commands_to_arduino_.clear();
}


// status commands
void SerialThreadObject::queueCommand(const CatheterChannelCmdSet &command_to_arduino, bool flush)
{
  // lock the mutex
  // flush the command queue if necessary
  // append the new command
  boost::recursive_mutex::scoped_lock
  lock(threadMutex_);
  // append the new command.
  if (flush)
  {
    flushCommandQueue();
  }
  queued_commands_to_arduino_.push_back(command_to_arduino);
}


void SerialThreadObject::queueCommands(const std::vector< CatheterChannelCmdSet > &commandsToArd, bool flush)
{
  // lock the mutex
  // flush the command queue if necessary
  // append the new commands
  boost::recursive_mutex::scoped_lock
  lock(threadMutex_);
  // append the new command.
  if (flush)
  {
    flushCommandQueue();
  }
  queued_commands_to_arduino_.insert(queued_commands_to_arduino_.end(), commandsToArd.begin(), commandsToArd.end());
}
