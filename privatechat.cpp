#include "privatechat.h"
#include "ui_privatechat.h"
#include "mainwindow.h"
#include <iostream>
#include <QTextBlock>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif


PrivateChat::PrivateChat(QWidget *parent, QString username, int serverSocket) : QWidget(parent), ui(new Ui::PrivateChat) {
    ui->setupUi(this);
    this->username = username;
    this->serverSocket = serverSocket;
}


PrivateChat::~PrivateChat() {
    delete ui;
}


QString PrivateChat::getUsername() {
    return this->username;
}

void PrivateChat::setLineSpacing(int lineSpacing) {

    int lineCount = 0;

    for (QTextBlock block = ui->chatWindow->document()->begin(); block.isValid(); block = block.next(), ++lineCount) {

        QTextCursor tc = QTextCursor(block);
        QTextBlockFormat fmt = block.blockFormat();

        if (fmt.topMargin() != lineSpacing || fmt.bottomMargin() != lineSpacing) {
            fmt.setBottomMargin(lineSpacing);
            tc.setBlockFormat(fmt);
        }
    }
}


void PrivateChat::appendChat(QString newMessage) {

    // Dodanie nowej wiadomości do okna czatu
    ui->chatWindow->append(newMessage);

    // Ustawienie przerwy między wyświetlanymi wiadomościami
    setLineSpacing(1);
}

void PrivateChat::sendMessageToServer (QString messageType, QString message) {

   // Utworzenie finalnej wiadomości
   QString finalMessage = messageType + ";" + username + ";" + message;

   // Przesłanie wiadomości do serwera
   send(serverSocket, finalMessage.toUtf8().constData(), finalMessage.toUtf8().length(), 0);
}



void PrivateChat::chatMessageRequest (QString message) {

    // Wyczyszczenie pola wiadomości
    ui->messageEdit->setText("");

    // Sprawdzenie czy wiadomość jest niepusta
    if (message.length() > 0 && message.at(0) != ' ')
        sendMessageToServer("2", message);
}

void PrivateChat::on_messageEdit_returnPressed() {
    chatMessageRequest(ui->messageEdit->text());
}
