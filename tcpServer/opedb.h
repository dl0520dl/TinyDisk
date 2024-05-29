#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = 0);
    static OpeDB& getInstance();
    void init();
    //~OpeDB();
    bool handleRegist(const char *name, const char *pwd);
    bool handleLogin(const char *name, const char *pwd);
    void handleOffline(const char *name);
    QStringList handleAllOnline();
    int handleSearchUser(const char *name);
    //加好友，首先判断对方是否在线，如果在线的话，①对方已经是好友，②对方不是好友
    int handleAddFriend(const char *pername, const char *name);
    int getIdByUserName(const char *name);                // 根据用户名获取用户id
    void handleAddFriendAgree(const char *addedName, const char *sourceName); // 处理同意好友申请
    QStringList handleFlushFriend(const char *name); // 处理同意好友申请
    bool handleDeleteFriend(const char *sourceName, const char *deleteName); // 删除好友申请
    QStringList handleGroupChat(const char *name);


signals:

public slots:
private:
     QSqlDatabase m_db;


};

#endif // OPEDB_H
