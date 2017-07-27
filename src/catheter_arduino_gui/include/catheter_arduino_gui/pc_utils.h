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

// The functions here seem important but another poorly scoped header file.
#ifndef CATHETER_ARDUINO_GUI_PC_UTILS_H
#define CATHETER_ARDUINO_GUI_PC_UTILS_H

#include "catheter_arduino_gui/catheter_commands.h"
#include <vector>
#include <string>

/** 
 * @brief int loadPlayFile(const char*,std::vector<CatheterChannelCmd> &): 
 * Loads and parses a playfile.

 * The playfile is a recorded vector of computed commands with inter command delay. 
 */
int loadPlayFile(const char * fname, std::vector<CatheterChannelCmdSet>& cmdVect);

/**
 * @brief Given the command list, generating a list of fixed-size group of 
 *        current based on the actuator. 
 *        dofs and the number of the actuator. 
 *        also generating a list of time slice. 
 *
 * @param const std::vector<CatheterChannelCmdSet>& cmdVect:   
 *        the input vector of Catheter command set
 * @param       std::vector<double>& timeSlice:                
 *              the time slices generated from the command set
 * @param       std::vector<std::vector<double>>& currentList: 
 *              the current wrapped in the group of given current size
 * @param       int actuatorDofs:                              
 *              the degree of freedom for Catheter actuators
 * @param       int numActuator:                               
 *                the number of actuators for Catheter
 * @return      int:                                           
 *                the return status, successful as 0
 */
int currentGen(const std::vector<CatheterChannelCmdSet>& cmdVect, std::vector<double>& timeSlice,
  std::vector< std::vector<double> >& currentList, int actuatorDofs, int numActuator);

/**
 * @brief Given the list of grouped current and the input time ticker, 
 *        this function will return the grouped current associated the time ticker
 * @param        double timeTicker:                             
 *                the input time ticker
 * @param  const std::vector<double>& timeSlice:                
 *               the vector of time slices
 * @param  const std::vector<std::vector<double>>& currentList: 
 *               the vector of grouped currents
 * @return       std::vector<double>:                           
 *                the answer of the time ticker associated
 */
std::vector<double> publishCurrent(double timeTicker, const std::vector<double>& timeSlice,
  const std::vector< std::vector<double> >& currentList);


bool writePlayFile(const char * fname, const std::vector<CatheterChannelCmdSet>& cmdVect);

/**
 * @brief Use the user input information to generate playfile and save it. 
 *
 * @param      
 * 
 */
bool writeBytes(const char* fname, const std::vector<uint8_t>& bytes);

/** @brief void summarizeCmd(const CatheterChannelCmd& cmd): summarizes a command reply.
 * This should not be used with gui's 
 */
void summarizeCmd(const CatheterChannelCmd& cmd);

void print_string_as_bits(int len, std::string s);

void print_bytes_as_bits(int len, std::vector<uint8_t> bytes);

#endif  // CATHETER_ARDUINO_GUI_PC_UTILS_H
