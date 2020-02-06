#pragma once
#include"ItemController.hpp"
#include"../view/ConnectionView.hpp"
#include<QGraphicsScene>
#include"GraphicsObjectController.hpp"
#include"ItemController.hpp"
#include"../common/Math.hpp"
namespace Kim {
    class KConnectionController : public KGraphicsObjectController{
        Q_OBJECT
    private:
        /**
         * @brief FoldCount
         * use to record the fold state of the item
         */
        // need serialize
        qint64 Identity = CreateID();
        bool Collapsed = false;
    signals:
        void ConnectChangedSignal(KConnectionController* ConnectionController,
                              KItemController* OldItemController,
                              KItemController* NewItemController);
        void FoldSignal(KConnectionController*);
        void SelectedAllChildrenSignal(KConnectionController*, SelectionType);
        void DestroyedSignal(KConnectionController* Controller);
    public slots:
        void Destroy(){
            delete this;
        }
        void OnItemSizeChanged(KItemController* Controller){
//            UpdateConnectionDecoration();
        }
        void UpdateSrcPosition(const QPointF& Position){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            ConnectionView->UpdateFrom(Position);
//            UpdateConnectionDecoration();
        }
        void UpdateDstPosition(const QPointF& Position){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            ConnectionView->UpdateTo(Position);
//            UpdateConnectionDecoration();
        }
        void UpdateConnectionDecoration(){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            QPointF SrcIntersection, SrcDir,
                    DstIntersection, DstDir;
            if(ConnectionView->IsShowFromDecoration() && SrcItemController){
                auto FromDecor = ConnectionView->GetFromDecoration();
                FromDecor->IsValid = GetIntersecionAndDirection(SrcItemController, SrcIntersection, SrcDir);
                if(FromDecor->IsValid){
                    UpdateDecoration(ConnectionView->GetFromDecoration(),
                                     SrcIntersection,
                                     SrcDir,
                                     SrcItemController->GetView()->pos());
                    ConnectionView->SetFromDecoration(ConnectionView->GetFromDecoration());
                }
            }
            if(ConnectionView->IsShowToDecoration() && DstItemController){
                auto ToDecor = ConnectionView->GetToDecoration();
                ToDecor->IsValid = GetIntersecionAndDirection(DstItemController, DstIntersection, DstDir);
                if(ToDecor->IsValid){
                    UpdateDecoration(ConnectionView->GetToDecoration(),
                                     DstIntersection,
                                     DstDir,
                                     DstItemController->GetView()->pos());
                    ConnectionView->SetToDecoration(ConnectionView->GetToDecoration());
                }
            }
        }

        QPointF CorrectDirection(const QPointF& Intersection,
                                 const QPointF& Dir,
                                 const QPointF& ItemPosition){
            QPointF ItemDir = ItemPosition - Intersection;
            if(QPointF::dotProduct(Dir, ItemDir) > 0){
                return Dir;
            }
            return -Dir;
        }

        void UpdateDecoration(KDecoration* Decoration,
                              const QPointF& Intersection,
                              const QPointF& Dir,
                              const QPointF& ItemPosition){
            QPointF CorrectDir = CorrectDirection(Intersection,
                                                  Dir,
                                                  ItemPosition);
            switch (Decoration->type()) {
            case KArrowDecoration::Type:{
                KArrowDecoration* ArrowDecor = static_cast<KArrowDecoration*>(Decoration);
                ArrowDecor->EndianPoint = Intersection;
                ArrowDecor->FromPoint = Intersection - CorrectDir * ArrowDecor->HSize;
                break;
            }
            case KCircleDecoration::Type:
                break;
            case KRectDecoration::Type:
                break;
            }
        }

        bool GetIntersecionAndDirection(KItemController* Controller,
                                        QPointF& OutIntersection, QPointF& OutDirection){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            switch (Controller->type()) {
            //////////////////////////////// TextItem & ImageItem ////////////////////////////////
            case KTextItemView::Type:
            case KImageItemView::Type:{
                auto View = Controller->GetView();
                const QRectF& Bounding = View->mapToScene(View->boundingRect()).boundingRect();
                switch (ConnectionView->GetShapeType()) {
                    //////////////////////////////// Line ////////////////////////////////
                    case KConnectionView::Line:{
                        QLineF Line(ConnectionView->GetFrom(), ConnectionView->GetTo());
                        return RectIntersectLine(Bounding, Line,
                                          OutIntersection, OutDirection);
                        break;
                    }
                    //////////////////////////////// Quad ////////////////////////////////
                    case KConnectionView::Quad:
                        break;
                    //////////////////////////////// Cubic ////////////////////////////////
                    case KConnectionView::Cubic:
                        break;
                }
                break;
            }
            //////////////////////////////// Default ////////////////////////////////
            default:
                break;
            }
        }

