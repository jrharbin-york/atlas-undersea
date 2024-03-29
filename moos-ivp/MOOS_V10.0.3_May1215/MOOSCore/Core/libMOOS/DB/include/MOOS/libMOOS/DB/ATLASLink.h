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

#include <regex>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>

#include "MOOS/libMOOS/DB/MOOSDB.h"
#include "MOOS/libMOOS/DB/MOOSDB_MQ.h"
#include "MOOS/libMOOS/DB/MOOSDB_ActiveFaults.h"

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

class CMOOSDBMQ;
class CMOOSDB_ActiveFaults;

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

    void cleanup();

public:
    ATLASLinkProducer(CMOOSDB_ActiveFaults * db,
		      const std::string& brokerURI,
		      const std::string& atlas_link_extraname);
    ~ATLASLinkProducer();
    void sendToMQ(CMOOSMsg &Msg);
};

class ATLASLinkConsumer : public MessageListener {
private:
    Connection* connection;
    Session* session;
    Destination* destination;
    MessageConsumer* consumer;
    long waitMillis;
    bool useTopic;
    bool sessionTransacted;

    CMOOSDBMQ * db_mq;
    CMOOSDB_ActiveFaults * db_activefaults;

    std::string brokerURI;
    std::regex messageRegex;
    //("(.+)\\|(\w+)=(\w+)");
    // FIX: set this regex here rather than in the
    // ATLAS link code

    void onMessage(const Message* message);
    void cleanup();

public:
    ATLASLinkConsumer(CMOOSDB_ActiveFaults * db,
		      const std::string& brokerURI,
		      const std::string& atlas_link_extraname);
    ~ATLASLinkConsumer();
};
