#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <list>
#include <time.h>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

//-----------------------

struct client {
        int sck;
        string name;
        string color;
        string chatRoom;
};

//-----------------------

#define CON_LIMIT 128
#define PORT 2000
#define BUFSIZE 1024;

pthread_t client_threads [CON_LIMIT];
pthread_t main_thread;
pthread_mutex_t sock_mutex = PTHREAD_MUTEX_INITIALIZER;
int cln_sockets [CON_LIMIT];
list<client> clients;

string colorsList[] = { "DeepPink", "DodgerBlue", "DarkTurquoise", "ForestGreen",
                        "Gold", "Indigo", "LightSalmon", "Crimson", "DarkOrange",
                        "DarkMagenta", "DarkViolet", "GoldenRod", "LightSeaGreen",
                        "MediumBlue", "Navy", "OrangeRed", "Orchid", "SeaGreen",
                        "Sienna", "SaddleBrown", "SteelBLue", "Teal", "Tomato" };

string chatRooms[] = { "Main Chat", "Book Lovers", "Music Fun", "Game Talk",
                       "Random Stuff", "Home Alone", "Chatters", "League of Legends" };

//-----------------------

/** Funkcja zamieniajca integer na string
 *
 * @param integer Wartość do zamiany na string
 * @return Zamieniony int na string
 *
 */
string intToStr(int integer) {

        stringstream intSS;
        intSS << integer;
        return intSS.str();
}



/** Funkcja zwracająca losowy kolor z tablicy kolorów
 *
 * @return Losowy kolor
 *
 */
string randomColor() {

        // Rozmiar tablicy kolorów
        int numberOfColors = sizeof(colorsList) / sizeof(colorsList[0]);

        // Losowy indeks tablicy colorsList
        int randomNumber = rand() % numberOfColors;

        return colorsList[randomNumber];
}



/** Funkcja odpowiedzialna za wysyłanie wiadomości do klienta
 *
 * @param sck Gniazdo do komunikacji z odpowiednim klientem
 * @param messageType Rodzaj przesyłanej wiadomości
 * @param message Przesyłana wiadomość
 *
 */
void sendMessageToClient(int sck, string messageType, string message) {

    // Przygotowanie wartości końcowej złożonej z typu wiadomości; rozmiaru wiadomości; wiadomości
    string finalMessage = messageType + ";" + intToStr(message.length()) + message;

    // Przesłanie wiadomości do klienta
    send (sck, finalMessage.c_str(), finalMessage.length(), 0);
}



/** Funkcja odpowiadająca za odświeżenie listy użytkowników w danym pokoju czatu
 *
 * Przesyła informację do każdego klienta z nową listą użytkowników znajdujących się w jego pokoju czatu
 *
 */
void refreshRoomUsersList() {

    // Przejrzenie całej tablicy użytkowników czatu
    for (list<client>::iterator i = clients.begin(); i != clients.end(); ++i) {

        string messageType = "2;";
        string usersList = "";

        for (list<client>::iterator it = clients.begin(); it != clients.end(); ++it)
            if (i->chatRoom == it->chatRoom) {
                usersList += ";" + it->name;
            }

        // Przesłanie do odpowiedniego klienta zaktualizowanej listy użytkowników
        sendMessageToClient(i->sck, "2", usersList);
    }
}



/** Funkcja zmiany pokoju czatu dla wybranego klienta
 *
 * @param sck Gniazdo klienta, który zmienia pokój czatu
 * @param chatName Nazwa nowego pokoju czatu
 *
 */
void changeChatRoom(int sck, string chatName) {

    for (list<client>::iterator it = clients.begin(); it != clients.end(); ++it) {

        if (it->sck == sck) {
            it->chatRoom = chatName;
            string welcomeMessage = ";<font color=\"DimGray \">Hi <font color=\"MidnightBlue \">" + it->name +
                                    "</font>! <br>Welcome to <font color=\"MidnightBlue \">" + chatName +"</font>!<br>" +
                                    "Respect other chat users and have fun!</font><br>";

            sendMessageToClient(sck, "1", welcomeMessage);

            // Odświeżenie listy użytkowników w każdym z pokoi
            refreshRoomUsersList();
            break;
        }
    }
}



