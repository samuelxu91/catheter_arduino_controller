#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "com/pc_utils.h"
#include "ser/serial_sender.h"
#include "ser/simple_serial.h"

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

CatheterSerialSender::CatheterSerialSender()
{
	port_name = "";
	sp = new SerialPort();
}

CatheterSerialSender::~CatheterSerialSender()
{
	// sp->stop();
	delete sp;
}

void CatheterSerialSender::getAvailablePorts(std::vector<std::string>& ports) {
	ports.clear();
	ports = sp->get_port_names();
}

void CatheterSerialSender::setPort(const std::string port) {
	this->port_name = port;
}

std::string CatheterSerialSender::getPort() {
	return port_name;
}

// design plan: 
// don't want to keep calling get_port_names() every time the serial port needs to be re-opened;
// should do this once when CatheterSerialSender is initialized, and again when the user requests
// the serial connection to be refreshed. Instead of a port_name field, keep a vector<string> of 
// discovered ports, as well as a port_id (default 0) to index into ports.
bool CatheterSerialSender::start() {
	if (!sp->isOpen()) {
		if (port_name.empty()) {
			std::vector<std::string> ports = sp->get_port_names();
			if (!ports.size()) {
				return false;
			}
			port_name = ports[0];
		}		
		return sp->start(port_name.c_str());
	} else {
		return true;
	}
}

// try not to use this method because it is unneccessary and 
// will probably be removed in the future, since one can
// accomplish the same task from outside the class by calling
// ss->setPort(port); ss->start();
bool CatheterSerialSender::start(const std::string& port) {
	if (!sp->isOpen()) {
		port_name = port;
		return start();
	}
	else {
		return true;
	}
}

bool CatheterSerialSender::stop() {
	sp->stop();
	return true;
}

void CatheterSerialSender::serialReset() {
	if (sp->isOpen()) {
		sp->flushData();
		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
		sp->stop();
		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	}
	start();
}

bool CatheterSerialSender::resetStop() {
	return stop();
}

bool CatheterSerialSender::dataAvailable()
{
	std::vector<unsigned char> temp = sp->flushData();
	bytesAvailable.insert(bytesAvailable.end(), temp.begin(), temp.end());
	if(bytesAvailable.size() > 0) return true;
	else return false;
}

comStatus CatheterSerialSender::getData(std::vector<CatheterChannelCmd> &cmd)
{
	cmd.clear();
	return parseBytes2Cmds(bytesAvailable, cmd);
}


bool CatheterSerialSender::connected()
{
	if (!sp->isOpen())
	{
		return false;
	}
	else return true;
}

int CatheterSerialSender::sendCommand(const CatheterChannelCmdSet & outgoingData, int pseqnum)
{
	// parse the command:
	std::vector< uint8_t > bytesOut(encodeCommandSet(outgoingData, pseqnum));
	if (connected())
	{
		// send it through the serial port:
		return static_cast<int> (sp->write_some_bytes(bytesOut, bytesOut.size()));
	}
	else
	{
		printf("not connected");
		return 0;
	}
}

std::string comStat2String(const comStatus& statIn)
{
	switch(statIn)
	{
	case invalid:
		return std::string("Invalid");
		break;
	case valid:
		return std::string("valid");
		break;
	case none:
		return std::string("none");
		break;
	default:
		return std::string("unknown status");
	}
}