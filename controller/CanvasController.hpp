#pragma once
#include"../view/CanvasView.hpp"
#include"../view/ItemView.hpp"
#include"ItemController.hpp"
#include"ConnectionController.hpp"
namespace Kim {
    class KTextSerializer;
    class KCanvasController : public QObject{
        Q_OBJECT
        friend class KTextSerializer;
    public:
        struct KSceneContext{
            KConnectionController* DragingConnectionController = nullptr;
        };
        struct KCanvasState{
            /**
             * @brief HasContent
             * HasContent:
             *      false: does not items nor connections
             *      true:  has any items or connections
             */
            bool HasContent = false;
            /**
             * @brief The KMoveTargetType enum
             * MoveItem: 按WASD时移动Item
             * MoveCursor: 按WASD时移动光标
             */
            enum KMoveTargetType{MoveItem, MoveCursor, MoveTargetCount};
            /**
             * @brief The KAddPosType enum
             * AtMouse: 在鼠标处添加Item
             * AtCursor: 在光标处添加Item
             * AtNearest: 在最近处添加Item
             */
            enum KAddPosType{AtMouse, AtCursor, AtNearest, AddPosCount};
            /**
             * @brief WriteDirectly
             * 对文本节点输入时直接覆盖文本
             */
            bool WriteDirectly;
            KMoveTargetType MoveTargetType = MoveCursor;
            KAddPosType AddPosType = AtCursor;
            void ToNextMoveTargetType(){
                this->MoveTargetType = KMoveTargetType((this->MoveTargetType + 1) % MoveTargetCount);
            }
            void ToNextAddPosType(){
                this->AddPosType = KAddPosType((this->AddPosType + 1) % AddPosCount);
            }
        };
    private:
        KCanvasState CanvasState;
        KCanvasView* CanvasView = new KCanvasView;
        KScene* Scene = new KScene;
        KSceneContext SceneContext;
        QList<KItemController*>ItemViewControlleres;
        QList<KConnectionController*>ConnectionControlleres;
    signals:
        void SaveSingal();
        void SaveAsSignal();
        void LoadSignal();
    public slots:
        void OnKeyPress(QKeyEvent* Event){
            // Ctrl + Alt + Key
            if(Event->modifiers() & Qt::ControlModifier
                    && Event->modifiers() & Qt::AltModifier){
                switch (Event->key()) {
                case Qt::Key_S:
                    emit SaveAsSignal();
                    break;
                default:
                    break;
                }
            }
            // Ctrl + Key
            else if(Event->modifiers() & Qt::ControlModifier){
                switch (Event->key()) {
                case Qt::Key_S:
                    emit SaveSingal();
                    break;
                case Qt::Key_O:
                    emit LoadSignal();
                    break;
                default:
                    break;
                }
            }
            // Alt + Key
            else if(Event->modifiers() & Qt::AltModifier){
            }
            else{
                switch (Event->key()) {
                case Qt::Key_Space:
                    OnSpaceRelease();
                    break;
                // WASD移动
                case Qt::Key_W:
                    MoveTarget(0, -1);
                    break;
                case Qt::Key_A:
                    MoveTarget(-1, 0);
                    break;
                case Qt::Key_S:
                    MoveTarget(0, 1);
                    break;
                case Qt::Key_D:
                    MoveTarget(1, 0);
                    break;
                // 切换移动对象
                case Qt::Key_M:
                    CanvasState.ToNextMoveTargetType();
                    break;
                // 切换添加位置
                case Qt::Key_P:
                    CanvasState.ToNextAddPosType();
                    break;
                default:
                    break;
                }
            }
        }

        void MoveTarget(int XCount, int YCount){
            using KGrid = KScene::KGrid;
            KGrid Grid = Scene->GetGrid();
            qreal Dx = Grid.CellW * XCount;
            qreal Dy = Grid.CellH * YCount;
            if(CanvasState.MoveTargetType == CanvasState.MoveCursor){
                Scene->SetCursorPos(Scene->GetCursorPos() + QPointF(Dx, Dy));
            }
            else if(CanvasState.MoveTargetType == CanvasState.MoveItem){

            }
        }

        void OnStartConnecting(KItemController* ItemController){
            if(SceneContext.DragingConnectionController)return;
            KConnectionController* Controller = new KConnectionController;
            Controller->SetSrcItemController(ItemController);
//            Scene->addItem(Controller->GetConnectionView());
            AddItemAt(Controller->GetConnectionView());
            ConnectionControlleres.append(Controller);
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
            if(SceneContext.DragingConnectionController){
                auto EndPos = SceneContext.DragingConnectionController->GetConnectionView()->GetTo();
                auto ItemView = ItemController->GetView();
                auto Bounding = ItemView->ToGraphics()->boundingRect();
                auto Pos = QPointF(EndPos.x() - Bounding.width() / 2.0,
                                   EndPos.y() - Bounding.height() / 2.0);
                auto TextItemController = CreateTextItem();
                ItemViewControlleres.append(TextItemController);
                SceneContext.DragingConnectionController->SetDstItemController(TextItemController);
                AddItemAt(TextItemController->GetView()->ToGraphics(), Pos);
                SceneContext.DragingConnectionController = nullptr;
            }
        }
    public:
        KCanvasController(){
            CanvasView->setScene(Scene);
            QObject::connect(CanvasView, &KCanvasView::KeyPressSignal,
                             this, &KCanvasController::OnKeyPress);
            QObject::connect(Scene, &KScene::DragMoveSignal,
                             this, &KCanvasController::OnDragMove);
            QObject::connect(Scene, &KScene::MouseReleaseSignal,
                             this, &KCanvasController::OnMouseRelease);

        }

        const KCanvasState& GetCanvasState(){
            return CanvasState;
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
            AddItem(Controller);
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
                    this, &KCanvasController::OnItemIgnoreDrop);
            return Controller;
        }

        void AddItem(KItemController* ItemController){
            QPointF Pos(0, 0);
            switch (CanvasState.AddPosType) {
            case KCanvasState::AtMouse:
                break;
            case KCanvasState::AtCursor:
                Pos = Scene->GetCursorPos();
                break;
            case KCanvasState::AtNearest:
                break;
            default:
                break;
            }
            auto Item = ItemController->GetView()->ToGraphics();
            AddItemAt(Item, Pos);
        }

        void AddItemAt(QGraphicsItem* Item, const QPointF& Pos = QPointF(0, 0)){
            Item->setPos(Pos);
            Scene->addItem(Item);
            OnAddItem(Item);
        }

        void OnAddItem(QGraphicsItem* Item){
            Q_UNUSED(Item)
            CanvasState.HasContent = true;
        }

    };
}
