#include<QApplication>
#include"./controller/ApplicationController.hpp"
#include<QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}
