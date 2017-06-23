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

#include "catheter_arduino_gui/simple_serial.h"
#include <stdio.h>
#include <iostream>
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


SerialPort::SerialPort(void):
  port_(NULL)
{
}


SerialPort::~SerialPort(void)
{
  // delete the smart pointer nicely.
  std::auto_ptr< boost::asio::io_service::work >
    work(new boost::asio::io_service::work(io_service_));
  work.reset();
  t_.join();
  return;
}


bool SerialPort::start(const char *com_port_name, Baud baud_rate)
{
  boost::system::error_code ec;
  if (port_ == NULL)
  {
    port_ = serial_port_ptr(new boost::asio::serial_port(io_service_));
  }
  else
  {
    if (port_->is_open())
    {
      std::cout << "error : port is already opened..." << std::endl;
      return true;
    }
  }
  port_->open(com_port_name, ec);
  if (ec)
  {
    std::cout << "error : port_->open() failed...com_port_name="
      << com_port_name << ", e=" << ec.message().c_str() << std::endl;
    return false;
  }
  ec.clear();

  // option settings...
  unsigned int baud = static_cast<int>(baud_rate);
  port_->set_option(boost::asio::serial_port_base::baud_rate(baud), ec);
  port_->set_option(boost::asio::serial_port_base::character_size(8), ec);
  port_->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one), ec);
  port_->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none), ec);
  port_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none), ec);

  // this thread may need to be joined during destructor...
  t_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

  async_read_some_bytes_();

  return true;
}


void SerialPort::stop()
{
  boost::mutex::scoped_lock look(mutex_);
  boost::system::error_code ec;
  if (port_ != NULL)
  {
    port_->cancel(ec);
    // from the documentation, I understand that close() internally makes a call to cancel()
    // so only the close() call is needed. Note: when the option is available, should always
    // use the version of the method that accepts an error_code argument so that the program
    // will not throw a runtime exception.
    port_->close(ec);
    // reset() is not a member of the serial_port class.
    port_.reset();
  }
  io_service_.stop();
  io_service_.reset();
}

bool SerialPort::get_port_name(const unsigned int &idx, std::string& port_name)
{
  std::vector<std::string> ports = get_port_names();
  if (idx < ports.size())
  {
    port_name.clear();
    port_name = ports[idx];
    return true;
  }
  return false;
}

std::vector<std::string> SerialPort::get_port_names()
{
  std::vector<std::string> ports;
  boost::system::error_code ec;
#ifdef _WINDOWS
  for (int i = 0; i < 64; i++)
  {
    char p[7] = "COM";
    char n[3];
    snprintf(n, sizeof(n), "%d", i);

    if (port_ != NULL)
    {
      if (start(std::strncat(p, n, sizeof(p) + sizeof(n)), BR_9600))
      {
        ports.push_back(std::string(p));
      }
    }
    else
    {
      if (port_->is_open())
      {
        port_->close();
      }
      port_->open(std::strncat(p, n, sizeof(p) + sizeof(n)), ec);
      if (!ec)
      {
        ports.push_back(std::string(p));
      }
    }
  }
  if (port_->is_open())
  {
    port_->close();
  }
#else
  std::string path = "/dev/tty";
  FILE *pipe = popen("ls /dev/tty* | egrep -o \"(ACM|USB)[0-9]\" | tr -d '\n'", "r");
  if (!pipe) return ports;
  char buf[8];
  while (!feof(pipe))
  {
    if (fgets(buf, 8, pipe) != NULL)
    {
      ports.push_back(path + buf);
    }
  }
  pclose(pipe);
#endif
  stop();
  return ports;
}

bool SerialPort::isOpen()
{
  if (port_ != NULL)
  {
    return port_->is_open();
  }
  return false;
}


int SerialPort::write_some_bytes(const std::vector<uint8_t> &buf)
{
  int write_size(buf.size());
  boost::system::error_code ec;
  if (!port_) return -1;
  if (write_size == 0) return 0;
  tSend_ = clock();
  int writeStatus(port_->write_some(boost::asio::buffer(buf, write_size), ec));
  std::string errorM(ec.message());
  printf("error msg: %s \n", errorM.c_str());
  return writeStatus;
}

void SerialPort::async_read_some_bytes_()
{
  if (port_.get() == NULL || !port_->is_open())
  {
    printf("invalid port");
    return;
  }
  boost::system::error_code ec;
  port_->async_read_some(
    boost::asio::buffer(read_buf_bytes_raw_, SERIAL_PORT_READ_BUF_SIZE),
    boost::bind(
    &SerialPort::on_receive_bytes_,
    this, ec,
    boost::asio::placeholders::bytes_transferred));
  ec.message();
}

void SerialPort::on_receive_bytes_(const boost::system::error_code& ec, size_t bytes_transferred)
{
  tRecieve_ = clock();
  clock_t dt = tRecieve_ - tSend_;
  // printf("It took me %d clicks (%f seconds). to reply.\n", static_cast<int> (dt), ((float)dt) / CLOCKS_PER_SEC);
  if (bytes_transferred > 0)
  {
    printf("Recieved %d bytes.\n", static_cast<int> (bytes_transferred));
  }
  if (port_.get() == NULL || !port_->is_open())
  {
    printf("Null or closed port");
    return;
  }
  if (ec)
  {
    printf("read byte error");
    ec.message();
    async_read_some_bytes_();
    return;
  }

  {
    boost::mutex::scoped_lock look(mutex_);
    for (unsigned int i = 0; i < bytes_transferred; i++)
    {
      uint8_t b = read_buf_bytes_raw_[i];
      read_buf_bytes_.push_back(b);
    }
  }

  async_read_some_bytes_();
}

std::vector<uint8_t> SerialPort::flushData()
{
  boost::mutex::scoped_lock look(mutex_);
  std::vector<uint8_t> outputVec;
  for (int i = 0; i < read_buf_bytes_.size(); i++)
  {
    outputVec.push_back(read_buf_bytes_[i]);
  }

  read_buf_bytes_.clear();
  return outputVec;
}



bool SerialPort::hasData()
{
  boost::mutex::scoped_lock look(mutex_);
  if (read_buf_bytes_.size() > 0) return true;
  return false;
}
