#include "authentication.h"
#include "ui_authentication.h"

Authentication::Authentication(QWidget *parent)
    : Registration(parent)
    , ui(new Ui::Authentication)
{
    ui->setupUi(this);
}

Authentication::~Authentication()
{
    delete ui;
}
