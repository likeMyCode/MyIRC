#include "signin.h"
#include "ui_signin.h"
#include "mainwindow.h"

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif


//-------------------------------


/** Konstruktor klasy SignIn (okna logowania):
 *
 * Tworzy nowe wystąpienie obiektu z klasy Ui.
 *
 * @param *parent Wskaźnik na obiekt z klasy wyższego rzędu, opisuje kto jest rodzicem obiektu z klasy SignIn
 *
 */
SignIn::SignIn(QWidget *parent) : QWidget(parent), ui(new Ui::SignIn) {
    #ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup( MAKEWORD( 2, 2 ), & wsaData );
        if( result != NO_ERROR )
             printf( "Initialization error.\n" );
    #endif
    ui->setupUi(this);
    ui->connectionLabel->setVisible(false);
}



/** Destruktor klasy SignIn (okna logowania):
 *
 * Usuwa obiekt z klasy Ui.
 *
 */

SignIn::~SignIn() {

    delete ui;
}



/** Funkcja sprawdzająca czy podany adres IP jest prawidłowo zapisany:
 *
 * @param ip Adres IP, przekazany jako QString
 * @return Prawda - gdy adres poprawny | Fałsz - gdy adres niepoprawny
 *
 */
bool checkIPFormat(QString ip) {

    // Sprawdzenie czy występują 3 kropki w adresie
    if (ip.count('.') == 3) {

        // Sprawdzenie czy rozdzielone kropkami wartości to liczby z przedziału <0;255>
        for (int i = 0; i < 3; i++) {

            bool ok;
            int number = ip.section(".", i,i).toInt(&ok, 10);

            if (ok == false) return false;
            if (!(number >= 0 && number <= 255)) return false;
        }
    } else
        return false;

    return true;
}



/** Funkcja sprawdzająca, czy podany numer portu jest prawidłowy:
 *
 * @param port Numer portu zapisany
 * @return Prawda - gdy numer portu jest prawidłowy | Fałsz - gdy numer portu jest nieprawidłowy
 *
 */
bool checkPortFormat(QString port) {

    bool ok;
    int number = port.toInt(&ok, 10);

    // Czy przekazany tekst jest liczbą
    if (ok == false) return false;

    // Czy jest liczbą z przedziału <1;65535>
    if (!(number >= 1 && number <= 65535)) return false;

    return true;
}



/** Funkcja sprawdzająca czy podany numer użytkownika jest poprawny:
 *
 * @param username Nazwa użytkownika
 * @return Prawda - gdy nazwa użytkownika jest prawidłowa | Fałsz - gdy nazwa użytkownika jest nieprawidłowa
 *
 */
bool checkUsernameFormat(QString username) {

    // Czy nazwa użytkownika zawiera średniki
    if (username.count(";") != 0) return false;

    // Czy nazwa użytkownika jest dłuższa niż 3 znaki i krótsza niż 15
    if (username.length() > 15 || username.length() < 3) return false;

    // Czy nazwa użytkownika zaczyna się od spacji
    if (username.at(0) == ' ') return false;

    // Czy w nazwie użytkownika występują 2 lub więcej spacje z rzędu
    for (int i = 0; i < username.length() - 1; i++) {
        if (username.at(i) == ' ' && username.at(i+1) == ' ') return false;
    }

    return true;
}



/** Funkcja próbująca nawiązać połączenie z serwerem:
 *
 * @return Prawda - jeżeli udało się połączyć z serwerem | Fałsz - gdy nie udało się nawiązać połączenia
 *
 */
