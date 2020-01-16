#pragma once
#include<QGraphicsItem>
namespace Kim {
    class KConnectionView : public QGraphicsItem{
    public:
        enum EShapeType{Line, Quad, Cubic};
    private:
        EShapeType ShapeType;
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
        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) = 0;

    };
}
