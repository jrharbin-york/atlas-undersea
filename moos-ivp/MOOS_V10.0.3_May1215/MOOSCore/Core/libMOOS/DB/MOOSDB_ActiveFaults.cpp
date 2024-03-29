#include "MOOS/libMOOS/DB/MOOSDB_ActiveFaults.h"
#include "MOOS/libMOOS/DB/MOOSDB.h"

#include <iostream>
#include <sstream>

#include "MOOS/libMOOS/DB/ATLASLink.h"

using namespace std;

CMOOSDB_ActiveFaults::CMOOSDB_ActiveFaults(int port, string mission_file) {
    debug_output.open("/tmp/af_debug" + mission_file + ".log");
    debug_output << "MOOSDB_ActiveFaults debug" << endl;

    activeMQPort = port;
    this->mission_file = mission_file;

    ostringstream ss;
    ss << brokerURIBase << to_string(port) << ")";

    broker_uri = ss.str();
    cout << "Creating CMOOSDB_ActiveFaults... activating ActiveMQ interface on "
         << port << endl;
    cout << "Connecting to " << broker_uri << endl;

    startMQInterface();
    prod = new ATLASLinkProducer(this, broker_uri, mission_file);
    cons = new ATLASLinkConsumer(this, broker_uri, mission_file);
    cout << "ActiveMQ connection completed!" << endl;
}

// For activating with ActiveFaults. Variables can be
// overriden by a fault and while the override is
// in effect, notifications will be ignored
bool CMOOSDB_ActiveFaults::OnNotify(CMOOSMsg &Msg) {
  double dfTimeNow = HPMOOSTime();
  CMOOSDBVar &rVar = GetOrMakeVar(Msg);

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
    // Reset the override
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
  CMOOSDBVar &rVar = GetOrMakeVar(Msg);
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
