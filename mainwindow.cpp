#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverlistener.h"

#include <QTextBlock>
#include <QStringListModel>
#include <QTime>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif


//--------------------------------------


/** Konstruktor klasy MainWindow (głownego okna czatu):
 *
 * Tworzy nowe wystąpienie obiektu z klasy Ui.
 *
 * @param *parent Wskaźnik na obiekt z klasy wyższego rzędu, opisuje kto jest rodzicem obiektu z klasy MainWindow
 *
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
}



/** Destruktor klasy MainWindow (głownego okna czatu):
 *
 * Usuwa obiekt z klasy Ui.
 *
 */
MainWindow::~MainWindow() {

    delete ui;
}



/** Funkcja ustawiająca socket klienta, oraz uruchamiająca nasłuchiwanie wiadomości od serwera:
 *
 * Parametr sck jest przesyłany z klasy SignIn
 * Następnie tworzony jest obiekt klasy ServerListener, który będzie nasłuchiwał, czy serwer
 * przesyła jakieś wiadomości od klienta, i obsługiwał przychodzące wiadomości
 *
 * @param sck Gniazdo niezbędne do komunikacji klienta z serwerem
 *
 */
void MainWindow::setSocket(int sck) {

    // Ustawienie socketu do komunikacji z serwerem
    serverSocket = sck;

    // Utworzenie obiektu odpowiedzialnego za nasłuchiwanie serwera
    ServerListener *sr = new ServerListener(sck, this);

    // Uruchomienie nasłuchiwania, które będzie wykonywane w osobnym wątku
    sr->start();
}



/** Funkcja parsująca otrzymany tekst do listy danych:
 *
 * Funkcja ta otrzymuje jako parametr odpowiednio zapisane
 * dane przedzielone średnikami. Następnie szuka danych spośród
 * średników, dodaje je jako kolejne elementy listy i na koniec
 * zwraca całą listę.
 *
 * @param list Zakodowana lista (rozdzielona średnikami)
 * @return Lista w czytelnym dla środowiska Qt modelu
 *
 */
QStringListModel* MainWindow::parseList(QString list) {

    // Utworzenie obiektu przechowującego dane o liście danych
    QStringListModel *listModel = new QStringListModel();

    // Zmienna przechowująca naszą listę danych
    QStringList returnList;

    for (int i = 0 ; i <= list.count(separator); i++) {
        returnList << list.section(separator, i, i);  // Dodawanie kolejnych danych do listy
    }

    // Ustawienie listy we wcześniej utworzonym modelu
    listModel->setStringList(returnList);

    return listModel;
}



/** Funkcja odświeżająca listę użytkowników w danym pokoju:
 *
 * Wywoływana gdy serwer prześle nową zaktualizowaną listę
 * dostępnych użytkowników w danym pokoju czatu.
 *
 * Funkcja wstawia odpowiednio sparsowane dane do obiektu
 * z klasy QListView, czyli obiektu wyświetlającego naszą listę
 * w głównym oknie.
 *
 * @param usersList Zwracana przez serwer zakodowana lista użytkowników w danym pokoju czatu
 *
 */
void MainWindow::refreshUsersList(QString usersList) {

    // Ustawienie nowego modelu (zwróconego z funkcji parsującej) dla obiektu z klasy QListView
    ui->usersList->setModel(parseList(usersList));
}



/** Funkcja odświeżająca listę dostępnych pokojów czatu:
 *
 * Wywoływana gdy serwer prześle nową zaktualizowaną listę
 * dostępnych pokojów do czatowania.
 *
 * Funkcja ta parsuje przekazane przez serwer dane, a następnie
 * wstawia je do odpowiedniego obiektu w naszej klasie.
 *
 * @param chatRoomsList Zwracana przez serwer zakodowana lista pokojów czatu
 *
 */
void MainWindow::refreshChatRooms(QString chatRoomsList) {

    // Ustawienie nowego modelu (zwróconego z funkcji parsującej) dla obiektu z klasy QListView
    ui->chatRoomsList->setModel(parseList(chatRoomsList));
}



/** Funkcja dodająca tekst do czatu:
 *
 * Wywoływana gdy serwer prześle nową wiadomość dla
 * czatu, w którym znajduje się klient.
 *
 * Wiadomość jest zapisana jako dokument HTML i jest
 * formatowana po stronie serwera.
 *
 * @param newMessage Nowa wiadomość, która zostanie dopisana do okna czatu
 *
 */
