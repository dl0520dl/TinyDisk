#include "tcpclient.h"

#include <QApplication>
#include "opewidget.h"
#include "online.h"
#include "friend.h"
#include "sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    tcpClient::getInstance().show();


//    ShareFile w;
//    w.show();

    return a.exec();
}
