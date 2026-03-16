#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#pragma once
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QFile>

class ConfigManager {

public:
    static ConfigManager& instance() {
        static ConfigManager inst;
        return inst;
    }

    bool load(const QString &filePath) {
        if (m_settings) {
            delete m_settings;  // Clean up old settings if reloading
        }
        m_settings = new QSettings(filePath, QSettings::IniFormat);

        qDebug() << ">>ConfigManager: Loading from:" << filePath;
        qDebug() << ">>ConfigManager: File exists:" << QFile::exists(filePath);
        qDebug() << ">>ConfigManager: Status:" << m_settings->status();
        qDebug() << ">>ConfigManager: All keys:" << m_settings->allKeys();
        qDebug() << ">>ConfigManager: DB Server:" << m_settings->value("Server/ipaddress").toString();
        qDebug() << ">>ConfigManager: DB Name:" << m_settings->value("Server/port").toString();

        return m_settings->status() == QSettings::NoError;
    }

    // Database getters
    QString IpServer()   const { QString val = m_settings ? m_settings->value("Server/ipaddress", "localhost").toString() : "localhost";
        qDebug() << ">>ConfigManager::IpServer() returning:" << val;
        return val;
    }


    // Server getters
    int serverPort() const { return m_settings ? m_settings->value("Server/port", 12345).toInt() : 12345; }

private:
    ConfigManager() : m_settings(nullptr) {}  // Initialize pointer to nullptr
    ~ConfigManager() {
        if (m_settings) {
            delete m_settings;
        }
    }

    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    QSettings *m_settings;
};

#endif // CONFIGMANAGER_H
