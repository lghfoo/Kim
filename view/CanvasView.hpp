#pragma once
#include<QGraphicsView>
#include<QGraphicsScene>
#include<QKeyEvent>
#include<QTimer>
#include<QDebug>
#include<QGraphicsSceneDragDropEvent>
namespace Kim {
    class KScene : public QGraphicsScene{
        Q_OBJECT
    signals:
        void DragMoveSignal(QGraphicsSceneDragDropEvent *mouseEvent);
        void MouseReleaseSignal(QGraphicsSceneMouseEvent *mouseEvent);
    public:
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event)override{
            emit DragMoveSignal(event);
            QGraphicsScene::dragMoveEvent(event);
        }

        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)override{
            QGraphicsScene::mouseReleaseEvent(mouseEvent);
            if(!mouseEvent->isAccepted()){
                emit MouseReleaseSignal(mouseEvent);
            }
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