/*** Funkcja sprawdzajaca dane potrzebne przy logowaniu
 * 
 * Sprawdza czy przeslane przez uzytkownika dane do logowania
 * sa prawidlowe czy nie.
 * 
 * @param username Nazwa uzytkownika
 * @param password Haslo do konta
 * 
 * @return Prawda gdy istnieje taki uzytkownik w bazie, Falsz w przeciwnym wypadku
 * 
 */
bool goodData(string username, string password) {

	fstream plik;
	string passwordFile;
	plik.open("user_data.txt");

	if (plik.good(), ios::out) {

		while (!plik.eof()) {
		    getline(plik, passwordFile);
		    string delimiter = ";";
    		    string usernameFile;
    		    size_t pos = 0;

    
    		    pos = passwordFile.find(delimiter);
        	    usernameFile = passwordFile.substr(0, pos);
        	    passwordFile.erase(0, pos + delimiter.length());


		    cout << endl << username << ":" << password << "   " << usernameFile << ":" << passwordFile << endl;
		    if (usernameFile == username && passwordFile == password) {
		        for (list<client>::iterator it = clients.begin(); it != clients.end(); ++it) 
			    if (it->name == username) return false;
			
			return true;
		    }
		}

		plik.close();
	}		

	return false;
}



/*** Funkcja odpowiadajaca za procedure sprawdzenie poprawnosci hasla
 * 
 * Bada czy uzytkownik ktory chce sie przylaczyc do czatu podal
 * prawidlowe dane logowania. Jezeli tak to jest przesylana do niego
 * wiadomosc o poprawnym zalogowaniu. W przeciwnym wypadku jego gniazdo 
 * jest odlaczane od serwera, a uzytkownik dostaje wiadomosc o niepowodzeniu
 * i nastepuje zamkniecie polaczenia z serwerem.
 * 
 * @param sck Gniazdo do komunikacji
 * @param password Haslo do konta
 */
void checkPassword(int sck, string password) {
	
    string delimiter = ";";
    string username;
    size_t pos = 0;

    // Sprawdzenie jaki typ wiadomości został wysłany
    	pos = password.find(delimiter);
        username = password.substr(0, pos);
        password.erase(0, pos + delimiter.length());
    
	if (goodData(username, password)) {
	    send (sck, "1", 1, 0);
	} else {
	    send (sck, "0", 1, 0);
  	 
   	    close (sck);
    	    pthread_mutex_lock (&sock_mutex);

		    // Zwolnienie gniazda
	    for (int i = 0; i < CON_LIMIT; i++)
		if (cln_sockets [i] == sck) {
		    cln_sockets [i] = 0;
		    break;
		}
    		pthread_mutex_unlock (&sock_mutex);
	}
}



/** Funkcja przygotowująca nowego klienta do korzystania z czatu
 *
 * @param sck Gniazdo nowego klienta
 * @param name Nazwa nowego klienta
 *
 */
void prepareClient(int sck, string name) {

    // Dodanie klienta do listy klientów
    client newClient;
    newClient.sck = sck;
    newClient.name = name;
    newClient.color = randomColor();
    clients.push_back(newClient);

    // Standardowo klient dodawany jest do pokoju głównego czatu
    changeChatRoom(sck, "Main Chat");

    string chatRoomsList = "";
    for (int i = 0; i < (int)(sizeof(chatRooms) / sizeof(chatRooms[0])); i++) {
            chatRoomsList += ";" + chatRooms[i];
    }

    // Przesłanie listy dostępnych czatów
    sendMessageToClient(sck, "3", chatRoomsList);
}



/** Funkcja wywoływana gdy klient opuścił czat
 *
 * @param sck Gniazdo klienta
 *
 */
