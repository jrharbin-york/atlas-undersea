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

#include <iostream>
#include <memory>
#include <regex>
#include <stdio.h>
#include <stdlib.h>

#include "ATLASLinkApp.h"
#include "MBUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

ATLASLinkProducer::ATLASLinkProducer(const std::string &brokerURI,
                                     const std::string &activeMQTopic) {
  try {
    debugLog.open("/tmp/ATLASLinkApp_debug.log");
    // Create a ConnectionFactory
    auto_ptr<ConnectionFactory> connectionFactory(
        ConnectionFactory::createCMSConnectionFactory(brokerURI));

    // Create a Connection
    connection = connectionFactory->createConnection();
    connection->start();

    // Create a Session
    if (this->sessionTransacted) {
      session = connection->createSession(Session::SESSION_TRANSACTED);
    } else {
      session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
    }

    destination = session->createTopic(activeMQTopic);

    // Create a MessageProducer from the Session to the Topic or Queue
    producer = session->createProducer(destination);
    producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

ATLASLinkProducer::~ATLASLinkProducer() { cleanup(); }

void ATLASLinkProducer::cleanup() {
  if (connection != NULL) {
    try {
      connection->close();
    } catch (cms::CMSException &ex) {
      ex.printStackTrace();
    }
  }

  // Destroy resources.
  try {
    delete destination;
    destination = NULL;
    delete producer;
    producer = NULL;
    delete session;
    session = NULL;
    delete connection;
    connection = NULL;
  } catch (CMSException &e) {
    e.printStackTrace();
  }
}

void ATLASLinkProducer::sendToMQ(CMOOSMsg &mooseMsg) {
  if (session != nullptr) {
    string activeMQMsg = mooseMsg.GetKey() + "=" + mooseMsg.GetAsString();
    TextMessage *msg = session->createTextMessage(activeMQMsg);
    // FIX: real properties here
    msg->setStringProperty("USER_NAME", "MOOSIvp");
    msg->setIntProperty("USER_CODE", 42);
    producer->send(msg);
    cout << "ActiveMQ Message sent: " << activeMQMsg << endl;
    debugLog << "ActiveMQ Message sent: " << activeMQMsg << endl;
    delete msg;
  } else {
          cout << "Cannot sendToMQ - session is null! ActiveMQ not started " << endl;
          debugLog << "Cannot sendToMQ - session is null! ActiveMQ not started " << endl;
  }
}
