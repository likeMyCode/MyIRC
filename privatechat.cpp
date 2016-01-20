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



/*** Konstruktor nowego prywatnego czatu
 * 
 * @param parent Wskaźnik na rodzica okna czatu, domyslnie 0
 * @param username Nazwa użytkownika z którym prowadzimy prywatna rozmowe
 * @param serverSocket Gniazdo do komunikacji siecowej
 * 
 */
PrivateChat::PrivateChat(QWidget *parent, QString username, int serverSocket) : QWidget(parent), ui(new Ui::PrivateChat) {
    
    ui->setupUi(this);
    this->username = username;
    this->serverSocket = serverSocket;
}



/*** Destruktor okna prywatnego czatu
 * 
 */
PrivateChat::~PrivateChat() {
    
    delete ui;
}



/*** Funkcja zwracajaca nazwe uzytkownika z ktorym rozmawiamy
 * 
 * @return Nazwa uzytkownika z ktorym rozmawiamy
 * 
 */
QString PrivateChat::getUsername() {
    
    return this->username;
}



/*** Ustawienie odstepow miedzy kolejnymi wiadomosciami
 * 
 * Wywolywana gdy dodajemy nowy tekst do czatu.
 * 
 * @param lineSpacing Szerokosc miedzy kolejnymi liniami [px]
 * 
 */
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



/*** Funkcja dodajaca nowa wiadomosc do okna czatu
 * 
 * Wywolywana gdy serwer przesle informacje o nowej wiadomosci
 * 
 * @param newMessage Otrzymana wiadomosc
 * 
 */
void PrivateChat::appendChat(QString newMessage) {

    // Dodanie nowej wiadomości do okna czatu
    ui->chatWindow->append(newMessage);

    // Ustawienie przerwy między wyświetlanymi wiadomościami
    setLineSpacing(1);
}



/*** Funkcja przesylajaca napisany przez nas tekst do serwera
 * 
 * Wywolywana, gdy nacisniemy przycisk send, lub nacisniemy Enter
 * 
 * @param messageType Typ przesylanej wiadomosci, w tej sytuacji 2
 * @param message Tresc wiadomosci
 * 
 */
void PrivateChat::sendMessageToServer (QString messageType, QString message) {

   // Utworzenie finalnej wiadomości
   QString finalMessage = messageType + ";" + username + ";" + message;

   // Przesłanie wiadomości do serwera
   send(serverSocket, finalMessage.toUtf8().constData(), finalMessage.toUtf8().length(), 0);
}



/*** Proba wyslana wiadomosci do serwera
 * 
 * Sprawdzenie czy przesylana wiadomosc spelnia zakladane 
 * wymogi prawidlowej wiadomosci. Jezeli tak to nastepuje
 * przeslanie jej dalej.
 * 
 * @param message Wiadomosc ktora chcemy przeslac
 * 
 */
void PrivateChat::chatMessageRequest (QString message) {

    // Wyczyszczenie pola wiadomości
    ui->messageEdit->setText("");

    // Sprawdzenie czy wiadomość jest niepusta
    if (message.length() > 0 && message.at(0) != ' ')
        sendMessageToServer("2", message);
}



/*** Wywolywana gdy uzytkownik nacisnie Enter piszac tekst
 * 
 * Przesyla informacje o checi wyslana wiadomosci do 
 * innego uzytkownika
 * 
 */
void PrivateChat::on_messageEdit_returnPressed() {
    chatMessageRequest(ui->messageEdit->text());
}
