#include "mytcpsocket.h"
#include <QDebug>
#include <mytcpserver.h>
#include <QDir>


MyTcpSocket::MyTcpSocket()
{
    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected()), this, SLOT(clientOffline()));
    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(sendFileToClient()));

}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();
    QString srcTmp;
    QString destTmp;
    for(int i = 0; i < fileInfoList.size(); i++)
    {
        if(fileInfoList[i].isFile())
        {
            srcTmp += strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTmp, destTmp);
        }
        else if(fileInfoList[i].isDir())
        {
            if(QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }
            srcTmp += strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            copyDir(srcTmp, destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload)
    {
        qDebug() << this->bytesAvailable();
        unit uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(unit));
        unit uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu + sizeof(unit), uiPDULen - sizeof(unit));
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            // 拷贝读取的信息
            strncpy(caName, pdu -> caData, 32);
            strncpy(caPwd, pdu -> caData + 32, 32);
            bool ret = OpeDB::getInstance().handleRegist(caName, caPwd); //处理请求，插入数据库
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, REGIST_OK);
                QDir dir;
                dir.mkdir(QString("./%1").arg(caName));
            }
            else
            {
                strcpy(respdu->caData, REGIST_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;

        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            // 拷贝读取的信息
            strncpy(caName, pdu -> caData, 32);
            strncpy(caPwd, pdu -> caData + 32, 32);
            bool ret = OpeDB::getInstance().handleLogin(caName, caPwd); //处理请求，插入数据库
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, LOGIN_OK);
                m_strName = caName;
            }
            else
            {
                strcpy(respdu->caData, LOGIN_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            unit uiMsgLen = ret.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i = 0; i < ret.size(); i++){
                memcpy((char*)(respdu->caMsg) + i*32, ret.at(i).toStdString().c_str(),
                       ret.at(i).size());
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:
        {
            int ret = OpeDB::getInstance().handleSearchUser(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
            if(-1 == ret){
                strcpy(respdu->caData, SEARCH_USER_NO);
            }else if(1 == ret){
                strcpy(respdu->caData, SEARCH_USER_ONLINE);
            }else if(0 == ret){
                strcpy(respdu->caData, SEARCH_USER_OFFLINE);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            // 拷贝读取的信息
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData + 32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
            if(-1 == ret){          //未知错误
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, UNKNOWN_ERROR);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(0 == ret){     //该好友已经存在
            //  respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, EXISTED_FRIEND);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(1 == ret){     //
                MyTcpServer::getInstance().resend(caPerName, pdu);
            //  respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, ADD_FRIEND_OK); // 表示加好友请求已发送
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(2 == ret){     //不在线
            //  respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData, ADD_FRIEND_OFFLINE );
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(3 == ret){     //不存在此人
             // respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData ,ADD_FRIEND_NO_EXISTED);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            char addedName[32] = {'\0'};
            char sourceName[32] = {'\0'};
            // 拷贝读取的信息
            strncpy(addedName, pdu->caData, 32);
            strncpy(sourceName, pdu->caData + 32, 32);
            PDU *respdu = mkPDU(0);
            OpeDB::getInstance().handleAddFriendAgree(addedName, sourceName);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            // 将新的好友关系信息写入数据库

            // 服务器需要转发给发送好友请求方其被同意的消息
            MyTcpServer::getInstance().resend(sourceName, pdu);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            char sourceName[32] = {'\0'};
               // 拷贝读取的信息
            strncpy(sourceName, pdu -> caData + 32, 32);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            // 服务器需要转发给发送好友请求方其被拒绝的消息
            MyTcpServer::getInstance().resend(sourceName, pdu);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            // 拷贝读取的信息
            char sourceName[32] = {'\0'};
            strncpy(sourceName, pdu->caData, 32);
            QStringList strList = OpeDB::getInstance().handleFlushFriend(sourceName);
            unit uiMsgLen = strList.size() / 2 * 36; // 36 char[32] 好友名字+ 4 int 在线状态
            PDU* respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i = 0; i * 2 < strList.size(); i++)
            {
                memcpy((char*)(respdu->caMsg) + 36 * i, strList.at(i * 2).toStdString().c_str(), 32);
                memcpy((char*)(respdu->caMsg) + 36 * i + 32, strList.at(i * 2 + 1).toStdString().c_str(), 4);

            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
         case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char sourceName[32] = {'\0'};
            char deleteName[32] = {'\0'};
            strncpy(sourceName, pdu->caData, 32);
            strncpy(deleteName, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleDeleteFriend(sourceName, deleteName);

            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, DELETE_OK);
            }
            else
            {
                strcpy(respdu->caData, DELETE_FAILED);
            }
            MyTcpServer::getInstance().resend(deleteName, pdu);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char sourceName[32] = {'\0'};
            char chatName[32] = {'\0'};
            strncpy(sourceName, pdu->caData, 32);
            strncpy(chatName, pdu->caData + 32, 32);
            MyTcpServer::getInstance().resend(chatName, pdu);
            MyTcpServer::getInstance().resend(sourceName, pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char sourceName[32] = {'\0'};
            strncpy(sourceName, pdu->caData, 32);
            QStringList onlineFriend = OpeDB::getInstance().handleGroupChat(sourceName);
            QString tmp;
            for(int i = 0; i * 2 < onlineFriend.size(); i++)
            {
                tmp = onlineFriend.at(i * 2);
                MyTcpServer::getInstance().resend(tmp.toStdString().c_str(), pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)pdu->caMsg);
            bool ret = dir.exists(QString(strCurPath));
            qDebug() << "strcurpath: " << strCurPath;
            PDU *respdu = mkPDU(0);
            if(ret)
            {
                char strNewPath[32] = {'\0'};
                memcpy(strNewPath, pdu->caData + 32, 32);
                QString newPath = strCurPath + "/" + strNewPath;
                qDebug() << newPath;
                bool isExist = dir.exists(newPath);
                if(isExist)
                {
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, DIR_ALREADY_EXSIT);
                }
                else
                {   respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, CREATE_DIR_OK);
                    dir.mkdir(newPath);
                }
            }
            else
            {
               respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
               strcpy(respdu->caData, DIR_N0_EXSIT);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);    //当前的路径
            qDebug() << "刷新请求的路径：" << pCurPath;
            QDir dir(pCurPath);
            QFileInfoList fileList = dir.entryInfoList();
            int fileCount = fileList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo) * (fileCount));
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            //遍历当前路径下所有的文件和文件夹
            for(int i = 0; i < fileList.size(); i++){
                //qDebug() << fileList.at(i).fileName() << " " << fileList.at(i).size()
                //        << "文件夹:" << fileList.at(i).isDir() << "常规文件：" << fileList.at(i).isFile();
                pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                strFileName = fileList[i].fileName();
                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                if(fileList[i].isDir())
                {
                    pFileInfo->iFileType = 0;
                }
                else if(fileList[i].isFile())
                {
                    pFileInfo->iFileType = 1;
                }
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
        {
            char strDirName[32] = {'\0'};
            strcpy(strDirName, pdu->caData);
            char *pDirPath = new char[pdu->uiMsgLen];
            memcpy(pDirPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pDirPath).arg(strDirName);
            qDebug() << strPath;
            bool ret = false;
            QFileInfo fileInfo(strPath);
            if(fileInfo.isDir())
            {
                QDir dir;
                dir.setPath(strPath);
                ret = dir.removeRecursively();
            }
            else if(fileInfo.isFile())
            {
                ret = false;
            }
            PDU *respdu;
            if(ret)
            {
                respdu = mkPDU(strlen(DIR_DELETE_OK) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                memcpy(respdu->caData, DIR_DELETE_OK, strlen(DIR_DELETE_OK));
            }
            else
            {
                respdu = mkPDU(strlen(DIR_DELETE_FAILED) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                memcpy(respdu->caData, DIR_DELETE_FAILED, strlen(DIR_DELETE_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char oldFileName[32] = {'\0'};
            char newFileName[32] = {'\0'};
            strncpy(oldFileName, pdu->caData, 32);
            strncpy(newFileName, pdu->caData + 32, 32);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strOldPath = QString("%1/%2").arg(pPath).arg(oldFileName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(newFileName);
            qDebug() << "oldpath:" << strOldPath << "newpath:" << strNewPath;
            bool ret = false;

            QDir dir;
            ret = dir.rename(strOldPath, strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret)
            {
                memcpy(respdu->caData, RENAME_FILE_OK, strlen(RENAME_FILE_OK));
            }
            else
            {
                memcpy(respdu->caData, RENAME_FILE_FAILED, strlen(RENAME_FILE_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterDirName[32] = {'\0'};
            strncpy(caEnterDirName, pdu->caData, 32);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caEnterDirName);
            qDebug() << strNewPath;
            QFileInfo fileInfo(strNewPath);
            if(fileInfo.isDir())
            {
                QDir dir(strNewPath);
                QFileInfoList fileList = dir.entryInfoList();
                int fileCount = fileList.size();
                PDU *respdu = mkPDU(sizeof(FileInfo) * (fileCount));
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo;
                QString strFileName;
                for(int i = 0; i < fileList.size(); i++){
                    pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                    strFileName = fileList[i].fileName();
                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                    if(fileList[i].isDir())
                    {
                        pFileInfo->iFileType = 0;
                    }
                    else if(fileList[i].isFile())
                    {
                        pFileInfo->iFileType = 1;
                    }
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            else if(fileInfo.isFile())
            {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                memcpy(respdu->caData, ENTER_DIR_FAILED, strlen(ENTER_DIR_FAILED));
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }

            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char uploadFileName[32] = {'\0'};
            qint64 uploadFileSize = 0;
            sscanf(pdu->caData, "%s %lld", uploadFileName, &uploadFileSize);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(uploadFileName);
            qDebug() << strNewPath;
            delete []pPath;
            pPath = NULL;

            m_file.setFileName(strNewPath);
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = uploadFileSize;
                m_iReceived = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
        {
            char strFileName[32] = {'\0'};
            strcpy(strFileName, pdu->caData);
            char *pDirPath = new char[pdu->uiMsgLen];
            memcpy(pDirPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pDirPath).arg(strFileName);
            qDebug() << strPath;
            bool ret = false;
            QFileInfo fileInfo(strPath);
            if(fileInfo.isDir())
            {
                 ret = false;
            }
            else if(fileInfo.isFile())
            {

                QDir dir;
                ret = dir.remove(strPath);
            }
            PDU *respdu;
            if(ret)
            {
                respdu = mkPDU(strlen(FILE_DELETE_OK) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                memcpy(respdu->caData, FILE_DELETE_OK, strlen(FILE_DELETE_OK));
            }
            else
            {
                respdu = mkPDU(strlen(FILE_DELETE_FAILED) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                memcpy(respdu->caData, FILE_DELETE_FAILED, strlen(FILE_DELETE_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            strcpy(caFileName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caFileName);

            qDebug() << "当前下载文件的地址："  << strNewPath;
            delete []pPath;
            pPath = NULL;

            QFileInfo fileInfo(strNewPath);
            qint64 fileSize = fileInfo.size();
            qDebug() << "caFileName" << caFileName << "filesize" << fileSize;
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData, "%s %lld", caFileName, fileSize);

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            m_file.setFileName(strNewPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char strSendName[32] = {'\0'};
            int shareNum = 0;
            sscanf(pdu->caData, "%s %d", strSendName, &shareNum);
            qDebug() << "分享文件的人：" << strSendName << " 人数：" << shareNum;
            int size = shareNum * 32;
            PDU *respdu = mkPDU(pdu->uiMsgLen - size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST;

            strcpy(respdu->caData, strSendName);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg) + size, pdu->uiMsgLen - size); //文件路径
            qDebug() << "接收到文件的路径为：" << respdu->caMsg;
            char caReceiveName[32] = {'\0'};
            for(int i = 0; i < shareNum; i++)
            {
                 memcpy(caReceiveName, (char*)(pdu->caMsg) + i * 32, 32);
                 qDebug() << "接收到文件的好友为：" << caReceiveName;
                 MyTcpServer::getInstance().resend(caReceiveName, respdu);
            }
            free(respdu);
            respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData, SHARE_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            QString strReceivePath = QString("./%1").arg(pdu->caData);
            QString strSharePath = QString("%1").arg((char*)(pdu->caMsg));

            int index = strSharePath.lastIndexOf('/');
            QString strFileName = strSharePath.right(strSharePath.size() - index - 1);
            qDebug() << "被分享者的路径：" << strReceivePath;
            qDebug() << "被分享的文件名：" << strFileName;
            strReceivePath = strReceivePath + '/' + strFileName;
            qDebug() << "被分享者的路径：" << strReceivePath;
            QFileInfo fileInfo(strSharePath);
            if(fileInfo.isDir())
            {
                copyDir(strSharePath, strReceivePath);
            }
            else if(fileInfo.isFile())
            {
                QFile::copy(strSharePath, strReceivePath);
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData, "%d%d%s", &srcLen, &destLen, caFileName);
            char *pSrcPath = new char[srcLen + 1];
            char *pDestPath = new char[destLen + 1 + 32];
            memset(pSrcPath, '\0', srcLen + 1);
            memset(pDestPath, '\0', destLen + 1 + 32);

            memcpy(pSrcPath, pdu->caMsg, srcLen);
            memcpy(pDestPath, (char*)(pdu->caMsg) + (srcLen + 1), destLen);

            QFileInfo fileInfo(pDestPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            if(fileInfo.isDir())
            {
                strcat(pDestPath, "/");
                strcat(pDestPath, caFileName);

                bool ret = QFile::rename(pSrcPath, pDestPath);
                if(ret)
                {
                    strcpy(respdu->caData, MOVE_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData, COMMON_ERROR);
                }

            }
            else if(fileInfo.isFile())
            {
                strcpy(respdu->caData, MOVE_FILE_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }


        default:
            break;
        }
        free(pdu);
        pdu = NULL;
    }
    else
    {
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        QByteArray buffer = readAll();
        m_file.write(buffer);
        m_iReceived += buffer.size();
        if(m_iTotal == m_iReceived)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iReceived)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILED);
        }

    }
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    char *buffer = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(buffer, 4096);
        if(ret > 0 && ret <= 4096)
        {
            write(buffer, ret);
        }
        else if(0 == ret)
        {
            m_file.close();
            break;
        }
        else if(ret < 0)
        {
            qDebug() << "发送文件内容给Client失败";
            m_file.close();
            break;
        }
    }
    delete []buffer;
    buffer = NULL;
}
