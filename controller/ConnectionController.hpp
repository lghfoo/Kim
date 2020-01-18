#pragma once
#include"ItemController.hpp"
#include"../view/ConnectionView.hpp"
#include<QGraphicsScene>
namespace Kim {
    class KConnectionController{
    private:
        KItemController* SrcItemController = nullptr;
        KItemController* DstItemController = nullptr;
        KConnectionView* ConnectionView = new KConnectionView;
    public:
        ~KConnectionController(){
            if(ConnectionView->scene()){
                ConnectionView->scene()->removeItem(ConnectionView);
            }
            if(SrcItemController){
                QObject::disconnect(
                            SrcItemController,
                            &KItemController::PosChangedSignal,
                            ConnectionView,
                            &KConnectionView::UpdateFrom);
            }
            if(DstItemController){
                QObject::disconnect(
                            DstItemController,
                            &KItemController::PosChangedSignal,
                            ConnectionView,
                            &KConnectionView::UpdateFrom);
            }
            // todo:
            //delete ConnectionView;
//            ConnectionView = nullptr;
        }
        void SetSrcItemController(KItemController* ItemController){
            SrcItemController = ItemController;
            UpdateSrcPosition(SrcItemController->GetView()->GetCenterPos());
            QObject::connect(
                        ItemController,
                        &KItemController::PosChangedSignal,
                        ConnectionView,
                        &KConnectionView::UpdateFrom);
        }
        void SetDstItemController(KItemController* ItemController){
            DstItemController = ItemController;
            UpdateDstPosition(SrcItemController->GetView()->GetCenterPos());
            QObject::connect(
                        ItemController,
                        &KItemController::PosChangedSignal,
                        ConnectionView,
                        &KConnectionView::UpdateTo);
        }
        KItemController* GetSrcItemController(){
            return SrcItemController;
        }
        KItemController* GetDstItemController(){
            return DstItemController;
        }
        void UpdateSrcPosition(const QPointF& Position){
            ConnectionView->UpdateFrom(Position);
        }
        void UpdateDstPosition(const QPointF& Position){
            ConnectionView->UpdateTo(Position);
        }
        KConnectionView* GetConnectionView(){
            return ConnectionView;
        }
    };
}
