#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);

signals:
};

#endif // APPWINDOW_H
