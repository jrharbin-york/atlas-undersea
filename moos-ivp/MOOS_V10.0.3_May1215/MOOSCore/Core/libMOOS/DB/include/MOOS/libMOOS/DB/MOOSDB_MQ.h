class CMOOSDBMQ : public CMOOSDB
{
public:
    bool faultsActive = true;
    bool ProcessMsg(CMOOSMsg &MsgRx,MOOSMSG_LIST & MsgListTx);
    bool OnNotify(CMOOSMsg &Msg);
    CMOOSDBMQ();

private:
    bool sendMsgOut = true;
    ATLASLinkProducer prod;
    ATLASLinkConsumer cons;
    void startMQInterface();
    void stopMQInterface();
};
