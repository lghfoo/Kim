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

        void OnStartConnecting(KItemController* ItemController){
            if(SceneContext.DragingConnectionController)return;
            KConnectionController* Controller = new KConnectionController;
            Controller->SetSrcItemController(ItemController);
            Scene->addItem(Controller->GetConnectionView());
            SceneContext.DragingConnectionController = Controller;
        }

        void OnEndConnecting(KItemController* ItemController){
            if(SceneContext.DragingConnectionController){
                if(ItemController){
                    SceneContext.DragingConnectionController->SetDstItemController(ItemController);
                    SceneContext.DragingConnectionController = nullptr;
                }
            }
        }

        void OnItemIgnoreDrop(KItemController* ItemController){
            printf("ignore drop\n");
            if(SceneContext.DragingConnectionController){
//                Scene->removeItem(SceneContext.DragingConnectionController->GetConnectionView());
                delete SceneContext.DragingConnectionController;
                SceneContext.DragingConnectionController = nullptr;
                Scene->update();
            }
        }
    public:
        KCanvasController(){
            CanvasView->setScene(Scene);
            QObject::connect(CanvasView, &KCanvasView::KeyReleaseSignal,
                             this, &KCanvasController::OnKeyRelease);
            QObject::connect(Scene, &KScene::DragMoveSignal,
                             this, &KCanvasController::OnDragMove);
            QObject::connect(Scene, &KScene::MouseReleaseSignal,
                             this, &KCanvasController::OnMouseRelease);

        }

        KCanvasView* GetCanvasView(){
            return CanvasView;
        }

        void OnSpaceRelease(){
            AddTextItem();
        }

        void OnMouseRelease(QGraphicsSceneMouseEvent *mouseEvent){
//            if(SceneContext.DragingConnectionController){
//                auto Controller = CreateTextItem();
//                SceneContext.DragingConnectionController->SetDstItemController(Controller);
//                AddItemAt(Controller->GetView()->GetGraphics(),
//                          SceneContext.DragingConnectionController->GetConnectionView()->GetTo());
//                ItemViewControlleres.append(Controller);
//                SceneContext.DragingConnectionController = nullptr;
//            }
        }

        void OnDragMove(QGraphicsSceneDragDropEvent* event){
            if(SceneContext.DragingConnectionController){
                SceneContext.DragingConnectionController->UpdateDstPosition(event->scenePos());
                Scene->update();
            }
        }

        void AddTextItem(){
            auto Controller = CreateTextItem();
            Scene->addItem(Controller->GetView()->GetGraphics());
            ItemViewControlleres.append(Controller);
        }

        KTextItemController* CreateTextItem(){
            KTextItemController* Controller = new KTextItemController;
            connect(Controller, &KItemController::StartConnectingSignal,
                    this, &KCanvasController::OnStartConnecting);
            connect(Controller, &KItemController::EndConnectingSignal,
                    this, &KCanvasController::OnEndConnecting);
            connect(Controller, &KItemController::PosChangedSignal,
                    this, [=]{Scene->update();});
            connect(Controller, &KItemController::IgnoreDropSignal,
                    [=](KItemController* Controller){
                printf("ignore drop\n");
                if(SceneContext.DragingConnectionController){
    //                Scene->removeItem(SceneContext.DragingConnectionController->GetConnectionView());
                    delete SceneContext.DragingConnectionController;
                    SceneContext.DragingConnectionController = nullptr;
                    Scene->update();
                }
            });
            return Controller;
        }

        void AddItemAt(QGraphicsItem* Item, const QPointF& Pos){
            Item->setPos(Pos);
            Scene->addItem(Item);
        }
    };
}
