#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class tcpServer;
}

//QT_END_NAMESPACE

class tcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit tcpServer(QWidget *parent = nullptr);
    ~tcpServer();
    void loadConfig();

private:
    Ui::tcpServer *ui;

    QString m_strIP;
    quint16 m_usPort;
};
#endif // TCPSERVER_H
