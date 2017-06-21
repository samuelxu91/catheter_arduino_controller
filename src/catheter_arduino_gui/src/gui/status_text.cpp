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
#include "catheter_arduino_gui/status_text.h"
#include <string>

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


void incomingText::appendText(const std::string  &newText)
{
  boost::mutex::scoped_lock lock(textMutex);
  this->update = true;
  stringData += "\n";
  stringData += newText;
}


CatheterStatusText::CatheterStatusText(wxWindow* parent, wxWindowID id):
  wxScrolledWindow(parent, id, wxDefaultPosition, wxSize(50, 150))
{
  this -> SetBackgroundColour(*wxBLACK);
  this -> SetForegroundColour(*wxYELLOW);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  statusText = new wxStaticText(this, wxID_ANY, wxEmptyString);
  sizer->Add(statusText, 0, wxALL, 5);

  this->SetSizer(sizer);
  this->FitInside();
  this->SetScrollRate(5, 5);
}


bool CatheterStatusText::addText(incomingText *incomingInfo)
{
  boost::mutex::scoped_lock lock(incomingInfo->textMutex);
  if (incomingInfo->update)
  {
    SetCatheterStatusText(wxString(incomingInfo->stringData));
    incomingInfo->stringData.clear();
    incomingInfo->update = false;
    return true;
  }
  incomingInfo->update = false;
  return false;
}

bool CatheterStatusText::addWxText(const wxString& msg)
{
  boost::mutex::scoped_lock lock(textMutex);
  SetCatheterStatusText(msg);
  return true;
}

void CatheterStatusText::SetCatheterStatusText(const wxString& msg)
{
  wxString current = statusText->GetLabelText();
  current = current + wxT("\n") + msg;
  statusText->SetLabelText(current);

  this->FitInside();
  int r(this->GetScrollRange(wxVERTICAL));
  this->Scroll(0, r);
}
