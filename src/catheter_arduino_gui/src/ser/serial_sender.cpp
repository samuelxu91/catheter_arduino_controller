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

CatheterSerialSender::CatheterSerialSender() {
	port_name = "";
	sp = new SerialPort();
}

CatheterSerialSender::~CatheterSerialSender() {
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



/*bool CatheterSerialSender::runPlayfile(const std::vector<CatheterChannelCmd>& cmds, std::vector<CatheterChannelCmd>& cmdsReturned) {
	cmdsReturned.clear();
	
	if (!sp->isOpen()) {
		start();
	}

	if (!sendReset()) return false;
	
	std::vector<std::vector<uint8_t>> cmdBytes;
	std::vector<int> delays;	
	std::vector<int> ncmdsPerPacket;
	// temporary (per packet) storage:
	std::vector<unsigned char> bytesRead;	
	std::vector<CatheterChannelCmd> parsedCmds;
	// for debugging:
	std::vector<unsigned char> allBytesSent;
	std::vector<unsigned char> allBytesRead;
	// for serial timing:
	unsigned int max_pause = MAX_PAUSE_MS;
	unsigned int expectedBytes = 0;
	unsigned int initial_delay_ms = 100;
	unsigned int delay_ms = 0;
	unsigned int npolled = 0;
	bool packets_ok = true;

	getPacketBytes(pseqnum, cmds, cmdBytes, delays, ncmdsPerPacket);
	if (!ncmdsPerPacket.size()) return false;
	pseqnum = ((pseqnum + (ncmdsPerPacket.size() - 1)) & 7) + 1;

	unsigned int cmd_counter = 0;
	for (int i = 0; i < cmdBytes.size(); i++) {
		bytesRead.clear();
		parsedCmds.clear();
		delay_ms = 0;		
		npolled = 0;
		for (int j = 0; j < ncmdsPerPacket[i]; j++) 
			if (cmds[cmd_counter + j].poll) npolled++;
		expectedBytes = RESPONSE_LEN(ncmdsPerPacket[i], (cmds[cmd_counter].channel == GLOBAL_ADDR), npolled);
		max_pause = (delays[i] > MAX_PAUSE_MS ? delays[i] : MAX_PAUSE_MS);
		cmd_counter += ncmdsPerPacket[i];

		//sp->flushData();
		boost::this_thread::sleep(boost::posix_time::milliseconds(initial_delay_ms));
		max_pause -= initial_delay_ms;

		// send packet bytes. bytesRead has been stripped of terminating characters.
		bool sent(sendBytes(cmdBytes[i], bytesRead, expectedBytes, &delay_ms, max_pause));
		packets_ok = packets_ok && sent;
		// verify checksum
		unsigned int f8 = fletcher8(bytesRead.size() - PCK_CHK_LEN, bytesRead.data());
		packets_ok = packets_ok && (f8 == bytesRead[bytesRead.size() - 1]);
		// parse received bytes into channel commands
		if (packets_ok) {
			packets_ok = packets_ok && parseBytes2Cmds_NEW(cmdBytes[i], bytesRead, parsedCmds);
			cmdsReturned.insert(cmdsReturned.end(), parsedCmds.begin(), parsedCmds.end());
		}
		// debugging
		allBytesSent.insert(allBytesSent.end(), cmdBytes[i].begin(), cmdBytes[i].end());
		allBytesRead.insert(allBytesRead.end(), bytesRead.begin(), bytesRead.end());
		
		if (delay_ms < delays[i]) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(delays[i] - delay_ms));
		}
	}

	sendReset();
	
	// validate that the current data returned match the current data sent
	if (packets_ok) {
		int cmdptr = 0;
		for (int i = 0; i < cmds.size(); i++) {		
			packets_ok = packets_ok && (abs(cmds[i].currentMilliAmp - cmdsReturned[i].currentMilliAmp) < 1); // 1 mA rounding error? 
		}
	}

	// debug
	// writePlayFile("cmds_to_send.play", cmds);
	// writeBytes("cmd_bytes_sent.local.txt", allBytesSent);
	// writeBytes("cmd_bytes_returned.local.txt", allBytesRead);
	// writePlayFile("cmds_returned.local.play", cmdsReturned);

	return packets_ok;	
} */


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