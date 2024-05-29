#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局
#include "protocol.h"
#include <QTimer>

#include "online.h"
#include "protocol.h"

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(PDU *pdu);
    void clearEnterDir();
    QString enterDir();

    void setDownLoadStatus(bool status);

    qint64 m_iTotal = 0;
    qint64 m_iReceived = 0;

    bool getDownloadStatus();
    QString getFileSavePath();
    QString getShareFileName();

signals:

public slots:
    void createDir();
    void flushFile();
    void deleteDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPreContent();
    void uploadFile();
    void uploadFileData();
    void deleteFile();

    void downloadFile();
    void shareFile();

    void moveFile();
    void selectDestDir();


private:
    QListWidget *m_pFileListWidget;         // 文件列表
    QPushButton *m_pReturnPB;               // 返回主页面
    QPushButton *m_pCreateDirPB;            // 新建文件夹
    QPushButton *m_pDeleteDirPB;            // 删除文件夹
    QPushButton *m_pRenameFilePB;           // 重命名文件
    QPushButton *m_pFlushFilePB;            // 刷新文件
    QPushButton *m_pUploadFilePB;           // 上传文件
    QPushButton *m_pDownLoadFilePB;         // 下载文件
    QPushButton *m_pDeleteFilePB;           // 删除文件
    QPushButton *m_pShareFilePB;            // 分享文件
    QPushButton *m_pMoveFilePB;             //移动文件
    QPushButton *m_pSelectMoveToDirPB;      //移动文件到其他文件夹

    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QString m_strFileSavePath;
    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDirPath;

    QTimer *m_pTimer;
    bool m_bDownload;
};

#endif // BOOK_H
