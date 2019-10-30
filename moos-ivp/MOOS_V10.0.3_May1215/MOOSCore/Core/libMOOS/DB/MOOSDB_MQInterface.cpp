/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// START SNIPPET: demo

// There is a producer that sends out the incoming Notify(), and a consumer
// on the ATLAS middleware side
// Then there is a consumer on this side that receives the messages that
// have been approved, modified by the fault generation system

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
#include <regex>

#include "MOOS/libMOOS/DB/MOOSDB_MQ.h"
#include "MOOS/libMOOS/DB/ATLASLink.h"

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

ATLASLinkProducer::~ATLASLinkProducer()
{
    cleanup();
}

// DELETE this constructor since we are only using CMOOSDB_ActiveFaults
ATLASLinkProducer::ATLASLinkProducer(CMOOSDBMQ *db, const std::string& brokerURI,
				     const std::string& atlas_link_extraname)
{
    try {
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
	
	destination = session->createTopic("FAULTS-SIM-TO-ATLAS");
	
	// Create a MessageProducer from the Session to the Topic or Queue
	producer = session->createProducer(destination);
	producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
    } catch (CMSException& e) {
	e.printStackTrace();
    }
}

ATLASLinkProducer::ATLASLinkProducer(CMOOSDB_ActiveFaults *db,
				     const std::string& brokerURI,
				     const std::string& atlas_link_extraname)
{
    try {

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

	// Determine the topic name
	ostringstream topicname;
	topicname << "FAULTS-SIM-TO-ATLAS";
	if (!atlas_link_extraname.empty())
	  topicname << "-" << atlas_link_extraname;

	destination = session->createTopic(topicname.str());
	
	// Create a MessageProducer from the Session to the Topic or Queue
	producer = session->createProducer(destination);
	producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
    } catch (CMSException& e) {
	e.printStackTrace();
    }
}

void ATLASLinkProducer::cleanup() {
    if (connection != NULL) {
	try {
	    connection->close();
	} catch (cms::CMSException& ex) {
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
    } catch (CMSException& e) {
	e.printStackTrace();
    }
};

void ATLASLinkProducer::sendToMQ(CMOOSMsg & moosemsg)
{
    TextMessage* msg = session->createTextMessage();
//    unsigned int msg_buffer_size = moosemsg.GetSizeInBytesWhenSerialised();

//    std::vector<unsigned char> buffer(msg_buffer_size);
//    if (!moosemsg.Serialize(buffer.data(), msg_buffer_size))
//    {
//	throw std::runtime_error("failed msg serialisation");
//    }
//    msg->setText(reinterpret_cast<const char*>(buffer.data()));
    msg->setText(moosemsg.GetAsString());
    // Set some Properties
    msg->setStringProperty("USER_NAME", "MOOSIvp");
    msg->setIntProperty("USER_CODE", 42);
    producer->send(msg);
    cout << "Message sent: " << moosemsg.GetAsString() << endl;
    delete msg;
}

ATLASLinkConsumer::ATLASLinkConsumer(CMOOSDBMQ *db, const std::string& brokerURI,
				     const std::string& atlas_link_extraname)
{
    this->db_mq = db;
    // Create a ConnectionFactory
    auto_ptr<ConnectionFactory> connectionFactory(
	ConnectionFactory::createCMSConnectionFactory(brokerURI));
    
    // Create a Connection
    connection = connectionFactory->createConnection();
    connection->start();
    //connection->setExceptionListener(this);
    
    // Create a Session
    if (this->sessionTransacted == true) {
	session = connection->createSession(Session::SESSION_TRANSACTED);
    } else {
	session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
    }
    
    // Create the destination (Topic or Queue)
    destination = session->createTopic("FAULTS-ATLAS-TO-SIM");
    // Create a MessageConsumer from the Session to the Topic or Queue
    consumer = session->createConsumer(destination);
    consumer->setMessageListener(this);
}

ATLASLinkConsumer::ATLASLinkConsumer(CMOOSDB_ActiveFaults *db,
				     const std::string& brokerURI,
				     const std::string& atlas_link_extraname)
{
    this->db_activefaults = db;
    // Create a ConnectionFactory
    auto_ptr<ConnectionFactory> connectionFactory(
	ConnectionFactory::createCMSConnectionFactory(brokerURI));
    
    // Create a Connection
    connection = connectionFactory->createConnection();
    connection->start();
    //connection->setExceptionListener(this);
    
    // Create a Session
    if (this->sessionTransacted == true) {
	session = connection->createSession(Session::SESSION_TRANSACTED);
    } else {
	session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
    }

    // Determine the topic name
    ostringstream topicname;
    topicname << "FAULTS-SIM-TO-ATLAS";
    if (!atlas_link_extraname.empty())
      topicname << "-" << atlas_link_extraname;
    
    // Create the destination (Topic or Queue)
    destination = session->createTopic(topicname.str());
    // Create a MessageConsumer from the Session to the Topic or Queue
    consumer = session->createConsumer(destination);
    consumer->setMessageListener(this);
}


ATLASLinkConsumer::~ATLASLinkConsumer() {
    cleanup();
}

void ATLASLinkConsumer::cleanup() {
    if (connection != NULL) {
	try {
	    connection->close();
	} catch (cms::CMSException& ex) {
	    ex.printStackTrace();
	}
    }
    
    // Destroy resources.
    try {
	delete destination;
	destination = NULL;
	delete consumer;
	consumer = NULL;
	delete session;
	session = NULL;
	delete connection;
	connection = NULL;
    } catch (CMSException& e) {
	e.printStackTrace();
    }
}

void ATLASLinkConsumer::onMessage(const Message* message)
{
    try {
	const TextMessage* textMessage = dynamic_cast<const TextMessage*> (message);
	string text = "";
	
	if (textMessage != NULL) {
	    text = textMessage->getText();
	    smatch matches;

	    std::regex tempRegex("(.+)\\|(\\w+)=(.+)");
		
	    // Parse the text from the message
	    // Message format e.g. (time_double)|m=v
	    if (std::regex_search(text, matches, tempRegex)) {
		double endTime = stod(matches[1]);
		string key = matches[2];
		string val = matches[3];
		CMOOSMsg * moosemsg = new CMOOSMsg(MOOS_STRING, key, val);
		if (db_activefaults)
		    db_activefaults->fromMQ(*moosemsg, endTime);
		delete moosemsg;
	    } else {
		cout << "No match" << endl;
	    }
	} else {
	    cout << "Not a text message" << endl;
	}
		
    } catch (CMSException& e) {
	e.printStackTrace();
    } catch (invalid_argument &ia) {
	cout << "Exception: invalid time" << endl;
    }
    
    // Commit all messages.
    if (this->sessionTransacted) {
	session->commit();
    }
    
    // No matter what, tag the count down latch until done.
    //doneLatch.countDown();
}

