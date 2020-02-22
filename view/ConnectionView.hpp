#pragma once
#include<QGraphicsItem>
#include <QPainter>
#include<QGraphicsSceneDragDropEvent>
#include <QKeyEvent>
#include<QDebug>
#include <QLinkedList>
#include"GraphicsViewBase.hpp"
#include"../common/Utility.hpp"
namespace Kim {
    //////////////////////////////// Decoration ////////////////////////////////
    struct KDecoration{
        bool IsValid = true;
        virtual int type()const = 0;
        virtual ~KDecoration(){}
        virtual void Paint(QPainter* Painter, const QBrush& Brush = Qt::black) = 0;
        virtual KDecoration* Clone(){return nullptr;}
    };

    struct KArrowDecoration : public KDecoration{
        enum {Type = 0};
        QPointF EndianPoint = QPointF(0, 0);
        QPointF FromPoint = QPointF(0, 0);
        qreal VSize = 6;
        qreal HSize = 10;
        virtual int type()const override{return Type;}
        virtual void Paint(QPainter* Painter, const QBrush& Brush = Qt::black)override{
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
            Painter->fillPath(Path, Brush);
        }
        virtual KDecoration* Clone()override{
            KArrowDecoration* Arrow = new KArrowDecoration;
            Arrow->EndianPoint = this->EndianPoint;
            Arrow->FromPoint = this->FromPoint;
            Arrow->VSize = this->VSize;
            Arrow->HSize = this->HSize;
            return Arrow;
        }
    };

    struct KRectDecoration : public KDecoration{
        enum {Type = 1};
        virtual int type()const override{return Type;}
        virtual void Paint(QPainter* Painter, const QBrush& Brush = Qt::black)override{
        }
        virtual KDecoration* Clone()override{
            KRectDecoration* Rect = new KRectDecoration;
            return Rect;
        }
    };

    struct KCircleDecoration : public KDecoration{
        enum {Type = 2};
        virtual int type()const override{return Type;}
        virtual void Paint(QPainter* Painter, const QBrush& Brush = Qt::black)override{
        }
        virtual KDecoration* Clone()override{
            KCircleDecoration* Circle = new KCircleDecoration;
            return Circle;
        }
    };
    //////////////////////////////// Connection ////////////////////////////////
    class KControlPoint : public QGraphicsObject{
        Q_OBJECT
    public:
        enum {Type = ViewType::ControlPointType};
    signals:
        void PosChangedSignal();
        void DeletedSignal(KControlPoint*);
    private:
        qreal Radius = 3.0;
    public:
        virtual int type()const override{
            return Type;
        }
        KControlPoint(){
            this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
            this->setFlag(QGraphicsItem::ItemIsMovable);
            this->setFlag(QGraphicsItem::ItemIsSelectable);
//            this->setBrush(Qt::black);
//            qreal Radius = 3;
//            this->setRect({-Radius, -Radius, Radius * 2, Radius * 2});
        }
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)override{
            if(change == QGraphicsItem::GraphicsItemChange::ItemPositionHasChanged){
                emit PosChangedSignal();
            }
            else if(change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged){
                if(this->isSelected()){
                    this->setFocus();
                }
            }
            return QGraphicsObject::itemChange(change, value);
        }
        virtual void paint(QPainter* Painter, const QStyleOptionGraphicsItem* Option, QWidget* Parent = nullptr)override{
            QPainterPath Path;
            QRectF Rect = {-Radius, -Radius, Radius * 2, Radius * 2};
            if(this->isSelected()){
                Rect = PaddingOut(Rect, 3);
            }
            Path.addEllipse(Rect);
            Painter->fillPath(Path, Qt::black);
        }

        virtual QRectF boundingRect()const override{
            return PaddingOut({-Radius, -Radius, Radius * 2, Radius * 2}, 5);
        }

