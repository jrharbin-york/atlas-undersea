#include "MOOS/libMOOS/DB/MOOSDB.h"
#include "MOOS/libMOOS/DB/MOOSDB_MQ.h"

#include <activemq/library/ActiveMQCPP.h>

#include <iostream>

CMOOSDBMQ::CMOOSDBMQ()
{
    std::cout << "Creating CMOOSDBMQ" << endl;
}

// Determines what to do with the message. 
// If it is a notification, we pass it off to the Apache MQ
bool CMOOSDBMQ::ProcessMsg(CMOOSMsg &MsgRx,MOOSMSG_LIST & MsgListTx)
{
    switch(MsgRx.m_cMsgType)
    {
    case MOOS_NOTIFY:    //NOTIFICATION
        return OnNotify(MsgRx);
        break;
    case MOOS_WILDCARD_UNREGISTER:
    case MOOS_UNREGISTER:
        return OnUnRegister(MsgRx);
        break;
    case MOOS_REGISTER:    //REGISTRATION
    case MOOS_WILDCARD_REGISTER:
        return OnRegister(MsgRx);
        break;
    case MOOS_NULL_MSG:
        break;    
    case MOOS_COMMAND:  //COMMAND
        break;
    case MOOS_SERVER_REQUEST:
        return DoServerRequest(MsgRx,MsgListTx);
        break;
    }
    return true;
}

bool CMOOSDBMQ::OnNotify(CMOOSMsg &Msg)
{
    cout << "Ignoring msg!!!";
    return true;
}

void startMQInterface() {
    activemq::library::ActiveMQCPP::initializeLibrary();
}

void stopMQInterface() {
    activemq::library::ActiveMQCPP::shutdownLibrary();
}