void clientLeft(int sck) {

    // Znalezienie i usunięcie wybranego klienta z listy klientów
    for (list<client>::iterator i = clients.begin(); i != clients.end();) {
        if (i->sck == sck)
            i = clients.erase(i);
         else
            ++i;
    }

    // Odświeżenie listy użytkowników
    refreshRoomUsersList();

    // Zamknięcie gniazda
    close (sck);

    pthread_mutex_lock (&sock_mutex);

    // Zwolnienie gniazda
    for (int i = 0; i < CON_LIMIT; i++)
        if (cln_sockets [i] == sck) {
            cln_sockets [i] = 0;
            break;
        }

    pthread_mutex_unlock (&sock_mutex);

    printf ("Client left socket %d is now open\n", sck);
    pthread_exit (NULL);
}



/** Funkcja przygotowująca wiadomość czatu do wysyłki
 *
 * @param sck Gniazdo klienta, który chce wysłać wiadomość
 * @param clientName Nazwa klienta
 * @param color Kolor klienta
 * @param message Treść wiadomości
 *
 */
void prepareChatMessage(int sck, string clientName, string color, string message) {

    bool showTime = true;
    string finalMessage;

    // Czy chcemy wyświetlać godzinę na czacie?
    if (showTime) {
        time_t rawtime;
        time (&rawtime);
        struct tm *time_info = localtime(&rawtime);
        string minutes, hour;

        // Sprawdzenie czy otrzymana wartość minuty nie jest z przedziału <1;6> jeżeli jest to dopisanie "0" przed minutą
        if (time_info->tm_min / 10 == 0) minutes = "0" + intToStr(time_info->tm_min);
        else minutes = intToStr(time_info->tm_min);

         // Sprawdzenie czy otrzymana wartość godziny nie jest z przedziału <1;6> jeżeli jest to dopisanie "0" przed godziną
        if (time_info->tm_hour / 10 == 0) hour = "0" + intToStr(time_info->tm_hour);
        else hour = intToStr(time_info->tm_hour);


        string time = hour + ":" + minutes;
        finalMessage = ";<html><p><font color=\"Silver\">" + time + "</font> <b><font color=\"" + color + "\">" + clientName + "</b></font>: " + message + "</p></html>";
    } else
        finalMessage = ";<html><p><b><font color=\"" + color + "\">" + clientName + "</b></font>: " + message + "</html>";

    // Przesłanie wiadomości do klienta
    sendMessageToClient(sck, "1", finalMessage);
}



/** Funkcja przygotowujaca prywatna wiadomosc do wysylki
 * 
 * @param sck Gniazdo do komunikacji
 * @param username Nazwa uzytkownka
 * @param clientName Nazwa uzytkownka z ktorym sie komunikujemy
 * @param color Kolor uzytkownika
 * @param message Wiadomosc ktora chcemy przeslac
 * 
 */
void preparePrivateChatMessage(int sck, string username, string clientName, string color, string message) {

    bool showTime = true;
    string finalMessage;

    // Czy chcemy wyświetlać godzinę na czacie?
    if (showTime) {
        time_t rawtime;
        time (&rawtime);
        struct tm *time_info = localtime(&rawtime);
        string minutes, hour;

        // Sprawdzenie czy otrzymana wartość minuty nie jest z przedziału <1;6> jeżeli jest to dopisanie "0" przed minutą
        if (time_info->tm_min / 10 == 0) minutes = "0" + intToStr(time_info->tm_min);
        else minutes = intToStr(time_info->tm_min);

         // Sprawdzenie czy otrzymana wartość godziny nie jest z przedziału <1;6> jeżeli jest to dopisanie "0" przed godziną
        if (time_info->tm_hour / 10 == 0) hour = "0" + intToStr(time_info->tm_hour);
        else hour = intToStr(time_info->tm_hour);


        string time = hour + ":" + minutes;
        finalMessage = ";" + username + ";<html><p><font color=\"Silver\">" + time + "</font> <b><font color=\"" + color + "\">" + clientName + "</b></font>: " + message + "</p></html>";
    } else
        finalMessage = ";" + username + ";<html><p><b><font color=\"" + color + "\">" + clientName + "</b></font>: " + message + "</html>";

    // Przesłanie wiadomości do klienta
    sendMessageToClient(sck, "4", finalMessage);
}



