#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "privatechat.h"
#include <QMainWindow>
#include <QStringListModel>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, QString username = NULL);
    ~MainWindow();
     void setSocket(int sck);

private slots:
    void on_messageEdit_returnPressed();
    void chatMessageRequest(QString message);
    void sendMessageToServer (QString messageType, QString message);
    void setLineSpacing(int lineSpacing);
    void on_usersList_doubleClicked(const QModelIndex &index);
    void on_chatRoomsList_doubleClicked(const QModelIndex &index);
    void on_sendMessageButton_clicked();
    QStringListModel* parseList(QString list);

public slots:
    void refreshUsersList(QString userList);
    void appendChat(QString newMessage);
    void appendPrivateChat (QString newMessage);
    void refreshChatRooms(QString chatList);
    void openNewPrivateChat(QString username);

private:
    Ui::MainWindow *ui;
    QString separator          = ";";
    QString chatType           = "1";
    QString usersListType      = "2";
    QString chatRoomsListType  = "3";
    int serverSocket;
    QString username;

    QVector<PrivateChat*> privateChatsVector;
};

#endif // MAINWINDOW_H
