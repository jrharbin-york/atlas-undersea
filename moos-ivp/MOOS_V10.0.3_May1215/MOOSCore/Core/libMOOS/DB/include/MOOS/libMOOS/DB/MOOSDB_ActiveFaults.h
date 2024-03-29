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
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>

#include "MOOS/libMOOS/DB/MOOSDB.h"
#include "MOOS/libMOOS/DB/ATLASLink.h"

class ATLASLinkProducer;
class ATLASLinkConsumer;

class CMOOSDB_ActiveFaults : public CMOOSDB
{
public:
    bool faultsActive = true;
    //bool ProcessMsg(CMOOSMsg &MsgRx,MOOSMSG_LIST & MsgListTx);
    CMOOSDB_ActiveFaults(int port, const std::string mission_file);
    bool OnNotify(CMOOSMsg &Msg);

    bool fromMQ(CMOOSMsg &Msg, double overrideTimeEnd);
    bool fromMQ(CMOOSMsg &Msg);


private:
    bool sendMsgOut = true;

    ofstream debug_output;
    const string brokerURIBase = "failover:(tcp://localhost:";
    int activeMQPort = 61616;  // Default
    string broker_uri;
    string mission_file;

    ATLASLinkProducer * prod;
    ATLASLinkConsumer * cons;

    void startMQInterface();
    void stopMQInterface();
};

