#include<QApplication>
#include"./controller/ApplicationController.hpp"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}
