#include "mainwindow.h"
#include <QApplication>
#include "Authentication/authentication.h"
#include "Registration/registration.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    // Show authentication first
    Authentication auth;
    Registration reg;

    auth.connectToServer("26.161.132.244", 12345);
    reg.connectToServer("26.161.132.244", 12345);

    // execute after successful authentication
    if (auth.exec() == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec();
    } else if(reg.exec() == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec();
    }
    
    // If authentication failed or was cancelled, exit
    return 0;
}
