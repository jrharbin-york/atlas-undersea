#pragma once

#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>

#include "MOOS/libMOOS/MOOSLib.h"

#include <regex>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <memory>

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

class ATLASLinkProducer {
private:
    Connection* connection;
    Session* session;
    Destination* destination;
    MessageProducer* producer;
    int numMessages;
    bool useTopic;
    bool sessionTransacted;
    std::string brokerURI;

    // These can be overriden in the configuration file
    int activeMQUserCode = 42;
    string activeMQUserName = "ATLASLinkApp";

    ofstream debugLog;

    void cleanup();

public:
    ATLASLinkProducer(const std::string& brokerURI,
                      const std::string& atlas_link_extraname);
    ~ATLASLinkProducer();

    void setUserCode(int userCode);
    void setUserName(string userName);

    //void sendToMQString(const string &textmsg);
    void sendToMQ(CMOOSMsg &mooseMsg);
};
