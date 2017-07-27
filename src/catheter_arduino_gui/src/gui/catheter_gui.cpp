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
#include "catheter_arduino_gui/catheter_gui.h"
#include "catheter_arduino_gui/pc_utils.h"
#include "catheter_arduino_gui/serial_thread.h"

#include <wx/wfstream.h>
#include <wx/numdlg.h>
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

// file definitions
#define playfile_wildcard wxT("*.play")

#define CATHETER_GUI_DEBUG 1
#define DBG(do_something) if (CATHETER_GUI_DEBUG) { do_something; }


IMPLEMENT_APP(CatheterGuiApp)


wxBEGIN_EVENT_TABLE(CatheterGuiFrame, wxFrame)
    EVT_BUTTON(CatheterGuiFrame::ID_REFRESH_SERIAL_BUTTON, CatheterGuiFrame::OnRefreshSerialButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SELECT_PLAYFILE_BUTTON, CatheterGuiFrame::OnSelectPlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_NEW_PLAYFILE_BUTTON, CatheterGuiFrame::OnNewPlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SAVE_PLAYFILE_BUTTON, CatheterGuiFrame::OnSavePlayfileButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SEND_COMMANDS_BUTTON, CatheterGuiFrame::OnSendCommandsButtonClicked)
    EVT_BUTTON(CatheterGuiFrame::ID_SEND_RESET_BUTTON, CatheterGuiFrame::OnSendResetButtonClicked)
  EVT_BUTTON(CatheterGuiFrame::ID_SEND_POLL_BUTTON, CatheterGuiFrame::OnSendPollButtonClicked)
  EVT_IDLE(CatheterGuiFrame::onIdle)
wxEND_EVENT_TABLE()


bool CatheterGuiApp::OnInit()
{
  // debug memory:
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

  serialObject_ = new SerialThreadObject;
  gui_ = new CatheterGuiFrame(wxT("Catheter Gui"), serialObject_);
  gui_->Show(true);
  return (gui_ != NULL);
}


int CatheterGuiApp::OnExit()
{
  delete serialObject_;
  return 0;
}


void CatheterGuiFrame::onIdle(wxIdleEvent & e)
{
  bool update(false);
  update |= this->statusGridPtr_->updateStatus(statusGridCmdPtr_);
  update |= this->statusText_->addText(statusTextData_);
  if (update)
  {
    this->Refresh();
  }
  e.RequestMore();
}


CatheterGuiFrame::CatheterGuiFrame(const wxString& title, SerialThreadObject* thrdPtr):
wxFrame(NULL, wxID_ANY, title)
{
  this->serialObject_ = thrdPtr;
  parentPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);

  grid_ = new CatheterGrid(parentPanel_);

  // add the status grid_:
  statusGridPtr_ = new StatusGrid(parentPanel_);
  statusGridCmdPtr_ = new statusData;

  // add the status text.
  statusText_ = new CatheterStatusText(parentPanel_, wxID_ANY);

  statusTextData_ = new incomingText;
  serialObject_->setStatusTextPtr(statusTextData_);
  serialObject_->setStatusGrid(statusGridCmdPtr_);
  // control buttons (break this up into 2 rows)

  // row 1:
  selectPlayfileButton_ = new wxButton(parentPanel_, ID_SELECT_PLAYFILE_BUTTON, wxT("Open Playfile"));
  newPlayfileButton_ = new wxButton(parentPanel_, ID_NEW_PLAYFILE_BUTTON, wxT("New Playfile"));
  savePlayfileButton_ = new wxButton(parentPanel_, ID_SAVE_PLAYFILE_BUTTON, wxT("Save Playfile"));
  pollButton_ = new wxButton(parentPanel_, ID_SEND_POLL_BUTTON, wxT("Poll Arduino"));

  // row 2
  sendCommandsButton_ = new wxButton(parentPanel_, ID_SEND_COMMANDS_BUTTON, wxT("Send Commands"));
  sendResetButton_ = new wxButton(parentPanel_, ID_SEND_RESET_BUTTON, wxT("Send Reset"));
  refreshSerialButton_ = new wxButton(parentPanel_, ID_REFRESH_SERIAL_BUTTON, wxT("Refresh Serial"));

  playfileSaved_ = false;
  playfilePath_ = wxEmptyString;

  // add buttons to the frame
  wxFlexGridSizer* buttonBox = new wxFlexGridSizer(2, 4, wxSize(2, 2));
  buttonBox->Add(selectPlayfileButton_);
  buttonBox->Add(newPlayfileButton_);
  buttonBox->Add(savePlayfileButton_);
  buttonBox->Add(pollButton_);
  buttonBox->Add(sendCommandsButton_);
  buttonBox->Add(sendResetButton_);
  buttonBox->Add(refreshSerialButton_);

  // Add the different boxes to the grid_.
  // This box is the top level one.
  wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
  vbox->Add(buttonBox, 0, wxALL, 5);

  // add the status Grid
  wxStaticBoxSizer *statusBox = new wxStaticBoxSizer(wxHORIZONTAL, parentPanel_, wxT("Status Information"));

  statusBox->Add(statusGridPtr_);
  vbox->Add(statusBox, 1, wxALL, 5);

  wxStaticBoxSizer *consoleBox = new wxStaticBoxSizer(wxVERTICAL, parentPanel_, wxT("Console Information"));
  consoleBox->Add(statusText_, 1, wxEXPAND | wxALL, 5);
  vbox->Add(consoleBox, 1, wxEXPAND | wxALL, 5);

  // wxPanel *fillPanel = new wxPanel(parentPanel_);

  // removed the wxExpand flag
  vbox->Add(grid_, 1, wxALL, 5);
  parentPanel_->SetSizer(vbox);
  vbox->SetSizeHints(parentPanel_);
  vbox->Fit(parentPanel_);

  this->Fit();
  this->Center();

  setStatusText(wxT("Welcome to Catheter Gui"));
}


