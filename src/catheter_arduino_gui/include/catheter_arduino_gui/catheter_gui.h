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


#ifndef CATHETER_ARDUINO_GUI_CATHETER_GUI_H
#define CATHETER_ARDUINO_GUI_CATHETER_GUI_H


#include <wx/wx.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "catheter_arduino_gui/catheter_grid.h"
#include "catheter_arduino_gui/status_text.h"
#include "catheter_arduino_gui/status_frame.h"
#include "catheter_arduino_gui/serial_thread.h"
#include <vector>
#include <wx/grid.h>


// This file defines the gui layout

class CatheterGuiFrame : public wxFrame
{
public:
    /**
     * @brief The catheter gui frame constructor for starting the frame up.
     *
     * This requires a pointer to the serialthread object for communicating with the arduino.
     * @param title string of the the frame.
     * @param a pointer to the serialthread object.
     */
    explicit CatheterGuiFrame(const wxString& title, SerialThreadObject *);

    /**
     * @brief The GUI frame destructor
     */
    ~CatheterGuiFrame();

    /**
     * @brief Refreshes the serial communication bus.
     *
     * @param The incomming event.
     */
    void OnRefreshSerialButtonClicked(wxCommandEvent& e);

    /**
     * @brief Loads a new play file.
     *
     * @param The incomming event.
     */
    void OnSelectPlayfileButtonClicked(wxCommandEvent& e);

    /**
     * @brief Opens a new play file in directory.
     *
     * @param The incomming event.
     */
    void OnNewPlayfileButtonClicked(wxCommandEvent& e);

    /**
     * @brief Saves a playfile and open a new empty playfile.
     *
     * @param The incoming event
     */
    void OnSavePlayfileButtonClicked(wxCommandEvent& e);

    /**
     * @brief Sends commands to the arduino. Save playfile in the directory.
     *
     * @param The incoming event
     */
    void OnSendCommandsButtonClicked(wxCommandEvent& e);

    /**
     * @brief Sends a reset command.
     *
     * @param The incoming event
     */
    void OnSendResetButtonClicked(wxCommandEvent& e);

    /**
     * @brief Pings the arduino for full system status.
     *
     * @param The incoming event
     */
    void OnSendPollButtonClicked(wxCommandEvent& e);

    /**
     * @brief Idle refresh function
     *
     * @param The incoming event
     */
    void onIdle(wxIdleEvent& e);

    /**
     * @brief Enumerated list of event ID numbers.
     *
     * These events start at 1024 and increment by 1.
     */
    enum
    {
        ID_SELECT_PLAYFILE_BUTTON = 1024,
        ID_NEW_PLAYFILE_BUTTON,
        ID_SAVE_PLAYFILE_BUTTON,
        ID_SEND_COMMANDS_BUTTON,
        ID_SEND_RESET_BUTTON,
        ID_REFRESH_SERIAL_BUTTON,
        ID_SEND_POLL_BUTTON
    };


    /**
     * @brief Declares the event table for the GUI
     */
    wxDECLARE_EVENT_TABLE();

private:
    /**
     * @brief set the status text
     *
     * @param the incoming string to append to the status text.
     */
    void setStatusText(const wxString& msg);

    /**
     * @brief Warn users to save the current file when they click on open playfile button. Save the new play file. 
     */
    void warnSavePlayfile();

    /**
     * @brief Warn users to save the current file when they click on new playfile button. Save the new play file.  
     */
    void warnSavePlayfileforNewPlayfile();

    /**
     * @brief save Playfile in the directory with checking and adding correct filename. 
     *
     * @return a wxString path to the play file.
     */
    void savePlayfile();

    /**
     * @brief load a playfile based on the input path.
     *
     * @param the wxString path to the play file.
     */
    wxString openPlayfile();

    /**
     * @brief load the selected playfile into the grid.
     * 
     * @TODO(rcj33) identify the purpose of this play file and if removal is necessary.
     */
    void loadPlayfile(const wxString& path);

    /**
     * @brief Save the current playfile with the correct file extension. 
     * 
     * @TODO(rcj33) identify the purpose of this play file and if removal is necessary.
     */
    void savePlayfilesecond(const wxString& path);

    /**
     * @brief Queue up commands to send to the arduino.
     */

    bool sendCommands(const std::vector<CatheterChannelCmdSet> &cmdVect);

    /**
     * @brief Get the grid commands and send them.
     *
     * @return true/false based on success.
     */
    bool sendGridCommands();

    /**
     * @brief Sends the reset commands to the arduino.
     *
     * @return true/false based on success.
     */
    bool sendResetCommand();

    /**
     * @brief Sends the Poll command to the arduino
     *
     * @return true/false based on success.
     */
    bool sendPollCommand();

    /**
     * @brief Refreshes the Serial connection.
     *
     * @return true/false based on success.
     */
    bool refreshSerialConnection();

    /**
     * @brief closes the serial connection.
     *
     * @return true/false based on success.
     */
    bool closeSerialConnection();

    /**
     * @brief convert a catheter channel command to a wxString object.
     *
     * @param a single channel catheter command.
     */
    wxString wxToString(const CatheterChannelCmd &cmd);

    /**
     * @brief summarize the commands into a text object.
     *
     * @param the vector of catheter commands.
     */
    void wxSummarizeCmds(const std::vector<CatheterChannelCmd> &cmds);

    /**
     * @brief The main panel that holds all of the elements
     */
    wxPanel* parentPanel_;


    /**
     * @brief The catheter grid object (pointer).
     */
    CatheterGrid* grid_;


    /**
     * @brief The catheter status text object (pointer).
     */
    CatheterStatusText *statusText_;

    /**
     * @brief The incoming status Text.
     */
    incomingText* statusTextData_;

    /**
     * @brief The status Data  pointer for commands.
     */
    statusData * statusGridCmdPtr_;

    /**
     * @brief a pointer to the status Grid
     */
    StatusGrid * statusGridPtr_;

    /**
     * @brief pointer to the select playfile button
     */
    wxButton* selectPlayfileButton_;

    /**
     * @brief pointer to the new playfile button
     */
    wxButton* newPlayfileButton_;

    /**
     * @brief pointer to the save playfile button
     */
    wxButton* savePlayfileButton_;

    /**
     * @brief pointer to the send commands button
     */
    wxButton* sendCommandsButton_;

    /**
     * @brief pointer to the send reset button.
     */
    wxButton* sendResetButton_;

    /**
     * @brief pointer to the poll button
     */
    wxButton* pollButton_;

    /**
     * @brief pointer to the refresh serial button
     */
    wxButton* refreshSerialButton_;

    /**
     * @brief playfile saved status.
     */
    bool playfileSaved_;

    /**
     * @brief path to the play file
     */
    wxString playfilePath_;

    /**
     * @brief Pointer to the serial thread object
     */
    SerialThreadObject *serialObject_;
};


/**
 * @brief The catheter GUI application object
 *
 * This object defines the entrypoint of the gui.
 */
class CatheterGuiApp : public wxApp
{
public:
    /**
     * @brief App entry point.
     *
     * @return bool to indicate success.
     */
    bool OnInit();

    /**
     * @brief App exit point.
     *
     * @return int to indicate exit code
     */
    int OnExit();

private:
    /**
     * @brief a pointer to the frame object.
     */
    CatheterGuiFrame* gui_;

    /**
     * @brief a pointer to the frame object.
     */
    SerialThreadObject *serialObject_;
};


#endif  // CATHETER_ARDUINO_GUI_CATHETER_GUI_H