/** Funkcja wywoływana gdy otrzymano przyszła nowa wiadomość na czacie
 *
 * @param sck Gniazdo klienta, który przesłał wiadomość czatu
 * @param message Wiadomość którą przesyła klient
 *
 */
void chatMessageReceived(int sck, string message) {

    // Znalezienie klienta, który przesyła wiadomość
    for (list <client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (sck == it->sck) {
            string clientChatRoom = it->chatRoom;
            string clientName = it->name;
            string clientColor = it->color;

            // Przesłanie do wszystkich innych klientów w tym pokoju tej wiadomości
            for (list <client>::iterator rest = clients.begin(); rest != clients.end(); ++rest) {
                if (rest->chatRoom == clientChatRoom) 
                    prepareChatMessage(rest->sck, clientName, clientColor, message);
                
            }

            break;
        }
    }
}



/** Funkcja obslugujaca przychodzaca wiadomosc od innego uzytkownika do serwera
 * 
 * @param sck Gniazdo do komunikacji
 * @param message Tresc wiadomosci
 * 
 */
void privateChatMessageReceived(int sck, string message) {

    string delimiter = ";";
    string username;
    size_t pos = 0;

    // Sprawdzenie jaki typ wiadomości został wysłany
    	pos = message.find(delimiter);
        username = message.substr(0, pos);
        message.erase(0, pos + delimiter.length());
    

    // Znalezienie klienta, który przesyła wiadomość
    for (list <client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (sck == it->sck) {
            string clientChatRoom = it->chatRoom;
            string clientName = it->name;
            string clientColor = it->color;

            // Przesłanie do wszystkich innych klientów w tym pokoju tej wiadomości
            for (list <client>::iterator rest = clients.begin(); rest != clients.end(); ++rest) {
                if (rest->name == username) {
		std::cout << rest->name << " " << username << std::endl;
                    preparePrivateChatMessage(rest->sck, clientName, clientName, clientColor, message);
		    preparePrivateChatMessage(sck, rest->name, clientName, clientColor, message);
			break;
		}   
            }

            break;
        }
    }
}



/** Funkcja obsługująca otrzymaną wiadomość
 *
 * @param sck Gniazdo klienta, który przesłał wiadomość do serwera
 * @param buffer Bufor z przesłanymi danymi
 *
 */
void handleReceivedMessage(int sck, char *buffer) {

    string message(buffer);
    string delimiter = ";";
    string token;
    size_t pos = 0;

    // Sprawdzenie jaki typ wiadomości został wysłany
    pos = message.find(delimiter);
    token = message.substr(0, pos);
    message.erase(0, pos + delimiter.length());
    

	std::cout << message << std::endl;

	if (token == "0")
	    prepareClient(sck, message);
	else if (token == "1")
	    chatMessageReceived(sck, message);
	else if (token == "2")
	    privateChatMessageReceived(sck, message);
	else if (token == "3")
	    changeChatRoom(sck, message);
	else if (token == "9")
	    checkPassword(sck, message);
}



/** Główna pętla klienta na serwerze, w której nasłuchuje on wiadomości od siebie (klienta)
 *
 * @param arg Przesyłane gniazdo klienta, który podłączył się do czatu
 * @return Wskaźnik na metodę
 *
 */
