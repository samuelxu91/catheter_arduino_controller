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

#ifndef CATHETER_ARDUINO_GUI_CATHETER_GRID_H
#define CATHETER_ARDUINO_GUI_CATHETER_GRID_H

#include "wx/wx.h"
#include "wx/panel.h"
#include "wx/grid.h"
#include "wx/headerctrl.h"
#include "wx/generic/gridctrl.h"
#include "wx/generic/grideditors.h"
#include <vector>

#include "catheter_arduino_gui/communication_definitions.h"
#include "catheter_arduino_gui/catheter_commands.h"

// This file defines the grid in which commands are entered and run.
// This object does not require any threaded communication as it is only
// changed by the main thread.
class CatheterGrid : public wxGrid
{
    public:
    explicit CatheterGrid(wxPanel* parent);
    ~CatheterGrid();

    void OnGridCellChanging(wxGridEvent& e);

    /**
     * @brief Set command from grid.
     *
     * @param The incomming event.
     */
    void SetCommands(const std::vector<CatheterChannelCmdSet>& cmds);

    /**
     * @brief Get command from grid.
     *
     * @param The incomming event.
     */
    void GetCommands(std::vector<CatheterChannelCmdSet>& cmds);

    /**
     * @brief Reset the grid to default.
     *
     * @param The incomming event.
     */
    void ResetDefault();
    wxDECLARE_EVENT_TABLE();

    private:
    void RecalculateGridSize(int rows);
    void setRowReadOnly(int row, bool readOnly);
    void formatDefaultRow(int row);
    void formatDefaultGrid(int nrows);
    void resetDefaultGrid(int nrows);
    bool isGridRowNumValid(int row);
    bool isGridCellEmpty(int row, int col);
    bool isGridRowComplete(int row);

    int64_t parseGridRow(int row, CatheterChannelCmd& c);

    void addGridRow(bool readOnly);
    void setGridRowChannel(int row, int channel);
    void setGridRowChannel(int row, const wxString& channel);
    void setGridRowcurrentMilliAmp(int row, double currentMilliAmp);
    void setGridRowDirection(int row, dir_t direction);
    void setGridRowDelayMS(int row, int delayMS);
    int getGridRowChannel(int row);
    double getGridRowcurrentMilliAmp(int row);
    dir_t getGridRowDirection(int row);
    int getGridRowDelayMS(int row);

    unsigned int cmdCount;
};

#endif  // CATHETER_ARDUINO_GUI_CATHETER_GRID_H
