#pragma once
#include"../view/CanvasView.hpp"
#include"../view/ItemView.hpp"
#include"ItemController.hpp"
#include"ConnectionController.hpp"
namespace Kim {
    class KCanvasController : public QObject{
        Q_OBJECT
    public:
        struct KSceneContext{
            KConnectionController* DragingConnectionController = nullptr;
        };
    private:
        KCanvasView* CanvasView = new KCanvasView;
        KScene* Scene = new KScene;
        KSceneContext SceneContext;
        QList<KItemController*>ItemViewControlleres;
        QList<KConnectionController*>ConnectionControlleres;
    public slots:
        void OnKeyRelease(QKeyEvent* Event){
            switch (Event->key()) {
            case Qt::Key_Space:
                OnSpaceRelease();
                break;
            default:
                break;
            }
        }
    public:
        KCanvasController(){
            CanvasView->setScene(Scene);
            QObject::connect(CanvasView, &KCanvasView::KeyReleaseSignal,
                             this, &KCanvasController::OnKeyRelease);
            QObject::connect(Scene, &KScene::DragMoveSignal,
                             this, &KCanvasController::OnDragMove);
        }

        KCanvasView* GetCanvasView(){
            return CanvasView;
        }

        void OnSpaceRelease(){
            AddTextItem();
        }

        void OnStartDragDrop(KItemController* View){
            KConnectionController* Controller = new KConnectionController;
            Controller->SetSrcItemController(View);
            Scene->addItem(Controller->GetConnectionView());
            SceneContext.DragingConnectionController = Controller;
        }

        void OnDragMove(QGraphicsSceneDragDropEvent* event){
            if(SceneContext.DragingConnectionController){
                SceneContext.DragingConnectionController->UpdateDstPosition(event->scenePos());
                Scene->update();
            }
        }

        void AddTextItem(){
            KTextItemController* Controller = new KTextItemController;
            connect(Controller, &KItemController::StartDragDropSignal,
                    this, &KCanvasController::OnStartDragDrop);
            Scene->addItem(Controller->GetView()->GetGraphics());
            ItemViewControlleres.append(Controller);
        }
    };
}
