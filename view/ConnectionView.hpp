#pragma once
#include<QGraphicsItem>
#include <QPainter>
#include<QGraphicsSceneDragDropEvent>
#include"GraphicsViewBase.hpp"
namespace Kim {
    struct KDecoration{
        virtual int type()const = 0;
        virtual ~KDecoration(){}
        virtual void Paint(QPainter* Painter) = 0;
    };

    struct KArrowDecoration : public KDecoration{
        enum {Type = 0};
        QPointF EndianPoint = QPointF(0, 0);
        QPointF FromPoint = QPointF(0, 0);
        qreal VSize = 10;
        qreal HSize = 16;
        virtual int type()const override{return Type;}
        virtual void Paint(QPainter* Painter)override{
            QPointF TransEndian = EndianPoint - FromPoint;
            qreal Distance = std::hypot(TransEndian.x(), TransEndian.y());
            if(Distance > 0){
                TransEndian /= Distance;
            }
            TransEndian *= VSize;
            QPointF Pos90 = QPointF(-TransEndian.y(), TransEndian.x()) + FromPoint;
            QPointF Neg90 = QPointF(TransEndian.y(), -TransEndian.x()) + FromPoint;
            QPainterPath Path;
            Path.moveTo(Pos90);
            Path.lineTo(Neg90);
            Path.lineTo(EndianPoint);
            Path.lineTo(Pos90);
            Painter->fillPath(Path, QBrush(Qt::black));
        }
    };

    struct KRectDecoration : public KDecoration{
        enum {Type = 1};
        virtual int type()const override{return Type;}
        virtual void Paint(QPainter* Painter)override{
        }
    };

    struct KCircleDecoration : public KDecoration{
        enum {Type = 2};
        virtual int type()const override{return Type;}
        virtual void Paint(QPainter* Painter)override{
        }
    };

    class KConnectionView : public KGraphicsViewBase{
        Q_OBJECT
    public:
        enum KShapeType{Line, Quad, Cubic};
        enum {Type = UserType + 1};
        struct KArrowInfo{
            QPointF EndianPoint = QPointF(0, 0);
            QPointF FromPoint = QPointF(0, 0);
            qreal VSize = 5;
            qreal HSize = 8;
        };

    private:
        KShapeType ShapeType = Line;
        QPointF From;
        QPointF CtrlFrom;
        QPointF CtrlTo;
        QPointF To;
        bool ShowFromDecoration = false;
        bool ShowToDecoration = true;
        KDecoration* FromDecoration = new KArrowDecoration;
        KDecoration* ToDecoration = new KArrowDecoration;
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
        virtual int type()const override{
            return Type;
        }
        KConnectionView(){
            this->setZValue(-1);
        }
        QPointF GetFrom(){return From;}
        QPointF GetTo(){return To;}
        QPointF GetCtrlFrom(){return CtrlFrom;}
        QPointF GetCtrlTo(){return CtrlTo;}
        KDecoration* GetFromDecoration(){
            return FromDecoration;
        }
        KDecoration* GetToDecoration(){
            return ToDecoration;
        }
        void SetFromDecoration(KDecoration* Decoration){
            this->prepareGeometryChange();
            if(Decoration != FromDecoration){
                delete FromDecoration;
                FromDecoration = Decoration;
            }
            this->update();
        }
        void SetToDecoration(KDecoration* Decoration){
            this->prepareGeometryChange();
            if(Decoration != ToDecoration){
                delete ToDecoration;
                ToDecoration = Decoration;
            }
            this->update();
        }
        void SetShapeType(KShapeType ShapeType){
            switch (ShapeType) {
            case KShapeType::Line:
                break;
            case KShapeType::Quad:
                break;
            case KShapeType::Cubic:
                break;
            }
        }
        KShapeType GetShapeType(){
            return ShapeType;
        }
        virtual QRectF boundingRect() const override{
            qreal Padding = 2.0;
            qreal X = std::min(From.x(), To.x()) - Padding/2.0;
            qreal Y = std::min(From.y(), To.y()) - Padding/2.0;
            qreal W = std::abs(To.x() - From.x()) + Padding;
            qreal H = std::abs(To.y() - From.y()) + Padding;
            return QRectF(X, Y, W, H);
        }

        QPainterPath GetShape()const{
            QPainterPath Path;
            Path.moveTo(From);
            switch (ShapeType) {
            case KShapeType::Line:
                Path.lineTo(To);
                break;
            case KShapeType::Quad:
                Path.quadTo(CtrlFrom, To);
                break;
            case KShapeType::Cubic:
                Path.cubicTo(CtrlFrom, CtrlTo, To);
                break;
            }
            return Path;
        }

        virtual QPainterPath shape()const override{
            QPainterPathStroker Stroker;
            Stroker.setWidth(16);
            return Stroker.createStroke(GetShape());
        }
        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override{
            QPen Pen = painter->pen();
            if(this->isSelected()){
                Pen.setWidth(3);
            }
            else{
                Pen.setWidth(1);
            }
            painter->setPen(Pen);
            painter->drawPath(GetShape());

            if(ShowToDecoration){
                ToDecoration->Paint(painter);
            }
            if(ShowFromDecoration){
                FromDecoration->Paint(painter);
            }
        }

    };
}
