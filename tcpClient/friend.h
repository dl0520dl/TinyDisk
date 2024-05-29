#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
// 页面所用到的部件
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局

#include "online.h"
#include "protocol.h"



class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUser(PDU *pdu);
    void updateFriendList(PDU *pdu);
    QString m_strSearchName;
    void updateGroupMsg(PDU *pdu);

    QListWidget *getFriendList();

signals:

public slots:
    void showOnline();
    void searchUser();
    void flushFriend();
    void deleteFriend();
    void privateChat();
    void groupChat();



private:
    QTextEdit *m_pShowMsgTE;     // 显示群聊信息
    QListWidget *m_pFriendListWidget;         // 好友列表
    QLineEdit *m_pInputMsgLE;       // 群聊信息输入框
    QPushButton *m_pDelFriendPB;      // 删除好友
    QPushButton *m_pFlushFriendPB;    // 刷新好友列表
    QPushButton *m_pShowOnlineUserPB; // 显示所有在线用户
    QPushButton *m_pSearchUserPB;     // 查找用户
    QPushButton *m_pMsgSendPB;   // 群聊发送消息
    QPushButton *m_pPrivateChatPB;    // 私聊按钮，默认群聊

    Online *m_pOnline;

};

#endif // FRIEND_H
