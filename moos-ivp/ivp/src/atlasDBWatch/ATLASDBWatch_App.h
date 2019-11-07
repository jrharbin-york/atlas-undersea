#pragma once

#include <functional>

#include "ATLASLinkApp.h"
#include "MOOS/libMOOS/MOOSLib.h"

class ATLASDBWatch : public CMOOSApp {
public:
  ATLASDBWatch();
  virtual ~ATLASDBWatch() {}

  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();

protected:
  ATLASLinkProducer *prod;
  void RegisterVariables();
  void SetupActiveMQ();
  void ProcessMissionFile();
  void SecondaryProcessMissionFile();
  bool ScanForVariable(const string &fileLine, const string &targetVar, function <void(string)> matchAction);

  vector<string> vars_to_watch;
  int mq_activemq_port;
  string mq_topic_name;
};
