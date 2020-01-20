#pragma once
#include<QGraphicsItem>
#include <QPainter>
#include<QGraphicsSceneDragDropEvent>
namespace Kim {
    class KConnectionView : public QObject, public QGraphicsItem{
        Q_OBJECT
    public:
        enum EShapeType{Line, Quad, Cubic};
    private:
        EShapeType ShapeType = Line;
        QPointF From;
        QPointF CtrlFrom;
        QPointF CtrlTo;
        QPointF To;
    public slots:
        void UpdateFrom(const QPointF& From){
            this->prepareGeometryChange();
            this->From = From;
            this->update();
        }
        void UpdateTo(const QPointF& To){
            this->prepareGeometryChange();
            this->To = To;
            this->update();
        }
    public:
        KConnectionView(){
            this->setZValue(-1);
        }
        QPointF GetFrom(){return From;}
        QPointF GetTo(){return To;}
        QPointF GetCtrlFrom(){return CtrlFrom;}
        QPointF GetCtrlTo(){return CtrlTo;}
        void SetShapeType(EShapeType ShapeType){
            switch (ShapeType) {
            case EShapeType::Line:
                break;
            case EShapeType::Quad:
                break;
            case EShapeType::Cubic:
                break;
            default:
                break;
            }
        }
        virtual QRectF boundingRect() const override{
            qreal Padding = 2.0;
            qreal X = std::min(From.x(), To.x()) - Padding/2.0;
            qreal Y = std::min(From.y(), To.y()) - Padding/2.0;
            qreal W = std::abs(To.x() - From.x()) + Padding;
            qreal H = std::abs(To.y() - From.y()) + Padding;
            return QRectF(X, Y, W, H);
        };
        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override{
            QPainterPath Path;
            Path.moveTo(From);
            switch (ShapeType) {
            case EShapeType::Line:
                Path.lineTo(To);
                break;
            case EShapeType::Quad:
                Path.quadTo(CtrlFrom, To);
                break;
            case EShapeType::Cubic:
                Path.cubicTo(CtrlFrom, CtrlTo, To);
                break;
            default:
                break;
            }
            painter->drawPath(Path);
        }
    };
}
