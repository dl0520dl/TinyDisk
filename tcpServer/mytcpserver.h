#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
private:

public:
    MyTcpServer();
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);

    void resend(const char *pername, PDU *pdu);

public slots:
    void deleteSocket(MyTcpSocket *mysocket);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
