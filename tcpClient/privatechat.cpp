#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>


PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString name)
{
    m_strChatName = name;
    m_strLoginName = tcpClient::getInstance().loginName();

}

//
void PrivateChat::updateMsg(const PDU *pdu)
{
    if(NULL == pdu)
    {
        return;
    }
    char sourceName[32] = {'\0'};
    memcpy(sourceName, pdu->caData, 32);
    QString strMsg = QString("%1 says: %2").arg(sourceName).arg((char*)pdu->caMsg);
    ui->showMsg_te->append(strMsg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear();
    if(NULL == strMsg)
    {
        QMessageBox::warning(this, "私聊", "聊天信息不能为空");
    }
    else
    {
        PDU *pdu = mkPDU(strMsg.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;

        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), 32);
        memcpy(pdu->caData + 32, m_strChatName.toStdString().c_str(), 32);
        strcpy((char*)pdu->caMsg, strMsg.toStdString().c_str());
        tcpClient::getInstance().gettcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    }
}
