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


#ifndef CATHETER_ARDUINO_GUI_STATUS_TEXT_H
#define CATHETER_ARDUINO_GUI_STATUS_TEXT_H
#include <wx/wx.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <string>

// This file defines the status text box.

struct incomingText
{
  boost::mutex textMutex;
  std::string stringData;
  bool update;

  incomingText() : update(false)
  {
  };

  void appendText(const std::string  &newText);
};

class CatheterStatusText : public wxScrolledWindow
{
public:
  // explicit constructor
  explicit CatheterStatusText(wxWindow* parent, wxWindowID id);

  // public method to send text into the status box
  bool addText(incomingText* newText);
  bool addWxText(const wxString& msg);

private:
  wxStaticText *statusText;

  // the mutex is used to pass information forward
  boost::mutex textMutex;

  // appends text and scrolls down.
  void SetCatheterStatusText(const wxString& msg);
};

#endif  // CATHETER_ARDUINO_GUI_STATUS_TEXT_H
