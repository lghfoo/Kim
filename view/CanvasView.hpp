#pragma once
#include<QGraphicsView>
#include<QGraphicsScene>
#include<QKeyEvent>
namespace Kim {
    class KScene : public QGraphicsScene{

    };

    class KCanvasView : public QGraphicsView{
        Q_OBJECT
    signals:
        void KeyReleaseSignal(QKeyEvent* event);
    public:
        KCanvasView(){
            this->setRenderHint(QPainter::Antialiasing);
        }
        virtual void keyReleaseEvent(QKeyEvent *event) override{
            emit KeyReleaseSignal(event);
        }
    };


}
