#include "mainwindow.h"
#include <QApplication>
#include "Authentication/authentication.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    // Show authentication first
    Authentication auth;

    // execute after successful authentication
    if (auth.exec() == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec();
    }
    
    // If authentication failed or was cancelled, exit
    return 0;
}
