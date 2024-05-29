#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>

#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局
#include <QButtonGroup>
#include <QScrollArea>
#include <QListWidget>
#include <QCheckBox>

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile &getInstance();

    void updateFriend(QListWidget *pFriendList);

signals:

public slots:
    void cancelSelect();
    void selectAll();
    void shareConfirm();
    void shareCancel();

private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancelSelectPB;

    QPushButton *m_pConfirmPB;
    QPushButton *m_pCancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QVBoxLayout *m_pFriendWVBL;
    QButtonGroup *m_pButtonGroup;

};

#endif // SHAREFILE_H
