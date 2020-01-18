#pragma once
#include<QGraphicsItem>
#include<QGraphicsTextItem>
#include<QGraphicsSimpleTextItem>
#include<QGraphicsWidget>
#include<QGraphicsProxyWidget>
#include<QPainter>
#include<QGraphicsSceneMouseEvent>
#include<QDrag>
#include<QWidget>
#include<QMimeData>
#include<QDebug>
namespace Kim {
    class KItemView : public QObject{
        Q_OBJECT
    signals:
        void StartDragDropSignal();
        void EndDragDropSignal();
        void IgnoreDropSignal();
        void PosChangedSignal();
    public:
        virtual QGraphicsItem* GetGraphics() = 0;
        virtual QPointF GetCenterPos() const = 0;
    };

   class KTextItemView: public QGraphicsSimpleTextItem, public KItemView {
   private:
       qreal Padding = 10.0;
   public:
       virtual QVariant	itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)override{
           if(change == QGraphicsItem::GraphicsItemChange::ItemPositionHasChanged){
               emit PosChangedSignal();
           }
           return QGraphicsItem::itemChange(change, value);
       }
       virtual QRectF boundingRect() const override{
           QRectF BoundRect = QGraphicsSimpleTextItem::boundingRect();
           QRectF TargetRect(BoundRect.x() - Padding/2.0, BoundRect.y() - Padding/2.0,
                             BoundRect.width() + Padding, BoundRect.height() + Padding);
           return TargetRect;
       }
       virtual QPointF GetCenterPos() const override{
           const QRectF& bounding = boundingRect();
           return QPointF(this->pos().x() + bounding.x() + bounding.width() / 2.0,
                          this->pos().y() + bounding.y() + bounding.height() / 2.0);
       }
       KTextItemView(){
           this->setAcceptDrops(true);
           this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
           this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);
           this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges);
           this->setText("Please Enter Text....");
       }
       virtual QGraphicsItem* GetGraphics() override{
           return this;
       }
       virtual void	paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override{
           QPainterPath Path;
           const QRectF& Bounding = this->boundingRect();
           Path.moveTo(Bounding.topLeft());
           Path.addRoundedRect(Bounding, 5.0, 5.0);
           painter->fillPath(Path, QBrush(Qt::white));
           QGraphicsSimpleTextItem::paint(painter, option, widget);
           painter->drawRoundedRect(this->boundingRect(), 5.0, 5.0);

       }

       virtual void dropEvent(QGraphicsSceneDragDropEvent *event)override{
           emit EndDragDropSignal();
           event->acceptProposedAction();
       }

       virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override{
           if(event->modifiers() & Qt::ControlModifier){
                emit StartDragDropSignal();
               QMimeData* Data = new QMimeData;
               QDrag* Drag = new QDrag(event->widget());
               Drag->setMimeData(Data);
               Qt::DropAction DropAction = Drag->exec();
               if(DropAction == Qt::IgnoreAction){
                   emit IgnoreDropSignal();
               }
           }
           else{
                QGraphicsSimpleTextItem::mouseMoveEvent(event);
           }
       }

       virtual void keyReleaseEvent(QKeyEvent *event)override{
            printf("item key release\n");
       }
   };

   class KTextItemPropertyView{

   };
}
