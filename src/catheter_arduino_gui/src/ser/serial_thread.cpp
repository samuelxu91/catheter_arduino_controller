#include "ser/serial_thread.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <wx/wx.h>
#include <wx/numdlg.h>
// Here is the serial thread.

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

#define PAUSE_INC_MS 1 //ms to wait between consecutive probes



// This thread loop is created as part of 
void SerialThreadObject::serialLoop()
{

	boost::posix_time::ptime t1(boost::posix_time::second_clock::local_time());
    int64_t delay(0);
	int cmdIndex(0);
	while (active)
	{
		if(ss->dataAvailable())
		{
			CatheterChannelCmd incomingData;
			boost::mutex::scoped_lock lock(threadMutex);
			comStatus newCom(ss->getData(commandFromArd.commandList));
			lock.unlock();
			printf("recieved command: \n");
			if(newCom == valid)
			{
				printf("valid\n");
				if(statusGridData != NULL)
				{
					statusGridData->updateCmdList(commandFromArd.commandList);
				}
			}
		}
		// This is a fifo command
		// warning: no lock is used to read the vector size.
		if (commandsToArd.size() > 0) 
		{
			boost::posix_time::time_duration diff(boost::posix_time::microsec_clock::local_time() - t1);
			int64_t diffAmt(diff.total_nanoseconds());
			if (diffAmt > delay)
			{
				// send the first command
				t1 = boost::posix_time::microsec_clock::local_time();
				boost::mutex::scoped_lock lock(threadMutex);
				int writeOut(ss->sendCommand(commandsToArd[0],cmdIndex));
				printf("Sent %d bytes", writeOut);
				delay = commandsToArd[0].delayTime*1000000;
				commandsToArd.erase(commandsToArd.begin());
				
				lock.unlock();
				cmdIndex++;
			}
		}
		boost::this_thread::sleep(boost::posix_time::microseconds(1));
	}
}

void SerialThreadObject::setStatusGrid(statusData* newPtr)
{
	statusGridData = newPtr;
}

void SerialThreadObject::serialCommand(const ThreadCmd& incomingCommand)
{
	// check for avaiable data:
		
		if(incomingCommand != noCmd)
		{
			boost::mutex::scoped_lock looplock(threadMutex);
			// act on the command.
			switch (incomingCommand)
			{
			case resetArduino:
			{
				// reset the arduino.
				//flush out the commands.
				commandsToArd.clear();
				commandsToArd.push_back(resetCmd());
				//add the reset command:
				;
			}
			break;
			case resetSerial:
			{
				if(textStatusData != NULL)
				{
					textStatusData->appendText(std::string("Attempting to reset Arduino Serial Connection"));
				}
				//reset the serial bus.
				std::vector<std::string> ports;
				ss->getAvailablePorts(ports);

				if (!ports.size())
				{
					if(textStatusData != NULL)
					{
						textStatusData->appendText(std::string("No Serial Ports found."));
					}
				} 
				else
				{
					if (ports.size() == 1)
					{
						ss->setPort(ports[0]);
						if(textStatusData != NULL)
						{
							textStatusData->appendText(std::string("Connecting to Port: ")+ports[0]);
						}
					}
					else
					{
					// have user select the correct port
						for (int i = 0; i < ports.size(); i++)
						{
							wxMessageBox(wxString::Format("Found Serial Port: %s (%d/%d)", wxString(ports[i]), i + 1, ports.size()));
						}
						int which_port(wxGetNumberFromUser(wxEmptyString, wxT("Select Serial Port Number"), wxEmptyString, 0, 1, ports.size()) - 1);
						wxMessageBox(wxString::Format("Selected Serial Port: %s", wxString(ports[which_port])));
						ss->setPort(ports[which_port]);
						if(textStatusData != NULL)
						{
							textStatusData->appendText(std::string("Connecting to Port: ")+ports[which_port]);
						}
					}


					connected = ss->start();
					if(textStatusData != NULL && connected)
					{
						textStatusData->appendText(std::string("Connected!!"));
					}
				}

			}	
			break;
			case connect:
				//connect to the arduino
			break;
			case disconnect:
				//disconnect from the arduino
			break;
			case poll:
				commandsToArd.clear();
				commandsToArd.push_back(pollCmd());
			break;
			default:
				if(textStatusData != NULL)
				{
					textStatusData->appendText(std::string("Command not recognized"));
				}
			}
			looplock.unlock();
		}
		// return currentStatus;

}

// explicit constructor
SerialThreadObject::SerialThreadObject(): connected(false), active(true), ss(new CatheterSerialSender), thrd(),
	textStatusData(NULL), statusGridData(NULL)
{
	// initialize all of the variables.
	std::vector< CatheterChannelCmd > commandsToArd;
	std::vector< CatheterChannelCmd > commandsFromArd;

	//start the loop.
	//retain a handle to the thread (This way a graceful exit is possible).
	thrd = boost::thread(boost::bind(&SerialThreadObject::serialLoop, this));

}

void SerialThreadObject::setStatusTextPtr(incomingText* textPtr)
{
	this->textStatusData = textPtr;
}

void SerialThreadObject::stopThreads()
{
	boost::mutex::scoped_lock lock(threadMutex);
	active = false;
	lock.unlock();
	thrd.join();
	return;
}

SerialThreadObject::~SerialThreadObject()
{
	// lock the mutex
	stopThreads();
	ss->stop();
	boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	delete ss;
	return;
}


	// status commands
void SerialThreadObject::queueCommand(const CatheterChannelCmdSet &commandToArd_, bool flush)
{
	// lock the mutex
	boost::mutex::scoped_lock
    lock(threadMutex);
    //append the new command.
    if (flush)
    {
    	commandsToArd.clear();
    }
    commandsToArd.push_back(commandToArd_);
}

void SerialThreadObject::queueCommands(const std::vector< CatheterChannelCmdSet > &commandsToArd_, bool flush)
{
	// lock the mutex
	boost::mutex::scoped_lock
    lock(threadMutex);
    //append the new command.
    if (flush)
    {
    	commandsToArd.clear();
    }
    commandsToArd.insert(commandsToArd.end(), commandsToArd_.begin(), commandsToArd_.end());
}
