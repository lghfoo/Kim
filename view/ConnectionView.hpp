#pragma once
#include<QGraphicsItem>
#include <QPainter>
namespace Kim {
    class KConnectionView : public QGraphicsItem{
    public:
        enum EShapeType{Line, Quad, Cubic};
    private:
        EShapeType ShapeType;
        QPointF From;
        QPointF CtrlFrom;
        QPointF CtrlTo;
        QPointF To;
    public:
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
        virtual QRectF boundingRect() const = 0;
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
