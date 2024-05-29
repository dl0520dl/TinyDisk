#include "tcpserver.h"
#include <QApplication>

#include "opedb.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpeDB::getInstance().init();
    tcpServer w;
    w.show();
    return a.exec();
}
