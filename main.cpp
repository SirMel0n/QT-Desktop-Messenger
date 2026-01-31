//#include "mainwindow.h"
//#include "Authentication/authentication.h"
#include "Registration/registration.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Registration w;
    w.show();
    return a.exec();
}
