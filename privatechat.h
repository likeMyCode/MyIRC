#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include <QString>
#include <QDesktopServices>


namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget {
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = 0, QString username = NULL, int serverSocket = -1);
    ~PrivateChat();
    QString getUsername();

private slots:
    void on_messageEdit_returnPressed();
    void sendMessageToServer (QString messageType, QString message);
    void chatMessageRequest (QString message);
    void setLineSpacing(int lineSpacing);

public slots:
    void appendChat (QString newMessage);

private:
    Ui::PrivateChat *ui;
    QString username;
    int serverSocket;

};

#endif // PRIVATECHAT_H