CatheterGuiFrame::~CatheterGuiFrame()
{
  // do nothing here
  // All windows should be auto cleaned.
  delete statusGridCmdPtr_;
  delete statusTextData_;
}


//////////////////////////////////
// public event handler methods //
//////////////////////////////////
void  CatheterGuiFrame::OnSelectPlayfileButtonClicked(wxCommandEvent& e)
{
  warnSavePlayfile();
}



void CatheterGuiFrame::OnNewPlayfileButtonClicked(wxCommandEvent& e)
{
  warnSavePlayfileforNewPlayfile();

  grid_->ResetDefault();

  playfileSaved_ = false;
  playfilePath_ = wxEmptyString;

  setStatusText(wxT("Editing New Playfile\n"));
}


void CatheterGuiFrame::OnSavePlayfileButtonClicked(wxCommandEvent& e)
{
  savePlayfile();
  if (playfileSaved_)
  {
    // save contents of edit panel to playfilePath_
    savePlayfilesecond(playfilePath_);
  }
}


void CatheterGuiFrame::OnSendCommandsButtonClicked(wxCommandEvent& e)
{
  if (sendGridCommands())
  {
    setStatusText(wxT("Commands Successfully Sent"));
  }
  else
  {
    setStatusText(wxT("Error Sending Commands"));
  }
}


void CatheterGuiFrame::OnSendPollButtonClicked(wxCommandEvent& e)
{
  sendPollCommand();
  setStatusText(wxT("Poll Command Successfully Sent"));
}


void CatheterGuiFrame::OnSendResetButtonClicked(wxCommandEvent& e)
{
  if (sendResetCommand())
  {
    setStatusText(wxT("Reset Command Successfully Sent"));
  }
  else
  {
    setStatusText(wxT("Error Sending Reset Command"));
  }
}


void CatheterGuiFrame::OnRefreshSerialButtonClicked(wxCommandEvent& e)
{
  if (!refreshSerialConnection())
  {
    setStatusText(wxString::Format("Serial Disconnected"));
  }
}


//////////////////////////////////
// status panel private methods //
//////////////////////////////////
void CatheterGuiFrame::setStatusText(const wxString& msg)
{
  // append the new message to the current status message
  // statusText_->SetLabel(wxString::Format("%s\n%s", statusText_->GetLabel(), msg));
  // statusText_->SetLabel(msg);
  statusText_->addWxText(msg);
}


