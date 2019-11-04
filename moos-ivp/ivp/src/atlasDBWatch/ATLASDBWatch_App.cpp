// ActiveMQ headers
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

#include "ATLASDBWatch_App.h"
#include "ATLASLinkApp.h"

#include "MBUtils.h"
#include <iterator>

using namespace std;

ATLASDBWatch::ATLASDBWatch() {
    activemq::library::ActiveMQCPP::initializeLibrary();
    prod = new ATLASLinkProducer("failover:(tcp://localhost:61616)", "targ_shoreside.moos");
    m_tally_recd = 0;
    m_tally_sent = 0;
    m_iterations = 0;
    m_start_time_postings = 0;
    m_start_time_iterations = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ATLASDBWatch::OnNewMail(MOOSMSG_LIST &NewMail) {
  MOOSMSG_LIST::iterator p;
  for (p = NewMail.begin(); p != NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key = msg.GetKey();

    if (key == "UHZ_DETECTION_REPORT") {
            cout << "UHZ_DETECTION_REPORT seen" << endl;
            std::string stest {"UHZ_DET_TEST"};
            prod->sendToMQString(stest);
    }
  }
  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ATLASDBWatch::OnConnectToServer() {
  RegisterVariables();
  return (true);
}

//------------------------------------------------------------
// Procedure: RegisterVariables

void ATLASDBWatch::RegisterVariables() {
  m_Comms.Register("UHZ_DETECTION_REPORT", 0);
}

//---------------------------------------------------------
// Procedure: Iterate()

bool ATLASDBWatch::Iterate() {
  m_iterations++;

  unsigned int i, amt = (m_tally_recd - m_tally_sent);
  for (i = 0; i < amt; i++) {
    m_tally_sent++;
    Notify(m_outgoing_var, m_tally_sent);
  }

  // If this is the first iteration just note the start time, otherwise
  // note the currently calculated frequency and post it to the DB.
  if (m_start_time_iterations == 0)
    m_start_time_iterations = MOOSTime();
  else {
    double delta_time = (MOOSTime() - m_start_time_iterations);
    double frequency = 0;
    if (delta_time > 0)
      frequency = (double)(m_iterations) / delta_time;
    Notify(m_outgoing_var + "_ITER_HZ", frequency);
  }

  // If this is the first time a received msg has been noted, just
  // note the start time, otherwise calculate and post the frequency.
  if (amt > 0) {
    if (m_start_time_postings == 0)
      m_start_time_postings = MOOSTime();
    else {
      double delta_time = (MOOSTime() - m_start_time_postings);
      double frequency = 0;
      if (delta_time > 0)
        frequency = (double)(m_tally_sent) / delta_time;
      Notify(m_outgoing_var + "_POST_HZ", frequency);
    }
  }
  return (true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//      Note: happens before connection is open

bool ATLASDBWatch::OnStartUp() {
  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  m_MissionReader.GetConfiguration(GetAppName(), sParams);

  RegisterVariables();
  cout << "RegisterVariables() done" << endl;
  return true;
}
