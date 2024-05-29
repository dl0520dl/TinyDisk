#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class tcpClient;
}
QT_END_NAMESPACE

class tcpClient : public QWidget
{
    Q_OBJECT

public:
    tcpClient(QWidget *parent = nullptr);
    ~tcpClient();
    void loadConfig();

    static tcpClient &getInstance();

    QTcpSocket &gettcpSocket();

    QString loginName();
    QString currentPath();
    void setCurrentPath(QString preContentPath);


public slots:
    void showConnect();
    void recvMsg();

private slots:
   // void on_send_pb_clicked();

    void on_login_clicked();

    void on_zhuce_clicked();

    void on_zhuxiao_clicked();

private:
    Ui::tcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;

    QTcpSocket m_tcpSocket;
    QString m_strLoginName;
    QString m_strCurPath;

    QFile m_file;
};
#endif // TCPCLIENT_H