void* client_loop (void* arg) {

    char buffer [1024];
    int sck = *((int*) arg);
    int rcvd;

    printf ("New client has connected. Socket %d is now taken!\n", sck);

    // Oczekiwanie na wiadomości od klienta przychodzące pod dany socket
    while( (rcvd = recv(sck , buffer, 1024 , 0)) > 0 ) {

        // Przekazanie otrzymanej wiadomości do funkcji, która sprawdzi jakiego typu to wiadomość
        handleReceivedMessage(sck, buffer);
        memset (&buffer, 0, 1024);
    }

    // Jeżeli klient opuści serwer wywoływana jest funkcja czyszcząca dane związane z klientem
    clientLeft(sck);

    return NULL;
}



/** Główna pętla serwera, w której nasłuchuje on czy nie próbują podłączyć się nowi klienci
 * @param arg
 * @return Wskaźnik na metodę
 *
 */
void* main_loop (void* arg) {

    // Deklaracja niezbędnych zmiennych i struktur
    int srv_socket, tmp_socket;
    socklen_t cln_addr_size;
    struct sockaddr_in srv_addr, cln_addr;

    // Szczegółowe dane dotyczące typu połączenia
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons (PORT);

    // Wyczyszczenie tablicy gniazd klientów
    bzero (cln_sockets, CON_LIMIT * sizeof (int));

    // Próba uzyskania gniazda serwera
    if ((srv_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror ("Main socket creation error:");
        exit (EXIT_FAILURE);
    }

    // Próba przypisania gniazda do struktury adresowej
    if (bind (srv_socket, (struct sockaddr*) &srv_addr, sizeof srv_addr) == -1) {
        perror ("Main socket bind error:");
        exit (EXIT_FAILURE);
    }

    // Próba nasłuchiwania
    if (listen (srv_socket, 16) == -1) {
        perror ("Main socket listen error:");
        exit (EXIT_FAILURE);
    }

    printf ("Server is ready to go!\n");

    // Nasłuchiwanie czy nowi klienci nie chcą się połączyć z serwerem
    while (1) {
        cln_addr_size = sizeof cln_addr;

        // Jeżeli przyjdzie zapytanie o akceptacje (protokół TCP) to wiemy, że nowy klient próbuje nawiązać z serwerem połączenie
        if ((tmp_socket = accept (srv_socket, (struct sockaddr*) &cln_addr, &cln_addr_size)) == -1) {
            perror ("Main socket accept error:");
            continue;
        }

        // Podnosimy semafor (aby wyeliminować niezgodności związane z wielowątkową pracą serwera)
        pthread_mutex_lock (&sock_mutex);

        int i;

        // Dopisanie nowego klienta do listy gniazd (o ile jest miejsce dla klienta)
        for (i = 0; i < CON_LIMIT; i++)
            if (cln_sockets [i] == 0) {
                cln_sockets [i] = tmp_socket;
                break;
            }

        // Opuszczenie semafora
        pthread_mutex_unlock (&sock_mutex);

        // Sprawdzenie czy nie osiągneliśmy maksymalnej liczby podłączonych klientóW
        if (i == CON_LIMIT) {
            printf ("Too many connections\n");
            close (tmp_socket);
            continue;
        }

        // Utworzenie nowego wątku dla obsługi klienta, aby mógł nasłuchiwać przychodzących wiadomości od klienta
        if (pthread_create (&client_threads [i], NULL, client_loop, &cln_sockets[i]) != 0) {
            printf ("Error creating new client thread\n");
            continue;
        }
    }
}



/** Funkcja startowa programu
 *
 * @param argc Liczba argumentów
 * @param argv Lista argumentów
 * @return Wartość wykonania programu
 *
 */
int main (int argc, char** argv) {

        srand(time(NULL));

        // Utworzenie nowego głównego wątku nasłuchującego czy nowi klienci chcą się przyłączyć do serwera
        if (pthread_create (&main_thread, NULL, main_loop, NULL) != 0) {
        printf ("Thread create error\n");
        exit (EXIT_FAILURE);
        }

        printf ("Server has started\n");
        printf ("Press <ENTER> to shutdown server\n");
        getc (stdin);

        return EXIT_SUCCESS;
}
