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

    QString login;
    QString password;
    const QString lanIp = ConfigManager::instance().IpServer();
    const int port = ConfigManager::instance().serverPort();

    bool accepted = false;

    {
        Authentication auth;
        QObject::connect(&auth, &Authentication::loginSuccessful,
                         [&](const QString &l, const QString &p) {
                             login = l;
                             password = p;
                         });
        auth.connectToServer(lanIp, port);

        if (auth.exec() == QDialog::Accepted) {
            accepted = true;
        }
    }

    if (!accepted) {
        Registration reg;
        QObject::connect(&reg, &Registration::loginSuccessful,
                         [&](const QString &l, const QString &p) {
                             login = l;
                             password = p;
                         });
        reg.connectToServer(lanIp, port);

        if (reg.exec() == QDialog::Accepted) {
            accepted = true;
        }
    }

    if (!accepted) {
        return 0;
    }

    MainWindow w(login, password);
    w.show();
    return a.exec();
}
