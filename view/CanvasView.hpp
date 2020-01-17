#pragma once
#include<QGraphicsView>
#include<QGraphicsScene>
#include<QKeyEvent>
namespace Kim {
    class KScene : public QGraphicsScene{
        Q_OBJECT
    signals:
        void DragMoveSignal(QGraphicsSceneDragDropEvent* event);
    protected:
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event)override{
            emit DragMoveSignal(event);
            QGraphicsScene::dragMoveEvent(event);
        }

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
