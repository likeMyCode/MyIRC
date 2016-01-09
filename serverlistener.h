#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include "mainwindow.h"
#include <QtCore>


class ServerListener : public QThread {

public:
    ServerListener(int socket, MainWindow *w);

private :
    MainWindow *window;
    int serverSocket;
    void parseMessage(QString message);
    void run();
};

#endif // SERVERLISTENER_H
