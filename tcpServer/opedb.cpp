#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\Qt\\tcpServer\\cloud.db");
    if(m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from userInfo");
         while(query.next())
         {
            QString data = QString("%1, %2, %3, %4").arg(query.value(0).toString()).arg(query.value(1).toString())
               .arg(query.value(2).toString()).arg(query.value(3).toString());
            qDebug() << data;
         }
    }
    else
    {
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败");
    }
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(name == NULL || pwd == NULL){
        return false;
    }
    QString data = QString("insert into userInfo(name, pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(name == NULL || pwd == NULL){
        return false;
    }
    QString data = QString("select * from userInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        data = QString("update userInfo set online=1 where name=\'%1\' and pwd=\'%2\'").arg(name).arg(pwd);
        QSqlQuery query;
        query.exec(data);
        return true;
    }
    else{
        return false;
    }
}

void OpeDB::handleOffline(const char *name)
{
    if(name == NULL){
        return;
    }
    QString data = QString("update userInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from userInfo where online=1");
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while(query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUser(const char *name)
{
    if(NULL == name){
        return -1;
    }
    QString data = QString("select online from userInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    //存在这个人
    if(query.next()){
        int ret = query.value(0).toInt();
        if(ret == 1){
            return 1;   //此人在线
        }else if(ret == 0){
            return 0;   //此人不在线
        }
    }
    else{
        return -1;      //没这个人
    }
    return -1;
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(NULL == pername || NULL == name){
        return -1;
    }
    QString data = QString("select * from friend"
                                   " where (id = (select id from userInfo where name = \'%1\') and "
                                   "friendId = (select id from userInfo where name = \'%2\')) or "  // 好友是双向的，数据库只存了单向，注意是or关系
                                   "(id = (select id from userInfo where name = \'%3\') and "
                                   "friendId = (select id from userInfo where name = \'%4\'))")
                .arg(pername).arg(name).arg(name).arg(pername);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return 0;   //双方已经是好友
    }
    else            //不是好友
    {
        QString data = QString("select online from userInfo where name=\'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if(query.next()){
            int ret = query.value(0).toInt();
            if(ret == 1){
                return 1;   //此人在线
            }else if(ret == 0){
                return 2;   //此人不在线
            }
        }
        else{
            return 3;      //没这个人
        }
    }
    return -1;
}

int OpeDB::getIdByUserName(const char *name)
{
    if(NULL == name)
    {
        return -1;
    }
    QString data = QString("select id from userInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    qDebug() << data;
    if(query.next())
    {
        return query.value(0).toInt();
    }
    else
    {
        return -1; // 不存在该用户
    }
}

void OpeDB::handleAddFriendAgree(const char *addedName, const char *sourceName)
{
    if(NULL == addedName || NULL == sourceName)
    {
        return;
    }
    int sourceUserId = -1;
    int addedUserId = -1;
    sourceUserId = getIdByUserName(sourceName);
    addedUserId = getIdByUserName(addedName);
    qDebug() << sourceUserId << " " << addedUserId;
    QString strQuery = QString("insert into friend values(%1, %2) ").arg(sourceUserId).arg(addedUserId);
    QSqlQuery query;
    query.exec(strQuery);
    return;

    //qDebug() << "handleAddFriendAgree " << strQuery;
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(NULL == name)
    {
        return strFriendList;
    }
    QString data = QString("select id from userInfo where name = \'%1\' and online = 1 ").arg(name);
    QSqlQuery query;
    query.exec(data);
    int sourceId = -1; // 请求方name对应的id

    if (query.next())
    {
        sourceId = query.value(0).toInt();
    }

    data = QString("select name, online from userInfo where id in (select id from friend where friendId= \'%1\') or id in (select friendId from friend where id = \'%2\')").arg(sourceId).arg(sourceId);
    query.exec(data);
    qDebug() << data;
    //既要返回好友的名字，同时返回好友的在线状态
    while(query.next())
    {
        qDebug() << "yes";
        char friendName[32] = {'\0'};
        char friendOnlineStatus[4] = {'\0'};
        memcpy(friendName, query.value(0).toString().toStdString().c_str(), 32);
        memcpy(friendOnlineStatus, query.value(1).toString().toStdString().c_str(), 4);
        strFriendList.append(friendName);
        strFriendList.append(friendOnlineStatus);
        qDebug() << "好友信息 " << friendName << " " << friendOnlineStatus;
        //qDebug() << strFriendList;
    }
    return strFriendList;
}

QStringList OpeDB::handleGroupChat(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(NULL == name)
    {
        return strFriendList;
    }
    strFriendList = handleFlushFriend(name);
    strFriendList.append(name);
    strFriendList.append("1");
    return strFriendList;
}

bool OpeDB::handleDeleteFriend(const char *sourceName, const char *deleteName)
{
    if(NULL == sourceName || NULL == deleteName)
    {
        return false;
    }
    int sourceId = getIdByUserName(sourceName);
    int deleteId = getIdByUserName(deleteName); // 请求方name对应的id
    qDebug() << sourceId << deleteId;
    QString data = QString("delete from friend where (id=\'%1\' and friendId=\'%2\') or (id=\'%3\' and friendId =\'%4\')").arg(sourceId).arg(deleteId).arg(deleteId).arg(sourceId);
    QSqlQuery query;
    query.exec(data);
    qDebug() << data;
    return true;
}
