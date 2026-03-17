#include "mainwindow.h"
#include <QApplication>
#include "Authentication/authentication.h"
#include "Registration/registration.h"
#include "ConfigManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // load in config file
    if (!ConfigManager::instance().load("socket_config.ini")) {
        qCritical() << " >> Failed to load socket_config.ini";
        return 1;
    }

    Authentication auth;
    Registration reg;

    QString login;
    QString password;

    QObject::connect(&auth, &Authentication::loginSuccessful,
                     [&](const QString &l, const QString &p) {
                         login = l;
                         password = p;
                     });

    QObject::connect(&reg, &Registration::loginSuccessful,
                     [&](const QString &l, const QString &p) {
                         login = l;
                         password = p;
                     });

    const QString lanIp = ConfigManager::instance().IpServer();
    const int port = ConfigManager::instance().serverPort();

    auth.connectToServer(lanIp, port);
    reg.connectToServer(lanIp, port);

    // execute after successful authentication
    if (auth.exec() == QDialog::Accepted) {
        MainWindow w(login, password);
        w.show();
        return a.exec();
    } else if (reg.exec() == QDialog::Accepted) {
        MainWindow w(login, password);
        w.show();
        return a.exec();
    }

    // If authentication failed or was cancelled, exit
    return 0;
}
