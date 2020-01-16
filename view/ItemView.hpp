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
namespace Kim {
   class KTextItemView : public QGraphicsSimpleTextItem {
   private:
       qreal Padding = 10.0;
   public:
       virtual QRectF boundingRect() const override{
           QRectF BoundRect = QGraphicsSimpleTextItem::boundingRect();
           QRectF TargetRect(BoundRect.x() - Padding/2.0, BoundRect.y() - Padding/2.0,
                             BoundRect.width() + Padding, BoundRect.height() + Padding);
           return TargetRect;
       }
       KTextItemView(){
           this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);
           this->setText("Please Enter Text....");
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
