#pragma once
#include"ItemController.hpp"
#include"../view/ConnectionView.hpp"
namespace Kim {
    class KConnectionController{
    private:
        KItemController* SrcItemController = nullptr;
        KItemController* DstItemController = nullptr;
        KConnectionView* ConnectionView = new KConnectionView;
    public:
        void SetSrcItemController(KItemController* ItemController){
            SrcItemController = ItemController;
            UpdateSrcPosition(SrcItemController->GetView()->GetGraphics()->pos());
        }
        void SetDstItemController(KItemController* ItemController){
            DstItemController = ItemController;
        }
        KItemController* GetSrcItemController(){
            return SrcItemController;
        }
        KItemController* GetDstItemController(){
            return DstItemController;
        }
        void UpdateSrcPosition(const QPointF& Position){
            ConnectionView->SetFrom(Position);
        }
        void UpdateDstPosition(const QPointF& Position){
            ConnectionView->SetTo(Position);
        }
        KConnectionView* GetConnectionView(){
            return ConnectionView;
        }
    };
}
