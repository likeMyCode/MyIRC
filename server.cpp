#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <list>
#include <time.h>
#include <sstream>

using namespace std;

#define CON_LIMIT 128
#define PORT 2000
#define BUFSIZE 1024;

struct client {
	int sck;
	string name;
	string color;
	string chatRoom;
};

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
		       "Random Stuff", "YMCA" };


string intToStr(int integer) {
	stringstream intSS;
	intSS << integer;
	return intSS.str();
}

string randomColor() {
	int numberOfColors = sizeof(colorsList) / sizeof(colorsList[0]);
	int randomNumber = rand() % numberOfColors;

	return colorsList[randomNumber];
}


void sendMessageToClient(int sck, string messageType, string message) {
	
	string finalMessage = messageType + ";" + intToStr(message.length()) + message;
	cout << finalMessage.c_str() << endl;
	send (sck, finalMessage.c_str(), finalMessage.length(), 0);
}


void refreshChatList() {
	
	for (list<client>::iterator i = clients.begin(); i != clients.end(); ++i) {

		string messageType = "2;";
		string usersList = "";
		for (list<client>::iterator it = clients.begin(); it != clients.end(); ++it) 	
			if (i->chatRoom == it->chatRoom) {
				usersList += ";" + it->name;
			}

		sendMessageToClient(i->sck, "2", usersList);
	}
}

void changeChat(int sck, string chatName) {

	for (list<client>::iterator it = clients.begin(); it != clients.end(); ++it) {
	
		if (it->sck == sck) {
			it->chatRoom = chatName;
			string welcomeMessage = ";<font color=\"DimGray \">Hi <font color=\"MidnightBlue \">" + it->name + 
						"</font>! <br>Welcome to <font color=\"MidnightBlue \">" + chatName +"</font>!<br>" + 
						"Respect other chat users and have fun!</font><br>";
			
			sendMessageToClient(sck, "1", welcomeMessage);
			refreshChatList();
			break;
		}
	}
}

void prepareClient(int sck, string name) {
	
	client newClient;
	newClient.sck = sck;
	newClient.name = name;
	newClient.color = randomColor();
	clients.push_back(newClient);
	changeChat(sck, "Main Chat");

	string chatRoomsList = "";
	for (int i = 0; i < sizeof(chatRooms) / sizeof(chatRooms[0]); i++) {
		chatRoomsList += ";" + chatRooms[i];
	}

	sendMessageToClient(sck, "3", chatRoomsList);
}



void clientLeft(int sck) {
	for (list<client>::iterator i = clients.begin(); i != clients.end();) {
		if (i->sck == sck) 
			i = clients.erase(i);
		 else 
			++i;
	}
	refreshChatList();

    close (sck);
    pthread_mutex_lock (&sock_mutex);
    for (int i = 0; i < CON_LIMIT; i++)
        if (cln_sockets [i] == sck) {
            cln_sockets [i] = 0;
            break;
        }
    pthread_mutex_unlock (&sock_mutex);

    printf ("Client thread ending for socket %d\n", sck);
    pthread_exit (NULL);

}

void sendMessage(int sck, string clientName, string color, string message) {

	bool showTime = true;
	string finalMessage;

	if (showTime) {
		time_t rawtime;
		time (&rawtime);
		struct tm *time_info = localtime(&rawtime);
		string minutes, hour;

		if (time_info->tm_min / 10 == 0) minutes = "0" + intToStr(time_info->tm_min);
		else minutes = intToStr(time_info->tm_min); 

		if (time_info->tm_hour / 10 == 0) hour = "0" + intToStr(time_info->tm_hour);
		else hour = intToStr(time_info->tm_hour);
		string time = hour + ":" + minutes;

		finalMessage = ";<html><p><font color=\"Silver\">" + time + "</font> <b><font color=\"" + color + "\">" + clientName + "</b></font>: " + message + "</p></html>";
	} else {
		finalMessage = ";<html><p><b><font color=\"" + color + "\">" + clientName + "</b></font>: " + message + "</html>";
	}

	sendMessageToClient(sck, "1", finalMessage);
}

void chatMessage(int sck, string message) {
	for (list <client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (sck == it->sck) {
			string clientChatRoom = it->chatRoom;
			string clientName = it->name;
			string clientColor = it->color;

			for (list <client>::iterator rest = clients.begin(); rest != clients.end(); ++rest) {
				if (rest->chatRoom == clientChatRoom) {
					sendMessage(rest->sck, clientName, clientColor, message);
					cout << rest->sck << " " << rest->color << endl;
				}
			}
			break;
		}
	}
}

void handleReceivedMessage(int sck, char *buffer) {
	
	string message(buffer);
	string delimiter = ";";
	string token;
	size_t pos = 0;

	while ((pos = message.find(delimiter)) != string::npos) {
		token = message.substr(0, pos);
		message.erase(0, pos + delimiter.length());
		if (token == "0") {
			prepareClient(sck, message);
		} else if (token == "1") {
			chatMessage(sck, message);
		} else if (token == "2") {

		} else if (token == "3") {
			changeChat(sck, message);
		}
	}

	fflush(stdout);
}

void* client_loop (void* arg)
{
    char buffer [1024];
    int sck = *((int*) arg);
    int i;
    int rcvd;

    printf ("New client thread started for socket %d\n", sck);

  	while( (rcvd = recv(sck , buffer, 1024 , 0)) > 0 ){	
        	handleReceivedMessage(sck, buffer);
		cout << buffer << endl;
		memset (&buffer, 0, 1024);
   	}
	
	clientLeft(sck);
}

void* main_loop (void* arg) {
    int srv_socket, tmp_socket;
    socklen_t cln_addr_size;
    struct sockaddr_in srv_addr, cln_addr;

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons (PORT);

    bzero (cln_sockets, CON_LIMIT * sizeof (int));

    if ((srv_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror ("Main socket creation error:");
        exit (EXIT_FAILURE);
    }

    if (bind (srv_socket, (struct sockaddr*) &srv_addr, sizeof srv_addr) == -1) {
        perror ("Main socket bind error:");
        exit (EXIT_FAILURE);
    }

    if (listen (srv_socket, 16) == -1) {
        perror ("Main socket listen error:");
        exit (EXIT_FAILURE);
    }

    printf ("Main socket started\n");

    while (1) {
        cln_addr_size = sizeof cln_addr;
        if ((tmp_socket = accept (srv_socket, (struct sockaddr*) &cln_addr, &cln_addr_size)) == -1) {
            perror ("Main socket accept error:");
            continue;
        }

        pthread_mutex_lock (&sock_mutex);
        int i;
        for (i = 0; i < CON_LIMIT; i++)
            if (cln_sockets [i] == 0) {
                cln_sockets [i] = tmp_socket;
                break;
            }
        pthread_mutex_unlock (&sock_mutex);

        if (i == CON_LIMIT) {
            printf ("Too many connections\n");
            close (tmp_socket);
            continue;
        }

        if (pthread_create (&client_threads [i], NULL, client_loop, &cln_sockets[i]) != 0) {
            printf ("Error creating new client thread\n");
            continue;
        }
    }
}

int main (int argc, char** argv) {

	srand(time(NULL));
    if (pthread_create (&main_thread, NULL, main_loop, NULL) != 0) {
        printf ("Thread create error\n");
        exit (EXIT_FAILURE);
    }

    printf ("Server has started\n");
    printf ("Press <ENTER> to shutdown server\n");
    getc (stdin);

    return EXIT_SUCCESS;
}

