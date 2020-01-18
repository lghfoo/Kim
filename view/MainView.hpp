#pragma once
#include<QMainWindow>
#include <QStatusBar>
#include"CanvasView.hpp"
namespace Kim {
    class KMainView : public QMainWindow{
    private:
        QStatusBar* StatusBar = new QStatusBar;

    public:
        KMainView(){
            this->setWindowTitle("Kim-Keep Everything in Mind");
            this->resize(800, 600);
            this->setStatusBar(StatusBar);
        }
    };
}
