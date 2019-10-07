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

ATLASLinkProducer::ATLASLinkProducer(CMOOSDBMQ *db, const std::string& brokerURI)
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
	
	destination = session->createTopic("FAULT-LINK-FROM-SIM");
	
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

void ATLASLinkProducer::sendToMQ(CMOOSMsg & msg)
{
    
}

ATLASLinkConsumer::ATLASLinkConsumer(CMOOSDBMQ *db, const std::string& brokerURI)
{
    this->db = db;
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
    destination = session->createTopic("FAULT-LINK-TO-SIM");
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
    // Need to convert this back into MOOS message
    static int count = 0;
    
    try {
	count++;
	const TextMessage* textMessage = dynamic_cast<const TextMessage*> (message);
	string text = "";
	
	if (textMessage != NULL) {
	    text = textMessage->getText();
	} else {
	    text = "NOT A TEXTMESSAGE!";
	}
	
	printf("Message #%d Received: %s\n", count, text.c_str());
	
    } catch (CMSException& e) {
	e.printStackTrace();
    }
    
    // Commit all messages.
    if (this->sessionTransacted) {
	session->commit();
    }
    
    // No matter what, tag the count down latch until done.
    //doneLatch.countDown();
}

/*
class ATLASLinkConsumer : public ExceptionListener,
                           public MessageListener,
                           public Runnable {

private:

    CountDownLatch latch;
    CountDownLatch doneLatch;
    Connection* connection;
    Session* session;
    Destination* destination;
    MessageConsumer* consumer;
    long waitMillis;
    bool useTopic;
    bool sessionTransacted;
    std::string brokerURI;

private:

    ATLASLinkConsumer(const ATLASLinkConsumer&);
    ATLASLinkConsumer& operator=(const ATLASLinkConsumer&);

public:

    ATLASLinkConsumer(const std::string& brokerURI, int numMessages, bool useTopic = false, bool sessionTransacted = false, int waitMillis = 30000) :
        latch(1),
        doneLatch(numMessages),
        connection(NULL),
        session(NULL),
        destination(NULL),
        consumer(NULL),
        waitMillis(waitMillis),
        useTopic(useTopic),
        sessionTransacted(sessionTransacted),
        brokerURI(brokerURI) {
    }

    virtual ~ATLASLinkConsumer() {
        cleanup();
    }

    void close() {
        this->cleanup();
    }

    void waitUntilReady() {
        latch.await();
    }

    virtual void run() {

        try {

            // Create a ConnectionFactory
            auto_ptr<ConnectionFactory> connectionFactory(
                ConnectionFactory::createCMSConnectionFactory(brokerURI));

            // Create a Connection
            connection = connectionFactory->createConnection();
            connection->start();
            connection->setExceptionListener(this);

            // Create a Session
            if (this->sessionTransacted == true) {
                session = connection->createSession(Session::SESSION_TRANSACTED);
            } else {
                session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
            }

            // Create the destination (Topic or Queue)
            if (useTopic) {
                destination = session->createTopic("TEST.FOO");
            } else {
                destination = session->createQueue("TEST.FOO");
            }

            // Create a MessageConsumer from the Session to the Topic or Queue
            consumer = session->createConsumer(destination);

            consumer->setMessageListener(this);

            std::cout.flush();
            std::cerr.flush();

            // Indicate we are ready for messages.
            latch.countDown();

            // Wait while asynchronous messages come in.
            doneLatch.await(waitMillis);

        } catch (CMSException& e) {
            // Indicate we are ready for messages.
            latch.countDown();
            e.printStackTrace();
        }
    }

    // Called from the consumer since this class is a registered MessageListener.
    virtual void onMessage(const Message* message) {

        static int count = 0;

        try {
            count++;
            const TextMessage* textMessage = dynamic_cast<const TextMessage*> (message);
            string text = "";

            if (textMessage != NULL) {
                text = textMessage->getText();
            } else {
                text = "NOT A TEXTMESSAGE!";
            }

            printf("Message #%d Received: %s\n", count, text.c_str());

        } catch (CMSException& e) {
            e.printStackTrace();
        }

        // Commit all messages.
        if (this->sessionTransacted) {
            session->commit();
        }

        // No matter what, tag the count down latch until done.
        doneLatch.countDown();
    }

    // If something bad happens you see it here as this class is also been
    // registered as an ExceptionListener with the connection.
    virtual void onException(const CMSException& ex AMQCPP_UNUSED) {
        printf("CMS Exception occurred.  Shutting down client.\n");
        ex.printStackTrace();
        exit(1);
    }

private:

    
};

*/
