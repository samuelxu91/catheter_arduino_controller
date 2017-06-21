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

#ifndef CATHETER_ARDUINO_GUI_STATUS_FRAME_H
#define CATHETER_ARDUINO_GUI_STATUS_FRAME_H


#include <wx/wx.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "catheter_arduino_gui/catheter_commands.h"
#include <vector>

// This files defines the display class which shows the current status of individual
// catheter channels. (ADC and DAC)
// This is the status frame.

// a shared mutex object is included to avoid race conditions.

/**
 * @brief the status data associated with the status grid
 */
struct statusData
{
  /**
   * @brief local mutex object for thread locking
   */
  boost::mutex statusMutex_;

  /**
   * @brief Input command list
   */
  std::vector<CatheterChannelCmd> inputCommands_;

  /**
   * @brief the updated status of the list
   */
  bool updated_;

  /**
   * @brief update the command list from a command vector
   *
   * @param The vector of channel commands
   */
  void updateCmdList(std::vector<CatheterChannelCmd> & inputCommands);

  /**
   * @brief default constructor of the statusData.
   */
  statusData() : updated_(false)
  {};
};


class StatusGrid: public wxFlexGridSizer
{
public:
  /**
   * @brief Construct the status grid using a parent panel
   *
   * @param The parent Panel.
   */
  explicit StatusGrid(wxPanel* parent);

  /**
   * @brief statusGrid destructor
   */
  ~StatusGrid();

  /**
   * @brief update the local status data 
   */
  bool updateStatus(statusData* dataPtr);

private:
  /**
   * @brief vector of text controls for wx control.
   */
  std::vector < wxTextCtrl* > textCtrl_;
};

#endif  // CATHETER_ARDUINO_GUI_STATUS_FRAME_H
