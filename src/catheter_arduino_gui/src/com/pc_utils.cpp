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

#include "catheter_arduino_gui/catheter_commands.h"
#include <algorithm>
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



/* parse a playfile into a command vector */
int loadPlayFile(const char* fileIn, std::vector<CatheterChannelCmdSet>& outputCmdsVect)
{
  std::ifstream inFile(fileIn, std::ifstream::in);

  if (inFile.bad()) return -1;
  if (!inFile.is_open()) return -2;

  outputCmdsVect.clear();
  int npackets = 0;
  std::string line;
  int linenum = 0;
  CatheterChannelCmdSet singleSet;

  while (getline (inFile, line))
  {
      std::string item;
      // line comment
      size_t posOcto1 = line.find("#");
      // after channel, before current
      size_t posComma1 = line.find(",");
      // after current (MilliAmp), before delay (MilliSec)
      size_t posComma2 = line.find(",", posComma1 + 1);

      // verify the line's validity.
      if ((posComma1 < posComma2) && posComma2 != std::string::npos)
      {
        // line is ok.
        std::istringstream linestream(line);
        CatheterChannelCmd singleCmd;

        /* parse channel */
        getline(linestream, item, ',');
        int channelIn = atoi(item.c_str());
        // std::cout<<channelIn<<std::endl;
        // bad channel; skip line
        if (!(channelIn >= 0 && channelIn <= NCHANNELS)) continue;
        singleCmd.channel = channelIn;

        /* parse current data, given in MilliAamp */
        getline(linestream, item, ',');
        singleCmd.currentMilliAmp = atof(item.c_str());

        singleCmd.poll = false;

        /* parse delay */
        // Delay is delimitered by new line so using the default delimiter.
        getline(linestream, item);
        int waitTime = atoi(item.c_str());
        // bad delay; skip line
        if (!(waitTime >= 0)) continue;

        singleSet.commandList.push_back(singleCmd);

        if (waitTime > 0)
        {
          singleSet.delayTime = waitTime;
          outputCmdsVect.push_back(singleSet);
          singleSet.commandList.clear();
          singleSet.delayTime = 0;
        }

        linenum++;
      }
  }
  inFile.close();
  return npackets;
}


int currentGen(const std::vector<CatheterChannelCmdSet>& cmdVect, std::vector<double>& timeSlice,
  std::vector<std::vector<double>>& currentList, int actuatorDofs, int numActuator)
{
  // clearing the input vectors
  timeSlice.clear();
  currentList.clear();

  // Initial the time slice vector with 0 as start point
  timeSlice.push_back(0);

  // Define the current list size
  int currentSize = (actuatorDofs * numActuator) / cmdVect[0].commandList.size();

  // Take the current list size as a group to combine single current value
  for (int i = 0; i < cmdVect.size(); i += currentSize)
  {
    std::vector<double> currentSet;
    for (int j = 0; j < currentSize; j++)
    {
      for (int k = 0; k < cmdVect[i+j].commandList.size(); k++)
        currentSet.push_back(cmdVect[i+j].commandList[k].currentMilliAmp);
    }
    currentList.push_back(currentSet);
  }

  // For each command in the command vector
  for (auto& obj : cmdVect)
  {
    // add the new delay time the latest time ticker in the vector
    timeSlice.push_back(timeSlice.back()+obj.delayTime);
  }
  return 0;
}


std::vector<double> publishCurrent(double timeTicker, const std::vector<double>& timeSlice,
  const std::vector<std::vector<double>>& currentList)
{
  if (timeTicker < timeSlice[0]) return currentList[0];
  else if (timeTicker > timeSlice.back()) return currentList.back();
  // Find the upper bound given the input time ticker and return the related position
  int itrPos = (std::lower_bound(timeSlice.begin(), timeSlice.end(), timeTicker))-timeSlice.begin()-1;
  // since the time slice and the current vector should mapped one-to-one, the corresponding current should be returned
  return currentList[itrPos];
}


bool writePlayFile(const char * fname, const std::vector<CatheterChannelCmdSet>& cmdVect)
{
  std::ofstream outFile(fname, std::ofstream::out);
  if (outFile.bad())
      return false;
  CatheterChannelCmd cmd;
  for (int i = 0; i < cmdVect.size(); i++)
  {
    for ( int j(0); j < cmdVect[i].commandList.size(); j++)
    {
      cmd = cmdVect[i].commandList[j];
      outFile << cmd.channel << ", " << cmd.currentMilliAmp << ", ";
      if ( (j+1) < cmdVect[i].commandList.size())
      {
        outFile << 0 << std::endl;
      }
      else
      {
        outFile << cmdVect[i].delayTime << std::endl;
      }
    }
  }
  outFile.close();
  return true;
}


bool writeBytes(const char *fname, const std::vector<uint8_t>& bytes)
{
  FILE* f = fopen(fname, "w");
  if (f == NULL)
    return false;
  for (int i = 0; i < bytes.size(); i++)
  {
    fprintf(f, "%u\n", bytes[i]);
  }
  fclose(f);
  return true;
}
