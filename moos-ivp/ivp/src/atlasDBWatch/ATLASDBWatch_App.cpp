#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/util/Config.h>
#include <cms/BytesMessage.h>
#include <cms/Connection.h>
#include <cms/ExceptionListener.h>
#include <cms/MapMessage.h>
#include <cms/MessageListener.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/Runnable.h>
#include <decaf/lang/System.h>
#include <decaf/lang/Thread.h>
#include <decaf/util/concurrent/CountDownLatch.h>

#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Utils/ProcessConfigReader.h"

#include "ATLASDBWatch_App.h"
#include "ATLASLinkApp.h"
#include "ATLASLog.h"

#include "MBUtils.h"

#include <functional>
#include <iterator>

using namespace std;

ATLASDBWatch::ATLASDBWatch() {
}

//---------------------------------------------------------
// Procedure: OnNewMail
bool ATLASDBWatch::OnNewMail(MOOSMSG_LIST &NewMail) {
  MOOSMSG_LIST::iterator p;
  for (p = NewMail.begin(); p != NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key = msg.GetKey();
    cout << key << " seen" << endl;
    prod->sendToMQ(msg);
  }
  return true;
}

void ATLASDBWatch::SetupActiveMQ() {
  activemq::library::ActiveMQCPP::initializeLibrary();
  string mq_activemq_url = "failover:(tcp://localhost:" + to_string(mq_activemq_port) + ")";
  prod = new ATLASLinkProducer(mq_activemq_url, mq_topic_name);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer
bool ATLASDBWatch::OnConnectToServer() {
  return true;
}

//------------------------------------------------------------
// Procedure: RegisterVariables
void ATLASDBWatch::RegisterVariables() {
  for (string var : vars_to_watch) {
    m_Comms.Register(var, 0);
    std::cout << "Registering with MOOSDB for variable " << var << endl;
  }
}

//---------------------------------------------------------
// Procedure: Iterate()
bool ATLASDBWatch::Iterate() {
    return true;
}

bool ATLASDBWatch::ScanForVariable(const string &fileLine,
                                   const string &targetVar,
                                   function<void(string)> matchAction) {
  string sLine = fileLine;
  sLine = stripBlankEnds(sLine);
  string sVarName = MOOSChomp(sLine, "=");
  if (MOOSStrCmp(sVarName, targetVar)) {
    if (!strContains(sLine, " ")) {
      stripBlankEnds(sLine);
      matchAction(sLine);
      return true;
    } else
      return false;
  } else
    return false;
}

// Process the mission file and determine the variables to watch
// store them to vars_to_watch, set other variables when found
void ATLASDBWatch::ProcessMissionFile() {
  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

  STRING_LIST::iterator p;
  for (p = sParams.begin(); p != sParams.end(); p++) {
    ScanForVariable(*p, "WATCH_VAR",
                    [this](string foundVal) { this->vars_to_watch.push_back(foundVal); });
    ScanForVariable(*p, "ACTIVEMQ_PORT",
                    [this](string foundVal) { this->mq_activemq_port = stoi(foundVal); });
    ScanForVariable(*p, "ACTIVEMQ_TOPIC",
                    [this](string foundVal) { this->mq_topic_name = foundVal; });
  }
}

// Get secondary paramters such as the user name/code for ActiveMQ messages
// Must be called after ActiveMQ is setup
void ATLASDBWatch::SecondaryProcessMissionFile() {
    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    m_MissionReader.GetConfiguration(GetAppName(), sParams);

    STRING_LIST::iterator p;
    for (p = sParams.begin(); p != sParams.end(); p++) {
        ScanForVariable(*p, "ACTIVEMQ_USERCODE",
                        [this](string foundVal) { this->prod->setUserCode(stoi(foundVal)); });
        ScanForVariable(*p, "ACTIVEMQ_USERNAME",
                        [this](string foundVal) { this->prod->setUserName(foundVal); });
    }
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//      Note: happens before connection is open
bool ATLASDBWatch::OnStartUp() {
  ProcessMissionFile();
  for (string var : vars_to_watch) {
      debug << "Watching variable " << var << "\n";
  };
  SetupActiveMQ();
  SecondaryProcessMissionFile();
  cout << "SetupActiveMQ() done" << endl;
  RegisterVariables();
  cout << "RegisterVariables() done" << endl;
  return true;
}
