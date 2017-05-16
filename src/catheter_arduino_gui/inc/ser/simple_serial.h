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


#ifndef CATHETER_ARDUINO_GUI_SIMPLE_SERIAL_H
#define CATHETER_ARDUINO_GUI_SIMPLE_SERIAL_H

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <string>
#include <vector>
#include <time.h>

// This file defines the low level serial interface.

/**
 * @brief a type definition for boost pointers to the serial port
 */
typedef boost::shared_ptr<boost::asio::serial_port> serial_port_ptr;


#define SERIAL_PORT_READ_BUF_SIZE 512

/**
 * @brief The serial port object which is used to send data back and forth between the arduino and the PC.
 */
class SerialPort
{
private:
  /**
   * @brief the io_service provides the framework for serial communication
   */
  boost::asio::io_service io_service_;


  /**
   * @brief a pointer to the boost serial port
   */
  serial_port_ptr port_;


  /**
   * @brief a mutex used for thread safe port accessing.
   */
  boost::mutex mutex_;


  /**
   * @brief the input data buffer.
   */
  uint8_t read_buf_bytes_raw_[SERIAL_PORT_READ_BUF_SIZE];


  /**
   * @brief the read buf raw (may be redundant).
   */
  unsigned char read_buf_raw_[SERIAL_PORT_READ_BUF_SIZE];


  /**
   * @brief the vector of input data buffer.
   */
  std::vector<uint8_t> read_buf_bytes_;


  /**
   * @brief the vector of input data buffer. (also may be redundant)
   */
  // std::vector<unsigned char> read_buf_str_;


  /**
   * @brief prevent autodeclaration of the copy constructor.
   */ 
  SerialPort(const SerialPort &p);


  /**
   * @brief prevent autodeclaration of the assignment operator.
   */
  SerialPort &operator=(const SerialPort &p);


  /**
   * @brief the time of sending data.
   */
  clock_t tSend_;


  /**
   * @brief the time of receiving the data
   */
  clock_t tRecieve_;


  /**
   * @brief the serial port thread.
   */
  boost::thread t_;


public:
  /**
   * @brief The enumerated list of serial bus speeds
   */
  enum Baud
  {
    BR_9600 = 9600,
    BR_19200 = 19200,
    BR_115200 = 115200
  };

  /**
   * @brief the default constructor
   */
  SerialPort(void);

  /**
   * @brief The default destructor
   */
  ~SerialPort(void);

  // @TODO(rcj33) proper identify and comment the functional purpose.
  char end_of_line_char() const;
  void end_of_line_char(const char &c);

  virtual bool start(const char *com_port_name, Baud baud_rate = BR_9600);
  virtual void stop();

  // int write_some(const std::string &buf);
  // int write_some(const char *buf, const int &size);
  int write_some_bytes(const std::vector<uint8_t> &buf);

  bool get_port_name(const unsigned int &idx, std::string& port_name);
  std::vector<std::string> get_port_names();

  bool isOpen();


  // std::vector<unsigned char> flushData();
  std::vector<uint8_t> flushData();

  bool hasData();

protected:
  // virtual void async_read_some_();
  // virtual void on_receive_(const boost::system::error_code& ec, size_t bytes_transferred);
  // virtual void on_receive_(const std::string &data);
  virtual void async_read_some_bytes_();
  virtual void on_receive_bytes_(const boost::system::error_code& ec, size_t bytes_transferred);
};

#endif  // CATHETER_ARDUINO_GUI_SIMPLE_SERIAL_H
