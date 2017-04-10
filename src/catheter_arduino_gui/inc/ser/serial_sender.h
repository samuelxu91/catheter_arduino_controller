#pragma once
#ifndef CATHETER_SERIAL_SENDER_H
#define CATHETER_SERIAL_SENDER_H

#include <boost/asio.hpp>

#include <vector>
#include <string>

#include "ser/simple_serial.h"
#include "com/catheter_commands.h"


#define PAD_CMDS false
#define MAX_PAUSE_MS 2000 //ms to wait maximum before declaring timeout, i.e. for sending reset

// This file defines the serial port interface. (mid-level)

class CatheterSerialSender {
private:
	std::string port_name;
	// The serial port pointer of the interface?
	SerialPort *sp;
	std::vector< unsigned char> bytesAvailable;
public:
	CatheterSerialSender();
	~CatheterSerialSender();

	void getAvailablePorts(std::vector<std::string>& ports);
	void setPort(const std::string port);
	std::string getPort();
	bool start(const std::string& port);
	bool start();
	bool stop();
	void serialReset();
	bool resetStop();
	
	bool sendReset();

	bool connected();

	bool dataAvailable();
	comStatus getData(std::vector< CatheterChannelCmd > &);

	int sendCommand(const CatheterChannelCmdSet &, int);
};


std::string comStat2String(const comStatus& );
#endif