        void SetRadius(qreal Radius){
            this->prepareGeometryChange();
            this->Radius = Radius;
            this->update();
        }

//        virtual void keyPressEvent(QKeyEvent* Event) override {
//            qDebug()<<"key press"<<Event->key();
//            if(Event->key() == Qt::Key_Delete){
//                emit DeletedSignal(this);
//            }
//        }
//        virtual bool sceneEvent(QEvent* Event)override{
//            qDebug()<<Event;
//            return QGraphicsObject::sceneEvent(Event);
//        }
    };

    class KConnectionView : public KGraphicsViewBase{
        Q_OBJECT
    public:
        enum KShapeType{Line, Quad, Cubic};
        enum {Type = ViewType::ConnectionType};
        struct KArrowInfo{
            QPointF EndianPoint = QPointF(0, 0);
            QPointF FromPoint = QPointF(0, 0);
            qreal VSize = 5;
            qreal HSize = 8;
        };
    signals:
        void FoldSignal();
        void SelectedAllChildrenSignal(SelectionType);
    private:
        KShapeType ShapeType = Line;
        QPointF From;
        QPointF CtrlFrom;
        QPointF CtrlTo;
        QPointF To;
        bool ShowFromDecoration = false;
        bool ShowToDecoration = true;
        bool SelectedOnly = false;
        KDecoration* FromDecoration = new KArrowDecoration;
        KDecoration* ToDecoration = new KArrowDecoration;
        QLinkedList<KControlPoint*>Ctrls={};
        QGraphicsItem* FromItem = nullptr;
        QGraphicsItem* ToItem = nullptr;
        qreal FromT = 1;
        qreal ToT = 1;
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
        void DeleteControlPoint(KControlPoint* Control){
            this->prepareGeometryChange();
            Ctrls.removeOne(Control);
            delete Control;
            this->update();
        }
    protected:
        virtual bool sceneEvent(QEvent* Event)override{
            if(SelectedOnly){
                if(Event->type() == QEvent::GraphicsSceneMousePress){
                    return KGraphicsViewBase::sceneEvent(Event);
                }
                return true;
            }
            switch (Event->type()) {
            case QEvent::KeyPress:{
                auto KeyEvent = static_cast<QKeyEvent*>(Event);
                if(KeyEvent->key() == Qt::Key_H){
                    emit FoldSignal();
                }
                else if(KeyEvent->key() == Qt::Key_L){
                    if(KeyEvent->modifiers() & Qt::AltModifier){
                        emit SelectedAllChildrenSignal(SelectionType::Reverse);
                    }
                    else if(KeyEvent->modifiers() & Qt::ShiftModifier){
                        emit SelectedAllChildrenSignal(SelectionType::None);
                    }
                    else{
                        emit SelectedAllChildrenSignal(SelectionType::All);
                    }
                }
                break;
            }
            default:
                break;
            }
            return KGraphicsViewBase::sceneEvent(Event);
        }
    public:
        virtual int type()const override{
            return Type;
        }

        KConnectionView(){
            this->setZValue(-1);
        }

        virtual ~KConnectionView() override{
            delete FromDecoration;
            delete ToDecoration;
        }

        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)override{
            if(change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged){
                if(this->isSelected()){
                    this->setFocus(Qt::FocusReason::NoFocusReason);
                }
            }
            return KGraphicsViewBase::itemChange(change, value);
        }

        void SetFromItem(QGraphicsItem* FromItem){
            this->FromItem = FromItem;
        }

        void SetToItem(QGraphicsItem* ToItem){
            this->ToItem = ToItem;
        }

        QGraphicsItem* GetFromItem(){
            return this->FromItem;
        }

        QGraphicsItem* GetToItem(){
            return this->ToItem;
        }

        virtual KGraphicsViewBase* Clone() override{
            KConnectionView* View = new KConnectionView;
            View->ShapeType = this->ShapeType;
            View->From = this->From;
            View->To = this->To;
            View->setPos(this->pos());
            View->CtrlFrom = this->CtrlFrom;
            View->CtrlTo = this->CtrlTo;
            View->ShowFromDecoration = this->ShowFromDecoration;
            View->ShowToDecoration = this->ShowToDecoration;
            View->FromDecoration = this->FromDecoration->Clone();
            View->ToDecoration = this->ToDecoration->Clone();
            return View;
        }

        void SetSelectedOnly(bool Only){
            this->SelectedOnly = Only;
        }

        bool IsSelectedOnly()const{
            return SelectedOnly;
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

        bool IsShowFromDecoration(){return ShowFromDecoration;}

        bool IsShowToDecoration(){return ShowToDecoration;}

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
            const qreal DecorPadding = 20;
            QRectF Rect{0,0,0,0};
            const auto& Points = CreatePoints();
            for(const auto& P : Points){
                ExtendRect(Rect, P);
            }
            return PaddingOut(Rect, 3.0 + DecorPadding);
        }

        QPainterPath GetShape()const{
            QPainterPath Path;
            const auto& Points = CreatePoints();
            Path.addPolygon({Points});
            return Path;
//            Path.moveTo(From);
//            const auto& Lines = CreateLines();
//            for(const auto& Line : Lines){
//                Path.moveTo(Line.p1());
//                Path.lineTo(Line.p2());
//            }
//            switch (ShapeType) {
//            case KShapeType::Line:
//                Path.lineTo(To);
//                break;
//            case KShapeType::Quad:
//                Path.quadTo(CtrlFrom, To);
//                break;
//            case KShapeType::Cubic:
//                Path.cubicTo(CtrlFrom, CtrlTo, To);
//                break;
//            }
            return Path;
        }

        virtual QPainterPath shape()const override{
            QPainterPathStroker Stroker;
            Stroker.setWidth(16);
            return Stroker.createStroke(GetShape());
        }

        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0) override{
            QPen Pen = painter->pen();
            QBrush Brush = Qt::black;
            if(this->isSelected()){
                Pen.setWidth(3);
            }
            else{
                Pen.setWidth(1);
            }
            if(!(this->isEnabled())){
                Pen.setColor(Qt::lightGray);
                Brush.setColor(Qt::lightGray);
            }
            Pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
            painter->setPen(Pen);
//            const auto& Lines = CreateLines();
//            painter->drawLines(Lines);
            auto Points = CreatePoints();
//            for(const auto& Line : Lines){
//                painter->drawLine(Line);
//            }
//            painter->drawPath(GetShape());


//            if(FromItem){
//                const auto Rect = FromItem->boundingRect();
//                auto StartPoint = Points.front();
//                if(!Rect.contains(StartPoint)){

//                }
//                else{
//                    for(int i = 0; i < Points.size(); ++i){

//                    }

//                }
//            }
            if(ToItem){
                const auto Bounding = PaddingOut(ToItem->mapToScene(ToItem->boundingRect()).boundingRect(), 2);
                auto EndPoint = Points.back();
                if(Bounding.contains(EndPoint)){
                    QLineF Lines[4] = {
                        QLineF(Bounding.topLeft(), Bounding.topRight()),
                        QLineF(Bounding.bottomLeft(), Bounding.bottomRight()),
                        QLineF(Bounding.topLeft(), Bounding.bottomLeft()),
                        QLineF(Bounding.topRight(), Bounding.bottomRight())
                    };
                    while (Points.size() >= 2) {
                        const auto& P0 = Points[Points.size() - 1];
                        const auto& P1 = Points[Points.size() - 2];
                        Points.pop_back();
                        QLineF L{P0, P1};
                        QPointF Result;
                        bool ShouldBreak = false;
                        for(const auto& Line : Lines){
                            auto IntersectType = Line.intersect(L, &Result);
                            if(IntersectType == QLineF::IntersectType::BoundedIntersection){
                                Points.push_back(Result);
                                ShouldBreak = true;
                                break;
                            }
                        }
                        if(ShouldBreak){
                            break;
                        }
                    }

                }
                if(Points.size() >=2 && ShowToDecoration && ToDecoration){
                    EndPoint = Points.back();
                    QPointF EndDir = Points[Points.size() - 2] - Points.back();
                    if(ToDecoration->type() == KArrowDecoration::Type){
                        if(KNormalize(EndDir)){
                            auto ArrowDecor = static_cast<KArrowDecoration*>(ToDecoration);
                            auto HSize = ArrowDecor->HSize;
                            ArrowDecor->FromPoint = EndPoint + EndDir * HSize;
                            ArrowDecor->EndianPoint = EndPoint;
                            Points.back() = (ArrowDecor->FromPoint + ArrowDecor->EndianPoint)/2;
                            ToDecoration->Paint(painter);
                        }
                    }
                }

            }

            painter->drawPolyline(&Points[0], Points.size());

//            if(ShowToDecoration && ToDecoration->IsValid){

//                ToDecoration->Paint(painter, Brush);
//            }

//            if(ShowFromDecoration && FromDecoration->IsValid){
//                FromDecoration->Paint(painter, Brush);
//            }

        }


        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)override{
            auto ScenePos = event->scenePos();
            if(Ctrls.isEmpty()){
                if(InRange(ScenePos.x(), From.x(), To.x())
                        && InRange(ScenePos.y(), From.y(), To.y())){
//                    ToClosestPoint(ScenePos, From, To);
                    Ctrls.append(CreateCtrlItem(ScenePos));
                }
            }
            else {
                QVector<QPointF> Points{From};
                for(auto Item : Ctrls){
                    Points.append(Item->pos());
                }
                Points.append(To);
                QVector<QPointF> Ps{};
                qreal Threshold = 6;
                for(int i = 0; i < Points.size() - 1; i++){
                    bool ShouldBreak = false;
                    Ps.clear();
                    CreatePoints(Ps, i, Points);
                    bool IsInBound = false;

                    for(int j = 0; j < Ps.size() - 1; j++){
                        auto Dis = PointDistanceToLine(ScenePos, {Ps[j], Ps[j+1]}, IsInBound);
                        if(IsInBound){
                            if(Dis < Threshold){
                                auto NewCtrl = CreateCtrlItem(ScenePos);
                                ShouldBreak = true;
                                auto Iter = Ctrls.begin();
                                while(i--){
                                    ++Iter;
                                }
                                Ctrls.insert(Iter, NewCtrl);
                                break;
                            }
                        }
                    }
                    if(ShouldBreak)break;
                }
            }
        }
        KControlPoint* CreateCtrlItem(const QPointF& Pos){
            auto ControlP = new KControlPoint;
            connect(ControlP, &KControlPoint::PosChangedSignal,
                    [=]{
                this->prepareGeometryChange();
                this->update();
            });
            connect(ControlP, &KControlPoint::DeletedSignal,
                    this, &KConnectionView::DeleteControlPoint);
            ControlP->setParentItem(this);
            ControlP->setPos(Pos);
            return ControlP;
        }

        void CreatePoints(QVector<QPointF>& OutputPoints, int i, const QVector<QPointF>& Points)const{
            qreal Scale = 0.5;
            if(i == 0){
                auto P0 = Points[0];
                auto P1 = Points[1];
                auto P2 = Points[2];

                auto V0 = P0 - P1;
                auto V1 = P2 - P1;

                auto Center = KToNormalized(V0) + KToNormalized(V1);
                QPointF Tangent{-Center.y(), Center.x()};
                auto V0Proj = Projection(V0, Tangent) * Scale;
                const auto& F = P0;
                const auto& C = P1 + V0Proj;
                const auto& T = P1;

                CreateQuadPoints(OutputPoints, F, C, T);
            }
            else if(i == Points.size() -2){
                auto P0 = Points[i-1];
                auto P1 = Points[i];
                auto P2 = Points[i+1];

                auto V0 = P0 - P1;
                auto V1 = P2 - P1;
                auto Center = KToNormalized(V0) + KToNormalized(V1);
                QPointF Tangent{-Center.y(), Center.x()};
                auto V1Proj = Projection(V1, Tangent) * Scale;
                const auto& F = P1;
                const auto& C = P1 + V1Proj;
                const auto& T = P2;

                CreateQuadPoints(OutputPoints, F, C, T);
            }
            else{
                auto P0 = Points[i-1];
                auto P1 = Points[i];
                auto P2 = Points[i+1];
                auto P3 = Points[i+2];

                auto V0 = P0 - P1;
                auto V1 = P2 - P1;
                auto V2 = P1 - P2;
                auto V3 = P3 - P2;
                auto Center1 = KToNormalized(V0) + KToNormalized(V1);
                auto Center2 = KToNormalized(V2) + KToNormalized(V3);
                QPointF Tangent1{-Center1.y(), Center1.x()};
                QPointF Tangent2{-Center2.y(), Center2.x()};
                auto V1Proj = Projection(V1, Tangent1) * Scale;
                auto V2Proj = Projection(V2, Tangent2) * Scale;
                const auto& F = P1;
                const auto& C1 = P1 + V1Proj;
                const auto& C2 = P2 + V2Proj;
                const auto& T = P2;

                CreateCubicPoints(OutputPoints, F, C1, C2, T);
            }
        }

        QVector<QPointF> CreatePoints()const{
            QVector<QPointF> OutputPoints{};
            if(Ctrls.isEmpty()){
                OutputPoints.append({From, To});
            }
            else{
                QVector<QPointF> Points{From};
                for(auto Item : Ctrls){
                    Points.append(Item->pos());
                }
                Points.append(To);
                for(int i = 0; i < Points.size() - 1; i++){
                    CreatePoints(OutputPoints, i, Points);
                }
            }
            return OutputPoints;
        }

        QList<QPointF> GetCtrlPoints(){
            QList<QPointF>Ret{};
            for(auto Ctrl : Ctrls){
                Ret.append(Ctrl->pos());
            }
            return Ret;
        }

        void SetCtrlPoints(const QList<QPointF>& Points){
            for(const auto& Point : Points){
                Ctrls.append(CreateCtrlItem(Point));
            }
        }
    };
}
