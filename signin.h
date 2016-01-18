#ifndef SIGNIN_H
#define SIGNIN_H

#include <QWidget>

namespace Ui {class SignIn;}

class SignIn : public QWidget
{
    Q_OBJECT

public:
    explicit SignIn(QWidget *parent = 0);
    ~SignIn();

private slots:
    void enterChat();
    bool tryConnect();
    void on_ipAddressEdit_returnPressed();
    void on_portEdit_returnPressed();
    void on_usernameEdit_returnPressed();
    void on_startChatButton_clicked();
    void on_passwordEdit_returnPressed();

private:
    Ui::SignIn *ui;
};

#endif // SIGNIN_H
