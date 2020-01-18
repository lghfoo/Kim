#pragma once
#include"../view/ItemView.hpp"
namespace Kim {
    class KItemController : public QObject{
        Q_OBJECT
    signals:
        void StartConnectingSignal(KItemController* Controller);
        void EndConnectingSignal(KItemController* Controller);
        void IgnoreDropSignal(KItemController* Controller);
        void PosChangedSignal(const QPointF& NewPow);
    public slots:
        void EmitStartConnectingSignal(){
            emit StartConnectingSignal(this);
        }
        void EmitEndConnectingSignal(){
            emit EndConnectingSignal(this);
        }
        void EmitPosChangeSignal(){
            emit PosChangedSignal(ItemView->GetCenterPos());
        }
        void EmitIgnoreDropSignal(){
            emit IgnoreDropSignal(this);
        }
    protected:
        KItemView* ItemView = nullptr;
    public:
        KItemController(KItemView* ItemView):ItemView(ItemView){
            connect(ItemView,
                    &KItemView::StartDragDropSignal,
                    this,
                    &KItemController::EmitStartConnectingSignal);
            connect(ItemView,
                    &KItemView::EndDragDropSignal,
                    this,
                    &KItemController::EmitEndConnectingSignal);
            connect(ItemView,
                    &KItemView::PosChangedSignal,
                    this,
                    &KItemController::EmitPosChangeSignal);
            connect(ItemView,
                    &KItemView::IgnoreDropSignal,
                    this,
                    &KItemController::EmitIgnoreDropSignal);
        }
        KItemView* GetView() {
            return ItemView;
        }
    };

    class KTextItemController : public KItemController{
    public:
        KTextItemController():KItemController(new KTextItemView){
        }
    };
}
