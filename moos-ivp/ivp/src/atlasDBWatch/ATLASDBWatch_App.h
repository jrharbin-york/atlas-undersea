#pragma once

#include "MOOS/libMOOS/MOOSLib.h"
#include "ATLASLinkApp.h"

class ATLASDBWatch : public CMOOSApp
{
 public:
  ATLASDBWatch();
  virtual ~ATLASDBWatch() {}

  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();

  void setIncomingVar(std::string s) {m_incoming_var=s;}
  void setOutgoingVar(std::string s) {m_outgoing_var=s;}

 protected:
  ATLASLinkProducer * prod;
  void RegisterVariables();

 protected: // State variables
  unsigned long int m_tally_recd;
  unsigned long int m_tally_sent;
  unsigned long int m_iterations;

 protected: // Configuration variables
  std::string       m_incoming_var;
  std::string       m_outgoing_var;

  double            m_start_time_postings;
  double            m_start_time_iterations;
};
