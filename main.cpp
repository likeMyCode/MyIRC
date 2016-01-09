#include "mainwindow.h"
#include "signin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SignIn s;
    s.show();

    return a.exec();
}