bool SignIn::tryConnect() {

    // Ustawienie adresu IP serwera
    char *server = ui->ipAddressEdit->text().toLatin1().data();

    // Ustawienie numer portu
    short service_port = ui->portEdit->text().toShort(NULL, 10);

    struct sockaddr_in sckAddr;
    int sck;

    // Wyczyszczenie struktury adresowej
    memset(&sckAddr, 0, sizeof sckAddr);

    #ifdef _WIN32
        sckAddr.sin_addr.s_addr = inet_addr(server);
    #else
        inet_aton(server, &sckAddr.sin_addr);
    #endif
    sckAddr.sin_family = AF_INET;
    sckAddr.sin_port = htons(service_port);

    // Próba tworzenia gniazda
    if ((sck = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror ("Nie można utworzyć gniazdka");
        return false;
    }

    // Próba nawiązania połączenia
    if (::connect (sck, (struct sockaddr*) &sckAddr, sizeof sckAddr) < 0) {
        perror ("Brak połączenia");
        return false;
    }

    // Pierwsza wiadomość do serwera z nazwą użytkownika
    QString firstMessage = "0;" + ui->usernameEdit->text();

    // Próba wysłania pierwszej wiadomości do serwera
    if(send(sck, firstMessage.toUtf8().constData() , firstMessage.toUtf8().length(), 0) < 0) {
        perror ("Błąd przesłania wiadomości");
        return false;
    }

   // Ukrycie okna łączenia do serwera
    this->hide();

    // Utworzenie głównego okna czatu
    MainWindow *w = new MainWindow();

    // Przesłanie informacj o sockecie do obiektu z klasy MainWindow
    w->setSocket(sck);

    // Wyświetlenie głównego okna czatu
    w->show();

    return true;
}



/** Funkcja próbująca uzyskać dostęp do głównego okna czatu:
 *
 * Sprawdzane są przesyłane dane i w przypadku ich prawidłowości próbowane jest
 * nawiązanie połączenia z serwerem. W przypadku niepowodzenia obsługa błędów.
 *
 */
void SignIn::enterChat() {

    bool noError = true;
    ui->connectionLabel->setVisible(false);

    // Sprawdzenie prawidłości podanego adresu IP
    if (!checkIPFormat(ui->ipAddressEdit->text())) {
        ui->ipAddressEdit->setStyleSheet("background-color: rgb(255, 255, 255);border: 1px solid #FF0000");
        noError = false;
    } else {
        ui->ipAddressEdit->setStyleSheet("background-color: rgb(255, 255, 255);border: 1px solid #AAAAAA");
    }

    // Sprawdzenie prawidłości podanego numeru portu
    if (!checkPortFormat(ui->portEdit->text())) {
        ui->portEdit->setStyleSheet("background-color: rgb(255, 255, 255);border: 1px solid #FF0000");
        noError = false;
    } else {
        ui->portEdit->setStyleSheet("background-color: rgb(255, 255, 255);border: 1px solid #AAAAAA");
    }

    // Sprawdzenie prawidłości podanej nazwy użytkownika
    if (!checkUsernameFormat(ui->usernameEdit->text())) {
        ui->usernameEdit->setStyleSheet("background-color: rgb(255, 255, 255);border: 1px solid #FF0000");
        noError = false;
    } else {
        ui->usernameEdit->setStyleSheet("background-color: rgb(255, 255, 255);border: 1px solid #AAAAAA");
    }

    // Jeżeli nie było żadnych błędów próba nawiązania połączenia
    if (noError) {

        // Zablokowanie możliwości ponownego połączenia dopóki nie otrzymamy informacji o niepowodzeniu
        ui->ipAddressEdit->setEnabled(false);
        ui->portEdit->setEnabled(false);
        ui->usernameEdit->setEnabled(false);
        ui->startChatButton->setEnabled(false);

        // Czy udało się połączyć
        if (!tryConnect()) {

            // Odblokowanie możliwości ponownego łączenia z serwerem
            ui->ipAddressEdit->setEnabled(true);
            ui->portEdit->setEnabled(true);
            ui->usernameEdit->setEnabled(true);
            ui->startChatButton->setEnabled(true);
            ui->connectionLabel->setVisible(true);
        }
    }
}


//----------- OBSŁUGA SYGNAŁÓW -------------


/** Przesłanie prośby o nawiązanie połączenia:
 *
 * W przypadku naciśnięcia klawisza ENTER
 *
 */
void SignIn::on_ipAddressEdit_returnPressed() {

    // Próba nawiązania połączenia
    enterChat();
}



/** Przesłanie prośby o nawiązanie połączenia:
 *
 * W przypadku naciśnięcia klawisza ENTER
 *
 */
void SignIn::on_portEdit_returnPressed() {

    // Próba nawiązania połączenia
    enterChat();
}



/** Przesłanie prośby o nawiązanie połączenia:
 *
 * W przypadku naciśnięcia klawisza ENTER
 *
 */
void SignIn::on_usernameEdit_returnPressed() {

    // Próba nawiązania połączenia
    enterChat();
}




/** Przesłanie prośby o nawiązanie połączenia:
 *
 * W przypadku naciśnięcia klawisza ENTER
 *
 */
void SignIn::on_startChatButton_clicked() {

    // Próba nawiązania połączenia
    enterChat();
}
