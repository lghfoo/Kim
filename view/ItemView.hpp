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
namespace Kim {
    class KItemView : public QObject{
        Q_OBJECT
    signals:
        void StartDragDropSignal();
    public:
        virtual QGraphicsItem* GetGraphics() = 0;
        virtual QPointF GetCenterPos() const = 0;
    };

   class KTextItemView: public QGraphicsSimpleTextItem, public KItemView {
   private:
       qreal Padding = 10.0;
   public:
       virtual QRectF boundingRect() const override{
           QRectF BoundRect = QGraphicsSimpleTextItem::boundingRect();
           QRectF TargetRect(BoundRect.x() - Padding/2.0, BoundRect.y() - Padding/2.0,
                             BoundRect.width() + Padding, BoundRect.height() + Padding);
           return TargetRect;
       }
       virtual QPointF GetCenterPos() const override{
           const QRectF& bounding = boundingRect();
           return QPointF(bounding.x() + bounding.width() / 2.0,
                          bounding.y() + bounding.height() / 2.0);
       }
       KTextItemView(){
           this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);
           this->setText("Please Enter Text....");
       }
       virtual QGraphicsItem* GetGraphics() override{
           return this;
       }
       virtual void	paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override{
           QGraphicsSimpleTextItem::paint(painter, option, widget);
           painter->drawRoundedRect(this->boundingRect(), 5.0, 5.0);

       }
       virtual void	dragMoveEvent(QGraphicsSceneDragDropEvent * event)override{
           printf("Drag Move\n");
       }
       virtual void	mouseMoveEvent(QGraphicsSceneMouseEvent * event)override{
           if(event->modifiers() & Qt::ControlModifier){
                QDrag *Drag = new QDrag(event->widget());
                emit StartDragDropSignal();
                Drag->setMimeData(new QMimeData());
                Drag->exec();
           }
           else{
                QGraphicsSimpleTextItem::mouseMoveEvent(event);
           }
       }
   };

   class KTextItemPropertyView{

   };
}
