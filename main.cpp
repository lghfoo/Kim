#include<QApplication>
#include"./controller/ApplicationController.hpp"
#include<QDebug>
class KApplication : public QApplication{
public:
    KApplication(int argc, char* argv[]):QApplication (argc, argv){

    }

    virtual bool notify(QObject *receiver, QEvent *e) override{
        switch (e->type()) {
        case QEvent::GraphicsSceneDrop:
            break;
        case QEvent::GraphicsSceneDragEnter:
            break;
        case QEvent::GraphicsSceneDragMove:
            break;
        case QEvent::GraphicsSceneDragLeave:
            break;
        default:
            break;
        }
        return QApplication::notify(receiver, e);
    }
};

int main(int argc, char *argv[])
{
    KApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}
