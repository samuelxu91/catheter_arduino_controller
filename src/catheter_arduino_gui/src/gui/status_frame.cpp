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
#include "catheter_arduino_gui/status_frame.h"
#include <vector>


StatusGrid:: ~StatusGrid()
{
}


StatusGrid::StatusGrid(wxPanel* parentPanel):
  wxFlexGridSizer(7, 4, 2, 10)
{
  wxStaticText *channelIndex = new wxStaticText(parentPanel, -1, wxT("Channel #"));
  wxStaticText *setMa = new wxStaticText(parentPanel, -1, wxT("Set Current(mA)"));
  wxStaticText *senseMa = new wxStaticText(parentPanel, -1, wxT("Sensed Current(mA)"));
  wxStaticText *enable = new wxStaticText(parentPanel, -1, wxT("Enabled"));

  this->Add(channelIndex);
  this->Add(setMa);
  this->Add(senseMa);
  this->Add(enable);

  textCtrl_.clear();
  for ( int index(0); index < 24; index++)
  {
    textCtrl_.push_back(new wxTextCtrl(parentPanel, -1));
    textCtrl_[index]->SetEditable(false);

    switch (index % 4)
    {
    case 0:  // channel number
      textCtrl_[index]->SetValue(wxString::Format(wxT("%d"), (index  >> 2)+1));
      break;
    case 1:
    case 2:
      textCtrl_[index]->SetValue(wxT("0.00"));
      break;
    case 3:
      textCtrl_[index]->SetValue(wxT("false"));
      break;
    }
    this->Add(textCtrl_[index]);
  }
}


bool StatusGrid::updateStatus(statusData* inputData)
{
  boost::mutex::scoped_lock lock(inputData->statusMutex_);
  if (inputData->updated_)
  {
    int cmdCount(inputData->inputCommands_.size());
    for (int index(0); index < cmdCount; index++)
    {
      int channelNum(inputData->inputCommands_[index].channel);
      int baseIndex(((channelNum - 1) << 2));

      textCtrl_[baseIndex + 1]->SetValue(wxString::Format(wxT("%f"),
        inputData->inputCommands_[index].currentMilliAmp));
      textCtrl_[baseIndex + 2]->SetValue(wxString::Format(wxT("%f"),
        inputData->inputCommands_[index].currentMilliAmp_ADC));

      if (inputData->inputCommands_[index].enable)
      {
        textCtrl_[baseIndex + 3]->SetValue(wxT("True"));
      }
      else
      {
        textCtrl_[baseIndex + 3]->SetValue(wxT("False"));
      }
    }
    inputData->updated_ = false;
    return true;
  }
  return false;
}


void statusData::updateCmdList(std::vector<CatheterChannelCmd> &inputCommands)
{
  boost::mutex::scoped_lock lock(this->statusMutex_);
  this->inputCommands_ = inputCommands;
  this->updated_ = true;
}
