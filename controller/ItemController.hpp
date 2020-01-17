#pragma once
#include"../view/ItemView.hpp"
namespace Kim {
    class KItemController : public QObject{
        Q_OBJECT
    signals:
        void StartDragDropSignal(KItemController* View);
    public slots:
        void EmitStartDragDropSignal(){
            emit StartDragDropSignal(this);
        }
    protected:
        KItemView* ItemView = nullptr;
    public:
        KItemController(KItemView* ItemView):ItemView(ItemView){
            connect(ItemView,
                    &KItemView::StartDragDropSignal,
                    this,
                    &KItemController::EmitStartDragDropSignal);
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
