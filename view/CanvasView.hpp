#pragma once
#include<QGraphicsView>
#include<QGraphicsScene>
#include<QKeyEvent>
#include<QTimer>
#include<QDebug>
#include<QGraphicsItem>
#include<QGraphicsSceneDragDropEvent>
#include<QFileDialog>
namespace Kim {
    class KScene : public QGraphicsScene{
        Q_OBJECT
    public:
        struct KGrid{
            qreal CellW = 32;
            qreal CellH = 32;
        };
        class KCursor : public QGraphicsItem{
        public:
            QRectF boundingRect() const override
            {
                return QRectF(-outer_radius, -outer_radius, outer_radius * 2, outer_radius * 2);
            }
            void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
            {
                QPen pen = painter->pen();
                pen.setWidth(2);
                pen.setColor(Qt::black);
                painter->setPen(pen);
                auto cycle = QRectF(-radius, -radius, radius * 2, radius * 2);
                painter->drawEllipse(cycle);

                pen.setCapStyle(Qt::PenCapStyle::RoundCap);
                pen.setWidth(3);
                painter->setPen(pen);
                constexpr int len = 4;
                qreal x0s[len] = {inner_radius, 0, -inner_radius, 0};
                qreal y0s[len] = {0, inner_radius, 0, -inner_radius};
                qreal x1s[len] = {outer_radius, 0, -outer_radius, 0};
                qreal y1s[len] = {0, outer_radius, 0, -outer_radius};
                for(int i = 0; i < len; i++){
                    painter->drawLine(QLineF(x0s[i], y0s[i], x1s[i], y1s[i]));
                }

            }
        private:
            qreal radius = 12.0;
            qreal inner_radius = 7;
            qreal outer_radius = radius + 9;
        };
    signals:
        void DragMoveSignal(QGraphicsSceneDragDropEvent *mouseEvent);
        void MouseReleaseSignal(QGraphicsSceneMouseEvent *mouseEvent);
    private:
        KGrid Grid;
        KCursor* Cursor = new KCursor;
    public:
        KScene(){
            Cursor->setZValue(1);
            this->addItem(Cursor);
        }
        void SetCursorPos(const QPointF& CursorPos){
            Cursor->setPos(CursorPos);
        }
        QPointF GetCursorPos(){
            return Cursor->pos();
        }
        const KGrid& GetGrid(){
            return Grid;
        }
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

        virtual void keyPressEvent(QKeyEvent* event)override{
            qDebug()<<"Scene"<<event->text()<<event->key();
            QGraphicsScene::keyPressEvent(event);
        }

        void drawBackground(QPainter *painter, const QRectF &rect) override
        {
            const qreal realLeft = rect.left();
            const qreal realRight = rect.right();
            const qreal realTop = rect.top();
            const qreal realBottom = rect.bottom();

            // Draw grid.
            const qreal firstLeftGridLine = realLeft - (std::fmod(realLeft, Grid.CellW));
            const qreal firstTopGridLine = realTop - (std::fmod(realTop, Grid.CellH));

            QVarLengthArray<QLineF, 100> lines;

            for (qreal x = firstLeftGridLine; x <= realRight; x += Grid.CellW)
                lines.append(QLineF(x, realTop, x, realBottom));
            for (qreal y = firstTopGridLine; y <= realBottom; y += Grid.CellH)
                lines.append(QLineF(realLeft, y, realRight, y));

            //painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(QPen(QColor(220, 220, 220), 0.0));
            painter->drawLines(lines.data(), lines.size());

            // Draw axes.
            painter->setPen(QPen(Qt::lightGray, 0.0));
//            painter->setPen(QPen(Qt::green, 0.0));
            painter->drawLine(QLineF(0, realTop, 0, realBottom));
//            painter->setPen(QPen(Qt::red, 0.0));
            painter->drawLine(QLineF(realLeft, 0, realRight, 0));
        }
    };

    class KCanvasView : public QGraphicsView{
        Q_OBJECT
    signals:
        void KeyPressSignal(QKeyEvent* event);
    public:
        KCanvasView(){
            this->setRenderHint(QPainter::Antialiasing);
        }
        virtual void keyPressEvent(QKeyEvent *event) override{
            qDebug()<<"canvas view"<<event->text()<<event->key();
            QGraphicsView::keyPressEvent(event);
            if(event->isAccepted())return;
            emit KeyPressSignal(event);

        }
    };


}
