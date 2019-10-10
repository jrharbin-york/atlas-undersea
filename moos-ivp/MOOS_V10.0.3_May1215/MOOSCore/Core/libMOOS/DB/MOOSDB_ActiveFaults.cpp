#include "MOOS/libMOOS/DB/MOOSDB.h"
#include "MOOS/libMOOS/DB/MOOSDB_MQ.h"

#include <activemq/library/ActiveMQCPP.h>

#include <iostream>

CMOOSDBMQ_ActiveFaults::CMOOSDBMQ()
{
    std::cout << "Creating CMOOSDBMQ... activating ActiveMQ interface\n";
    
    startMQInterface();
    prod = new ATLASLinkProducer(this, brokerURI);
    cons = new ATLASLinkConsumer(this, brokerURI);
    std::cout << "Completed" << endl;
}


// For activating with ActiveFaults. Variables can be
// overriden by a fault and while the override is
// in effect, notifications will be ignored
bool CMOOSDBMQ_ActiveFaults::OnNotify(CMOOSMsg &Msg)
{
  double dfTimeNow = HPMOOSTime();
  CMOOSDBVar & rVar= GetOrMakeVar(Msg);

  // If we have not overriden this variable, 
  // override time will be less than the current time.
  // This is always true since it begins as -1.0
  if (rVar.m_dfOverrideTime < dfTimeNow) {
    // In this case, handle notification normally
    // by calling the superclass method
    CMOOSDB::OnNotify(Msg);
    // Reset the override to 
    rVar.m_dfOverrideTime = -1.0;
  } else {
    // Override is still in the future
    std::cout << "Ignored msg due to override";
  }
}

bool CMOOSDBMQ_ActiveFaults::fromMQ(CMOOSMsg &Msg, double overrideTimeEnd) {
  // When a message notification comes in from the ActiveMQ,
  // always handle it as a standard notification
  return CMOOSDB::OnNotify(Msg);
  // Need to set a notification on this message
  setOverride(Msg, overrideTimeEnd);
}

void CMOOSDBMQ_ActiveFaults::startMQInterface() {
    activemq::library::ActiveMQCPP::initializeLibrary();
}

void CMOOSDBMQ_ActiveFaults::stopMQInterface() {
    activemq::library::ActiveMQCPP::shutdownLibrary();
}
