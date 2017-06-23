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


#ifndef CATHETER_ARDUINO_GUI_SERIAL_THREAD_H
#define CATHETER_ARDUINO_GUI_SERIAL_THREAD_H

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "catheter_arduino_gui/catheter_commands.h"
#include "catheter_arduino_gui/serial_sender.h"
#include "catheter_arduino_gui/status_text.h"
#include "catheter_arduino_gui/status_frame.h"

#include <vector>

// This class acts a thread manager for offloading the serial communication. (high-level)
// Prevents gui hangs.
class SerialThreadObject
{
public:
  /**
   * @brief this is the default constructor.
   */
  SerialThreadObject();


  /**
   * @brief assign the pointer to the text console object.
   */
  void setStatusTextPtr(incomingText* statuTextPtr);


  /**
   * @brief assign the pointer to the status grid object.
   */
  void setStatusGrid(statusData* statusDataPtr);


  /**
   * @brief enumerated commands to the thread object
   */
  enum ThreadCmd
  {
    noCmd = 0, resetArduino = -2, resetSerial = -1, poll  = 1, connect, disconnect
  };


  /**
   * @brief enumerated status of the thread object
   */
  enum ThreadStatus
  {
    ready, error
  };


  /**
   * @brief: thread object destructor
   */
  ~SerialThreadObject();


  /**
   * @brief: queue a vector of commands to pipe to the arduino.
   *
   * @param a vector of commands
   * @param an option to reset the current list of commands.
   */
  void queueCommands(const std::vector< CatheterChannelCmdSet > &, bool = false);


  /**
   * @brief: queue a single command to pipe to the arduino.
   *
   * @param a single command
   * @param an option to reset the current list of commands.
   */
  void queueCommand(const CatheterChannelCmdSet &, bool = false);


  /*
   * @brief: This sends a command to the  loop thread
   */
  void serialCommand(const ThreadCmd&);

private:
  /**
   * @brief Stop the serial processing thread
   */
  void stopThreads();

  /**
   * @brief flush the command Queue.
   *
   * This command is NOT thread safe but is only called from within mutex locked code blocks
   */
  void flushCommandQueue();

  /**
   * @brief the incoming thread command
   */
  ThreadCmd incomingCommand_;

  /**
   * @brief The current status of the thread
   */
  ThreadStatus currentStatus_;

  /**
   * The thread Mutex
   */
  boost::recursive_mutex threadMutex_;

  /**
   * @brief Handle to the thread which the gui communicates with to conduct serial IO.
   */
  boost::thread thread_loop_handle_;

  /**
   * @brief The serial thread loop function.
   */
  void serialLoop();

  /**
   * @brief The active status of the thread?
   * @TODO(rcj33) identify if this is used
   */
  bool active_;

  /**
   * @brief Pointer to the serial sender object
   */
  CatheterSerialSender* ss_;


  /**
   * @breif queued data to send to the arduino.
   */
  std::vector< CatheterChannelCmdSet > queued_commands_to_arduino_;

  /**
   * @brief The immeidiate reply from the arduino
   */
  CatheterChannelCmdSet commandFromArd;

  /**
   * @brief handle to the  GUI status grid.
   * 
   * This is used for populating data to and from the arduino. (i.e. current measured and commanded)
   */
  statusData * statusGridData_;

  /**
   * @brief handle to the  GUI console status.
   * 
   * This is used for text based feedback on sending command to
   * and receiving commands from the arduino.
   */
  incomingText* textStatusData_;
};


#endif  // CATHETER_ARDUINO_GUI_SERIAL_THREAD_H