void MainWindow::appendChat(QString newMessage) {

    // Dodanie nowej wiadomości do okna czatu
    ui->chatWindow->append(newMessage);

    // Ustawienie przerwy między wyświetlanymi wiadomościami
    setLineSpacing(3);
}



/** Funkcja wykorzystywana do przesyłania danych do serwera:
 *
 * Wywołując tę funkcję wysyłamy wiadomość o określonym typie
 * do serwera
 *
 * @param messageType Typ przesyłanej wiadomości
 * @param message Przesyłana przez nas wiadomość
 *
 */
void MainWindow::sendMessageToServer (QString messageType, QString message) {

    // Utworzenie finalnej wiadomości
    QString finalMessage = messageType + ";" + message;

    // Przesłanie wiadomości do serwera
    send(serverSocket, finalMessage.toUtf8().constData(), finalMessage.toUtf8().length(), 0);
}



/** Funkcja uruchamiana przez klienta aby wysłać wiadomość na czacie:
 *
 * Sprawdza czy próbujemy przesłać prawidłową wiadmość (niepustą).
 * Jeżeli tak to przesyłamy wiadomość do serwera.
 *
 * @param message Przekazywana przez nas wiadomość czatu
 *
 */
void MainWindow::chatMessageRequest (QString message) {

    // Wyczyszczenie pola wiadomości
    ui->messageEdit->setText("");

    // Sprawdzenie czy wiadomość jest niepusta
    if (message.length() > 0 && message.at(0) != ' ')
        sendMessageToServer("1", message);
}



/** Funkcja ustawiająca przerwy między wyświetlanymi w oknie czatu wiadomościami:
 *
 * Dzięki wbudowanym funkcją biblioteki Qt ustawiamy przerwy
 *
 * @param lineSpacing Wielkość przerwy między wiadomościami
 *
 */
void MainWindow::setLineSpacing(int lineSpacing) {

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


//----------- OBSŁUGA SYGNAŁÓW ------------


/** Funkcja uruchamiana, gdy zostanie naciśnięty enter podczas pisania wiadomości:
 *
 * Przesyła informację, że gotowa jest nowa wiadomość do wysyłki do serwera.
 *
 */
void MainWindow::on_messageEdit_returnPressed() {

    // Prośba o wysłanie wiadomości do serwera
    chatMessageRequest(ui->messageEdit->text());
}



/** Funkcja uruchamiana, gdy zostanie naciśnięty przycisk przesłania wiadomości:
 *
 * Przesyła informację, że gotowa jest nowa wiadomość do wysyłki do serwera.
 *
 */
void MainWindow::on_sendMessageButton_clicked() {

    // Prośba o wysłanie wiadomości do serwera
    chatMessageRequest(ui->messageEdit->text());
}



/** Funkcja uruchamiana gdy "podwójnie" klikniemy na obiekt na liście użytkowników:
 *
 * Przesyła informację do serwera o wybraniu użytkownika
 *
 * @param &index Adres obiektu przechowującego informacje o klikniętym przez nas rekordzie.
 */
void MainWindow::on_usersList_doubleClicked(const QModelIndex &index) {

    // Wysłanie wiadomości do serwera z odpowiednio wybranym użytkownikiem
    sendMessageToServer("2", index.data(Qt::DisplayRole).toString());
}


/** Funkcja uruchamiana gdy "podwójnie" klikniemy na obiekt na liście czatów:
 *
 * Przesyła informację do serwera o wybraniu nowego czatu.
 *
 * @param &index Adres obiektu przechowującego informację o klikniętym przez nas rekordzie.
 *
 */
void MainWindow::on_chatRoomsList_doubleClicked(const QModelIndex &index) {

    // Zmiana nazwy okna na nowy pokój czatu
    this->setWindowTitle(index.data(Qt::DisplayRole).toString());

    // Wyczyszczenie okna rozmowy
    ui->chatWindow->clear();

    // Wysłanie wiadomości do serwera o wybraniu nowego pokoju czatu
    sendMessageToServer("3", index.data(Qt::DisplayRole).toString());
}



