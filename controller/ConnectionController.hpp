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
    signals:
        void ConnectChangedSignal(KConnectionController* ConnectionController,
                              KItemController* OldItemController,
                              KItemController* NewItemController);
    public slots:
        void OnItemSizeChanged(KItemController* Controller){
            UpdateConnectionDecoration();
        }
        void UpdateSrcPosition(const QPointF& Position){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            ConnectionView->UpdateFrom(Position);
            UpdateConnectionDecoration();
        }
        void UpdateDstPosition(const QPointF& Position){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            ConnectionView->UpdateTo(Position);
            UpdateConnectionDecoration();
        }
        void UpdateConnectionDecoration(){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            QPointF SrcIntersection, SrcDir,
                    DstIntersection, DstDir;
            if(SrcItemController){
                GetIntersecionAndDirection(SrcItemController, SrcIntersection, SrcDir);
                UpdateDecoration(ConnectionView->GetFromDecoration(),
                                 SrcIntersection,
                                 SrcDir,
                                 SrcItemController->GetView()->pos());
                ConnectionView->SetFromDecoration(ConnectionView->GetFromDecoration());
            }
            if(DstItemController){
                GetIntersecionAndDirection(DstItemController, DstIntersection, DstDir);
                UpdateDecoration(ConnectionView->GetToDecoration(),
                                 DstIntersection,
                                 DstDir,
                                 DstItemController->GetView()->pos());
                ConnectionView->SetToDecoration(ConnectionView->GetToDecoration());
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

        void GetIntersecionAndDirection(KItemController* Controller,
                                        QPointF& OutIntersection, QPointF& OutDirection){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            switch (Controller->type()) {
            //////////////////////////////// TextItem ////////////////////////////////
            case KTextItemView::Type:{
                auto View = Controller->GetView();
                const QRectF& Bounding = View->mapToScene(View->boundingRect()).boundingRect();
                switch (ConnectionView->GetShapeType()) {
                    //////////////////////////////// Line ////////////////////////////////
                    case KConnectionView::Line:{
                        QLineF Line(ConnectionView->GetFrom(), ConnectionView->GetTo());
                        RectIntersectLine(Bounding, Line,
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
        void RectIntersectLine(const QRectF& Bounding, const QLineF Line,
                               QPointF& OutIntersection, QPointF& OutDirection){
            QPointF Intersection;
            QLineF Lines[4] = {
                QLineF(Bounding.topLeft(), Bounding.topRight()),
                QLineF(Bounding.bottomLeft(), Bounding.bottomRight()),
                QLineF(Bounding.topLeft(), Bounding.bottomLeft()),
                QLineF(Bounding.topRight(), Bounding.bottomRight())
            };
            for(int i = 0; i < 4; i++){
                auto IntersectType = Line.intersect(Lines[i], &Intersection);
                if(IntersectType == QLineF::IntersectType::BoundedIntersection){
                    break;
                }
            }
            OutIntersection = Intersection;
            OutDirection = (Line.p1() - Line.p2());
            if(!KNormalize(OutDirection)){
                qWarning()<<"Direction is zero vector.";
            }
        }
    public:
        KConnectionController():KGraphicsObjectController(new KConnectionView){

        }
        ~KConnectionController(){
            auto ConnectionView = static_cast<KConnectionView*>(GraphicsObject);
            if(ConnectionView->scene()){
                ConnectionView->scene()->removeItem(ConnectionView);
            }
            delete ConnectionView;
        }

        void SetSrcItemController(KItemController* ItemController){
            auto OldController = SrcItemController;
            SrcItemController = ItemController;
            if(SrcItemController){
                auto Pos = SrcItemController->GetView()->pos();
                UpdateSrcPosition(Pos);
                // 在没有Dst时，起始位置和结束位置相同
                if(!DstItemController){
                    UpdateDstPosition(Pos);
                }
                QObject::connect(
                            ItemController,
                            &KItemController::PosChangedSignal,
                            this,
                            &KConnectionController::UpdateSrcPosition);
                QObject::connect(
                            SrcItemController,
                            &KItemController::SizeChangedSignal,
                            this,
                            &KConnectionController::OnItemSizeChanged);
            }
            emit ConnectChangedSignal(this, OldController, SrcItemController);
        }
        void SetDstItemController(KItemController* ItemController){
            auto OldController = DstItemController;
            DstItemController = ItemController;
            if(DstItemController){
                UpdateDstPosition(DstItemController->GetView()->pos());
                QObject::connect(
                            ItemController,
                            &KItemController::PosChangedSignal,
                            this,
                            &KConnectionController::UpdateDstPosition);
                QObject::connect(
                            DstItemController,
                            &KItemController::SizeChangedSignal,
                            this,
                            &KConnectionController::OnItemSizeChanged);
            }
            emit ConnectChangedSignal(this, OldController, DstItemController);
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
