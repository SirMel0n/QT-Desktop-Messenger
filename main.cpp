//#include "mainwindow.h"
//#include "Authentication/authentication.h"
#include "Registration/registration.h"
#include <QApplication>
#include "Authentication/authentication.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Authentication w;
    Registration y;
    w.show();
    y.show();
    return a.exec();
}