///////////////////////////////////
// control panel private methods //
///////////////////////////////////
wxString CatheterGuiFrame::openPlayfile()
{
  wxString path = wxEmptyString;
  wxFileDialog openDialog(this, "Open Playfile", wxGetCwd(), "", playfile_wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (openDialog.ShowModal() != wxID_CANCEL)
  {
    wxFileInputStream playfile_stream(openDialog.GetPath());
    if (playfile_stream.IsOk())
    {
      path = openDialog.GetPath();
    }
    else
    {
      wxLogError("Selected file could not be opened.");
    }
  }
  return path;
}


void CatheterGuiFrame::savePlayfile()
{
  wxString path = wxEmptyString;
  wxFileDialog saveDialog(this, wxT("Save Playfile"), wxGetCwd(),
  " ", "*.play", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if (saveDialog.ShowModal() != wxID_CANCEL)
  {
    wxString name = saveDialog.GetPath();
    wxString check = name.Right(5);
    if (check.Matches(".play"))
    {
      wxFileOutputStream save_stream(name);
      setStatusText(wxString::Format(wxT("Saved Playfile as %s"), name));
    }
    else
    {
      name = name.Append(".play");
      setStatusText(wxString::Format(wxT("Saved Playfile as %s"), name));
    }
     wxFileOutputStream save_stream(name);
       if (!save_stream.IsOk())
       {
        wxLogError("Could not save to selected file");
       }
       else
       {
        path = name;
       }
  }
  if (!path.IsEmpty())
       {
        playfilePath_ = path;
        playfileSaved_ = true;
       }
}


void CatheterGuiFrame::loadPlayfile(const wxString& path)
{
  std::vector<CatheterChannelCmdSet> gridCmds;
  loadPlayFile(path.mb_str(), gridCmds);
  grid_->SetCommands(gridCmds);
}


void CatheterGuiFrame::savePlayfilesecond(const wxString& path)
{
  std::vector<CatheterChannelCmdSet> gridCmds;
  grid_->GetCommands(gridCmds);
  writePlayFile(path.mb_str(), gridCmds);
}


void CatheterGuiFrame::warnSavePlayfile()
{
  if (!playfileSaved_)
  {
    int answer = wxMessageBox(wxT("Do you want to save the current playfile before proceeding?"), wxT("Warning!"),
      wxICON_QUESTION | wxCANCEL | wxYES_NO, this);
    if (answer == wxYES)
    {
      savePlayfile();
      return;
    }
    if (answer == wxNO)
    {
       wxString path = openPlayfile();
         if (!path.IsEmpty())
           grid_->ResetDefault();
           playfileSaved_ = false;
           playfilePath_ = path;
           loadPlayfile(playfilePath_);
    }
         else
          return;
  }
}

void CatheterGuiFrame::warnSavePlayfileforNewPlayfile()
{
  if (!playfileSaved_)
  {
    int answer = wxMessageBox(wxT("Do you want to save the current playfile before proceeding?"), wxT("Warning!"),
      wxICON_QUESTION | wxCANCEL | wxYES_NO, this);
    if (answer == wxYES)
    {
      savePlayfile();
      if (playfileSaved_)
      {
        // save contents of edit panel to playfilePath_
        savePlayfilesecond(playfilePath_);
  }
      return;
    }
  }
}


// @TODO: The functionality of this block is very different now.
bool CatheterGuiFrame::sendCommands(const std::vector<CatheterChannelCmdSet> &cmdVect)
{
  if (cmdVect.size())
  {
    serialObject_->queueCommands(cmdVect);
    return true;
  }
  return false;
}


bool CatheterGuiFrame::sendGridCommands()
{
  std::vector<CatheterChannelCmdSet> cmds;
  grid_->GetCommands(cmds);
  setStatusText(wxString::Format("Parsed %d Channel Commands\n", static_cast<int>(cmds.size())));
  setStatusText(wxString::Format("Parsed %d Channel Commands\n", static_cast<int>(cmds.size())));
  return sendCommands(cmds);
}


// resets the catheter.
bool CatheterGuiFrame::sendResetCommand()
{
  setStatusText(wxT("Sending Global Reset Command...\n"));
  bool reset(true);
  // send a reset to the arduino
  serialObject_->serialCommand(SerialThreadObject::ThreadCmd::resetArduino);
  return reset;
}


bool CatheterGuiFrame::sendPollCommand()
{
  setStatusText(wxT("Sending Global Poll Command...\n"));
  serialObject_->serialCommand(SerialThreadObject::ThreadCmd::poll);
  return true;
}


// @TODO: The functionality of this code will be dramatically different.
bool CatheterGuiFrame::refreshSerialConnection()
{
  serialObject_->serialCommand(SerialThreadObject::ThreadCmd::resetSerial);
  return true;
}


bool CatheterGuiFrame::closeSerialConnection()
{
  // return ss->resetStop();
  return false;
}


void CatheterGuiFrame::wxSummarizeCmds(const std::vector<CatheterChannelCmd> &cmds)
{
  if (!(cmds.size() % NCHANNELS))
  {
    for (int i = 0; i < (cmds.size() / NCHANNELS); i++)
    {
      wxString allchannels = wxEmptyString;
      for (int j = 0; j < NCHANNELS; j++)
      {
        allchannels = allchannels + wxToString(cmds[(i * NCHANNELS) + j]) + wxT("\n");
      }
      wxMessageBox(allchannels);
    }
  }
  else
  {
    for (int i = 0; i < cmds.size(); i++)
      wxMessageBox(wxToString(cmds[i]));
  }
}


wxString CatheterGuiFrame::wxToString(const CatheterChannelCmd &cmd)
{
  return wxString::Format("channel: %d\ncurrent: %3.3f\n", cmd.channel, cmd.currentMilliAmp);
}


