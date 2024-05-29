#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include "privatechat.h"


Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pShowMsgTE = new QTextEdit;     // 显示群发信息
    m_pFriendListWidget = new QListWidget;         // 好友列表
    m_pInputMsgLE = new QLineEdit;       // 群聊信息输入框
    m_pDelFriendPB = new QPushButton("删除好友");      // 删除好友
    m_pFlushFriendPB = new QPushButton("刷新好友");    // 刷新好友列表
    m_pShowOnlineUserPB = new QPushButton("显示在线用户"); // 显示所有在线用户
    m_pSearchUserPB = new QPushButton("查找用户");     // 查找用户

    m_pMsgSendPB = new QPushButton("好友群发");   // 群聊发送消息
    m_pPrivateChatPB = new QPushButton("私聊");    // 私聊按钮，默认群聊

    QVBoxLayout *pLeftPBVBL = new QVBoxLayout;
    pLeftPBVBL->addWidget(m_pDelFriendPB);
    pLeftPBVBL->addWidget(m_pFlushFriendPB);
    pLeftPBVBL->addWidget(m_pShowOnlineUserPB);
    pLeftPBVBL->addWidget(m_pSearchUserPB);
    pLeftPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pLeftPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);


    connect(m_pShowOnlineUserPB, SIGNAL(clicked(bool)), this, SLOT(showOnline()));

    connect(m_pSearchUserPB, SIGNAL(clicked(bool)), this, SLOT(searchUser()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(deleteFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool)), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool)), this, SLOT(groupChat()));


}

void Friend::showAllOnlineUser(PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    m_pOnline->showUser(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(NULL == pdu)
    {
        return;
    }
    char caName[32] = {'\0'};
    char onlineStatus[4] = {'\0'};
    unit strSize = pdu->uiMsgLen / 36;

    m_pFriendListWidget->clear();
    for(unit i = 0; i < strSize ; i++)
    {
        memcpy(caName, (char*)(pdu->caMsg) + i * 36, 32);
        memcpy(onlineStatus, (char*)(pdu->caMsg) + 32 + i * 36, 4);
        qDebug() << "客户端好友" << caName << " " << onlineStatus;
        m_pFriendListWidget->addItem(QString("%1\t%2").arg(caName)
                                     .arg(strcmp(onlineStatus, "1") == 0?"在线":"离线"));
    }
    return;
}

void Friend::updateGroupMsg(PDU *pdu)
{
    if(NULL == pdu)
    {
        return;
    }
    QString strMsg = QString("%1 says: %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden()){
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        tcpClient::getInstance().gettcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else{
        m_pOnline->hide();
    }
}

void Friend::searchUser()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名：");
    if(!m_strSearchName.isEmpty()){
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        tcpClient::getInstance().gettcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriend()
{
    QString strName = tcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
    tcpClient::getInstance().gettcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::deleteFriend()
{
    QListWidgetItem *deleteItem = m_pFriendListWidget->currentItem();
    if(NULL == deleteItem)
    {
        QMessageBox::warning(this, "删除好友", "请选择要删除的好友");
        return;
    }
    QString deleteName = deleteItem->text().split("\t")[0];
    QString sourceName = tcpClient::getInstance().loginName(); // 登录用户用户名
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    memcpy(pdu->caData, sourceName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, deleteName.toStdString().c_str(), 32);
    tcpClient::getInstance().gettcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::privateChat()
{
    QListWidgetItem *privateChatItem = m_pFriendListWidget->currentItem();
    if(NULL == privateChatItem)
    {
        QMessageBox::warning(this, "私聊好友", "请选择要私聊的好友");
        return;
    }
    QString privateChatName = privateChatItem->text().split("\t")[0];
    PrivateChat::getInstance().setChatName(privateChatName);
    if(PrivateChat::getInstance().isHidden())
    {
        PrivateChat::getInstance().show();
    }

}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text();
    if(NULL == strMsg)
    {
        QMessageBox::warning(this, "好友群发", "信息不能为空");
        return;
    }
    PDU *pdu = mkPDU(strMsg.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
    QString sourceName = tcpClient::getInstance().loginName();
    strncpy(pdu->caData ,sourceName.toStdString().c_str(), sourceName.size());  //群发的人
    strncpy((char*)pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());    //群发的信息
    tcpClient::getInstance().gettcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}
