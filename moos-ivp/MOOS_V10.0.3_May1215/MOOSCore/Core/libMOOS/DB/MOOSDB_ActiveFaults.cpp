#include "MOOS/libMOOS/DB/MOOSDB.h"
#include "MOOS/libMOOS/DB/MOOSDB_ActiveFaults.h"

#include <iostream>
#include <sstream>

#include "MOOS/libMOOS/DB/ATLASLink.h"

using namespace std;

CMOOSDB_ActiveFaults::CMOOSDB_ActiveFaults(int port)
{
    activeMQPort = port;

    ostringstream ss;
    ss << brokerURIBase << to_string(port) << ")";
    
    brokerURI = ss.str();
    cout << "Creating CMOOSDB_ActiveFaults... activating ActiveMQ interface on "
	 << port << endl;
    cout << "Connecting to " << brokerURI << endl;
    
    startMQInterface();
    prod = new ATLASLinkProducer(this, brokerURI);
    cons = new ATLASLinkConsumer(this, brokerURI);
    cout << "Completed" << endl;
}

// For activating with ActiveFaults. Variables can be
// overriden by a fault and while the override is
// in effect, notifications will be ignored
bool CMOOSDB_ActiveFaults::OnNotify(CMOOSMsg &Msg)
{
    double dfTimeNow = HPMOOSTime();
    CMOOSDBVar & rVar= GetOrMakeVar(Msg);
    // If problems, check the value of this variable, in
    // cases in which overrides are still in effect
    bool res = false;
    
    // If we have not overriden this variable, 
    // override time will be less than the current time.
    // This is always true since it begins as -1.0
    if (rVar.m_dfOverrideTime < dfTimeNow) {
	// In this case, handle notification normally
	// by calling the superclass method
	res = CMOOSDB::OnNotify(Msg);
	// Reset the override to 
	rVar.m_dfOverrideTime = -1.0;
    } else {
	// Override is still in the future
	std::cout << "Ignored msg due to override";
    }
    return res;
}

bool CMOOSDB_ActiveFaults::fromMQ(CMOOSMsg &Msg) {
  // When a message notification comes in from the ActiveMQ,
  // always handle it as a standard notification
  return CMOOSDB::OnNotify(Msg);
}

bool CMOOSDB_ActiveFaults::fromMQ(CMOOSMsg &Msg, double overrideTimeEnd) {
  CMOOSDBVar & rVar = GetOrMakeVar(Msg);
  // When a message notification comes in from the ActiveMQ,
  // always handle it as a standard notification
  bool res = CMOOSDB::OnNotify(Msg);
  // Need to set a notification on this message
  rVar.m_dfOverrideTime = overrideTimeEnd;
  // Return the original result from the notify call
  return res;
}

void CMOOSDB_ActiveFaults::startMQInterface() {
    activemq::library::ActiveMQCPP::initializeLibrary();
}

void CMOOSDB_ActiveFaults::stopMQInterface() {
    activemq::library::ActiveMQCPP::shutdownLibrary();
}
