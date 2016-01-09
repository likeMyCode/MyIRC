#include "serverlistener.h"
#include "mainwindow.h"

#include <QString>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


//-------------------------------

/** Konstruktor klasy ServerListener (nasłuchiwanie serwera):
 *
 * Tworzy nowe wystąpienie obiektu z klasy Ui.
 *
 * @param *parent Wskaźnik na obiekt z klasy wyższego rzędu, opisuje kto jest rodzicem obiektu z klasy ServerListener
 *
 */
ServerListener::ServerListener(int socket, MainWindow *w) {

    window = w;
    serverSocket = socket;
}

/** Funkcja wykonująca odpowiednią akcję na podstawie otrzymanej wiadomośći:
 *
 * @param message Wiadomość otrzymana od serwera
 *
 */
void ServerListener::parseMessage(QString message) {

    // Badanie typu wiadomości
    if (message.section(";", 0, 0) == "1") {

        // Dodanie otrzymanej wiadomości do okna czatu
        QMetaObject::invokeMethod(window, "appendChat", Qt::AutoConnection, Q_ARG(QString, message.section(";", 1, -1)));
    } else if (message.section(";", 0, 0) == "2") {

        // Odświeżenie listy użytkowników
        QMetaObject::invokeMethod(window, "refreshUsersList", Qt::AutoConnection, Q_ARG(QString, message.section(";", 1, -1)));
    } else if (message.section(";", 0, 0) == "3") {

        // Odświeżenie listy pokojów czatu
        QMetaObject::invokeMethod(window, "refreshChatRooms", Qt::AutoConnection, Q_ARG(QString, message.section(";", 1, -1)));
    }
}

/** Funkcja nasłuchująca czy przyszły jakieś wiadomości od serwera:
 *
 * Uruchumiona w osobnym wątku, aby możlwie było jednoczesne zarządzanie GUI i nasłuchiwanie serwera
 *
 */
void ServerListener::run() {

    while (1) {
        char server_reply[1024];
        memset(&server_reply, 0, 1024);

        // Pobieramy dwie pierwsze znaki wiadomości aby określić typ wiadomości
        recv(serverSocket , server_reply , 2 , 0);

        // Dopisujemy typ wiadomości do wiadomości końcowej
        QString finalMessage = QString::fromUtf8(server_reply);
        QString messageSize = "";
        QString lastCharacter = "";

        bool ok;

        // Dopóki nie napotkamy kolejnego średnika
        do {

            // Dopisujemy ostatnio pobrany znak do zmiennej tekstowej przechowującej rozmiar wiadomości
            messageSize += lastCharacter;

            memset(&server_reply, 0, 1024);

            // Odbieramy jeden znak (dopóki nie odbierzemy średnika)
            recv(serverSocket, server_reply, 1, 0);

            // Zapisujemy jaki znak ostatnio pobraliśmy
            lastCharacter = QString::fromUtf8(server_reply);

        } while (lastCharacter != ";");

        memset(&server_reply, 0, 1024);

        // Pobieramy wiadomość o takim rozmiarze jaki został przesłany w wiadomości wcześniej
        recv(serverSocket, server_reply, messageSize.toInt(&ok, 10) - 1, 0);

        // Dopisujemy pobraną wiadomość do wiadomości końcowej
        finalMessage += server_reply;

        // Obsługujemy otrzymaną wiadomość
        parseMessage(finalMessage);
    }
}
