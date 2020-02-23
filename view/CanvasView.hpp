#pragma once
#include<QGraphicsView>
#include<QGraphicsScene>
#include<QKeyEvent>
#include<QTimer>
#include<QDebug>
#include<QGraphicsItem>
#include<QGraphicsSceneDragDropEvent>
#include<QFileDialog>
#include <QScrollBar>
#include "../common/Utility.hpp"
namespace Kim {
    class KScene : public QGraphicsScene{
        Q_OBJECT
    public:
        struct KGrid{
            qreal CellW = 128;
            qreal CellH = 128;
            int SubCellWCount = 2;
            int SubCellHCount = 2;
        };
        class KCursor : public QGraphicsItem{
        private:
            qreal radius = 12.0;
            qreal inner_radius = 7;
            qreal outer_radius = radius + 9;
        public:
            QRectF boundingRect() const override
            {
                qreal Padding = 6.0;
                qreal W = outer_radius * 2 + Padding;
                qreal X = -W / 2.0;
                return QRectF(X, X, W, W);
            }
            QPainterPath Shape() const{
                QPainterPath Path;
                auto cycle = QRectF(-radius, -radius, radius * 2, radius * 2);
                Path.addEllipse(cycle);
                constexpr int len = 4;
                qreal x0s[len] = {inner_radius, 0, -inner_radius, 0};
                qreal y0s[len] = {0, inner_radius, 0, -inner_radius};
                qreal x1s[len] = {outer_radius, 0, -outer_radius, 0};
                qreal y1s[len] = {0, outer_radius, 0, -outer_radius};
                for(int i = 0; i < len; i++){
                    Path.moveTo(x0s[i], y0s[i]);
                    Path.lineTo(x1s[i], y1s[i]);
                }
                return Path;
            }
            virtual QPainterPath shape()const override{
                QPainterPathStroker Stroker;
                Stroker.setWidth(3);
                return Stroker.createStroke(Shape());
            }
            void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
            {
                QPen Pen;
                Pen.setColor(Qt::black);
                Pen.setWidth(3);
                Pen.setCapStyle(Qt::RoundCap);
                painter->setPen(Pen);
                painter->drawPath(Shape());
            }
        };
    signals:
        void DragMoveSignal(QGraphicsSceneDragDropEvent *DragDropEvent);
        void DropSignal(QGraphicsSceneDragDropEvent* DragDropEvent);
    private:
        KGrid Grid;
        KCursor* Cursor = new KCursor;
        qreal SceneScale = 5.0;
        QPointF SceneOffset = QPointF(0, 0);
        bool ShowingCursor = true;
    public:
        KScene(){
            Cursor->setZValue(ZOrder::CursorZ);
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

        void ShowCursor(bool Show){
            if(ShowingCursor == Show)return;
            ShowingCursor = Show;
            if(Show){
                this->addItem(Cursor);
            }
            else{
                this->removeItem(Cursor);
            }
        }

        virtual bool event(QEvent *event)override{
            switch (event->type()) {
            case QEvent::GraphicsSceneDragMove:{
                QGraphicsSceneDragDropEvent* E = static_cast<QGraphicsSceneDragDropEvent*>(event);
                E->setAccepted(false);
                QGraphicsScene::event(E);
                if(!E->isAccepted()){
                    E->acceptProposedAction();
                    E->setAccepted(true);
                }
                emit DragMoveSignal(E);
                return true;
            }
            case QEvent::GraphicsSceneDrop:{
                QGraphicsSceneDragDropEvent* E = static_cast<QGraphicsSceneDragDropEvent*>(event);
                E->setAccepted(false);
                QGraphicsScene::event(E);
                if(!E->isAccepted()){
                    E->acceptProposedAction();
                    E->setAccepted(true);
                    emit DropSignal(E);
                }
                return true;
            }
            default:
                break;
            }


            if(event->type() == QEvent::KeyPress){
                QKeyEvent *k = static_cast<QKeyEvent *>(event);
                if(k->key() == Qt::Key_Tab){
                    this->keyPressEvent(k);
                    return true;
                }
            }
            return QGraphicsScene::event(event);
        }

        virtual void mousePressEvent(QGraphicsSceneMouseEvent* Event) override{
            if(!itemAt(Event->scenePos(), QTransform())){
                Cursor->setPos(Event->scenePos());
//                Cursor->update();
            }
            QGraphicsScene::mousePressEvent(Event);
        }

        virtual void keyPressEvent(QKeyEvent* event)override{
            QGraphicsScene::keyPressEvent(event);
        }

        virtual void keyReleaseEvent(QKeyEvent* event)override{
            QGraphicsScene::keyReleaseEvent(event);
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

            QPen Pen;
            Pen.setColor(QColor(180, 180, 180));
            Pen.setWidth(1);
            painter->setPen(Pen);
            painter->drawLines(lines.data(), lines.size());

            // Draw axes.
            Pen.setColor(QColor(120, 120, 120));
            Pen.setWidth(2);
            painter->setPen(Pen);
            painter->drawLine(QLineF(0, realTop, 0, realBottom));
            painter->drawLine(QLineF(realLeft, 0, realRight, 0));
        }
    };

