class CMOOSDBMQ : public CMOOSDB
{
public:
    bool faultsActive = true;
    bool ProcessMsg(CMOOSMsg &MsgRx,MOOSMSG_LIST & MsgListTx);
    bool OnNotify(CMOOSMsg &Msg);
    CMOOSDBMQ();

private:
    void startMQInterface();
    void stopMQInterface();
};

