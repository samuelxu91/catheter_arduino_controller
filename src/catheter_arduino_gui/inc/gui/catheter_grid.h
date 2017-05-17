#ifndef CATHETER_GUI_CATHETER_GRID_H
#define CATHETER_GUI_CATHETER_GRID_H

#include "wx/wx.h"
#include "wx/panel.h"
#include "wx/grid.h"
#include "wx/headerctrl.h"
#include "wx/generic/gridctrl.h"
#include "wx/generic/grideditors.h"
#include <vector>

#include "com/communication_definitions.h"
#include "com/catheter_commands.h"

// This file defines the grid in which commands are entered and run.
// This object does not require any threaded communication as it is only
// changed by the main thread.
class CatheterGrid : public wxGrid
{
    public:
    explicit CatheterGrid(wxPanel* parent);
    ~CatheterGrid();

    void OnGridCellChanging(wxGridEvent& e);

    void SetCommands(const std::vector<CatheterChannelCmdSet>& cmds);
    void GetCommands(std::vector<CatheterChannelCmdSet>& cmds);
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

#endif  // CATHETER_GUI_CATHETER_GRID_H