    private:
        KItemController* SrcItemController = nullptr;
        KItemController* DstItemController = nullptr;
        bool RectIntersectLine(const QRectF& Bounding, const QLineF Line,
                               QPointF& OutIntersection, QPointF& OutDirection){
            QPointF Intersection;
            QLineF Lines[4] = {
                QLineF(Bounding.topLeft(), Bounding.topRight()),
                QLineF(Bounding.bottomLeft(), Bounding.bottomRight()),
                QLineF(Bounding.topLeft(), Bounding.bottomLeft()),
                QLineF(Bounding.topRight(), Bounding.bottomRight())
            };
            QLineF::IntersectType IntersectType = QLineF::IntersectType::NoIntersection;
            for(int i = 0; i < 4; i++){
                IntersectType = Line.intersect(Lines[i], &Intersection);
                if(IntersectType == QLineF::IntersectType::BoundedIntersection){
                    break;
                }
            }
            OutIntersection = Intersection;
            OutDirection = (Line.p1() - Line.p2());
            if(!KNormalize(OutDirection)){
                qWarning()<<"Direction is zero vector.";
                return false;
            }
            return IntersectType == QLineF::IntersectType::BoundedIntersection;
        }
    public:
        KConnectionController():KGraphicsObjectController(new KConnectionView){
            auto View = static_cast<KConnectionView*>(GraphicsObject);
            connect(View,
                    &KConnectionView::FoldSignal,
                    this,
                    [=]{
                emit FoldSignal(this);
            });
            connect(View,
                    &KConnectionView::SelectedAllChildrenSignal,
                    this,
                    [=](SelectionType Type){
                emit SelectedAllChildrenSignal(this, Type);
            });
        }
        virtual ~KConnectionController() override{
            emit DestroyedSignal(this);
            delete GraphicsObject;
            if(SrcItemController){
                SrcItemController->OutConnections.removeOne(this);
                SrcItemController->UpdateMark();
            }
            if(DstItemController){
                DstItemController->InConnections.removeOne(this);
            }
            GraphicsObject = nullptr;
            SrcItemController = DstItemController = nullptr;
//            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
//            if(ConnectionView->scene()){
//                ConnectionView->scene()->removeItem(ConnectionView);
//            }
        }

        qint64 GetIdentity(){
            return this->Identity;
        }

        void SetIdentity(qint64 Identity){
            this->Identity = Identity;
        }

        bool IsCollapsed(){
            return Collapsed;
        }

        void SetCollapsed(bool Collapsed){
            this->Collapsed = Collapsed;
            this->GraphicsObject->setVisible(!Collapsed);
        }

        void SetSrcItemController(KItemController* ItemController, bool Connect = true){
            auto OldController = SrcItemController;
            if(OldController){
                OldController->OutConnections.removeOne(this);
            }
            SrcItemController = ItemController;
            GetConnectionView()->SetFromItem(SrcItemController->GetView());
            if(SrcItemController){
                // 在没有Dst时，起始位置和结束位置相同
                if(!DstItemController){
                    UpdateDstPosition(SrcItemController->GetView()->pos());
                }
                if(Connect)
                    ConnectSrcItem();
            }
            emit ConnectChangedSignal(this, OldController, SrcItemController);
        }
        void SetDstItemController(KItemController* ItemController, bool Connect = true){
            auto OldController = DstItemController;
            if(OldController){
                OldController->InConnections.removeOne(this);
            }
            DstItemController = ItemController;
            GetConnectionView()->SetToItem(DstItemController->GetView());
            if(DstItemController){
                if(Connect)
                    ConnectDstItem();
            }
            emit ConnectChangedSignal(this, OldController, DstItemController);
        }
        void ConnectSrcItem(){
            QObject::connect(
                        SrcItemController,
                        &KItemController::PosChangedSignal,
                        this,
                        &KConnectionController::UpdateSrcPosition);
            QObject::connect(
                        SrcItemController,
                        &KItemController::SizeChangedSignal,
                        this,
                        &KConnectionController::OnItemSizeChanged);
            SrcItemController->OutConnections.append(this);
            SrcItemController->UpdateMark();
            UpdateSrcPosition(SrcItemController->GetView()->pos());
        }
        void DisconnectDstItem(){
            disconnect(DstItemController, nullptr, this, nullptr);
            DstItemController->InConnections.removeOne(this);
        }
        void DisconnectSrcItem(){
            disconnect(SrcItemController, nullptr, this, nullptr);
            SrcItemController->OutConnections.removeOne(this);
            SrcItemController->UpdateMark();
        }
        bool IsSrcConnected(){
            return SrcItemController && SrcItemController->OutConnections.contains(this);
        }
        bool IsDstConnected(){
            return DstItemController && DstItemController->InConnections.contains(this);
        }
        void ConnectDstItem(){
            QObject::connect(
                        DstItemController,
                        &KItemController::PosChangedSignal,
                        this,
                        &KConnectionController::UpdateDstPosition);
            QObject::connect(
                        DstItemController,
                        &KItemController::SizeChangedSignal,
                        this,
                        &KConnectionController::OnItemSizeChanged);
            DstItemController->InConnections.append(this);
            UpdateDstPosition(DstItemController->GetView()->pos());
        }
        KItemController* GetSrcItemController(){
            return SrcItemController;
        }
        KItemController* GetDstItemController(){
            return DstItemController;
        }
        KConnectionView* GetConnectionView(){
            return static_cast<KConnectionView*>(GraphicsObject);
        }
    };
}