    class KCanvasView : public QGraphicsView{
        Q_OBJECT
    private:
        KScene* Scene = new KScene;
    signals:
        void KeyPressSignal(QKeyEvent* event);
    protected:
//        virtual void showEvent(QShowEvent *event) override{
//            qDebug()<<"show event";
//            QGraphicsView::showEvent(event);
//            this->ScrollToCenter();
//        }

        virtual bool event(QEvent *event)override{
            if(event->type() == QEvent::KeyPress){
                QKeyEvent *k = static_cast<QKeyEvent *>(event);
                if(k->key() == Qt::Key_Tab){
                    this->keyPressEvent(k);
                    return true;
                }
            }
            return QGraphicsView::event(event);
        }

        void wheelEvent(QWheelEvent *event) override
        {
            const qreal Scale = 1.05;
            if(event->delta() > 0)
                scale(Scale, Scale);
            else
                scale(1 / Scale, 1 / Scale);
        }

        virtual void keyReleaseEvent(QKeyEvent* event)override{
            QGraphicsView::keyReleaseEvent(event);
        }

        virtual void keyPressEvent(QKeyEvent *event) override{
            if(event->key() == Qt::Key_F2)
                ToggleDragMode();
            else if(event->key() == Qt::Key_C && event->modifiers() & Qt::ShiftModifier){
                this->ScrollToCenter();
            }
            QGraphicsView::keyPressEvent(event);
            if(event->isAccepted())return;
            emit KeyPressSignal(event);

        }

        virtual void mousePressEvent(QMouseEvent* Event)override{
            if(Event->button() == Qt::MiddleButton)
                ToggleDragMode();
            QGraphicsView::mousePressEvent(Event);
        }

        virtual void mouseMoveEvent(QMouseEvent* Event)override{
            QGraphicsView::mouseMoveEvent(Event);
        }

        virtual void mouseReleaseEvent(QMouseEvent* Event)override{
            QGraphicsView::mouseReleaseEvent(Event);
        }
    public:
        KCanvasView(){
            const int SceneSize = 65536;
            Scene->setSceneRect(
                        -SceneSize / 2,
                        -SceneSize / 2,
                        SceneSize,
                        SceneSize
                        );
            this->setScene(Scene);
            setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setRenderHint(QPainter::Antialiasing);
        }

        KScene* GetScene(){
            return Scene;
        }

        static void ScrollToCenter(QScrollBar* Bar){
            int Min = Bar->minimum();
            int Max = Bar->maximum();
//            qDebug()<<Min<<Max;
//            int Len = Bar->pageStep();
            Bar->setValue((Max + Min) / 2);
        }

        void ScrollToCenter(){
            this->centerOn(0, 0);
//            ScrollToCenter(this->verticalScrollBar());
//            ScrollToCenter(this->horizontalScrollBar());
        }

        void ToggleDragMode(){
            if(dragMode() == RubberBandDrag){
                setDragMode(ScrollHandDrag);
            }
            else{
                setDragMode(RubberBandDrag);
            }
        }
    };


}
