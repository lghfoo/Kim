#pragma once
#include"../view/CanvasView.hpp"
#include"../view/ItemView.hpp"
#include"ItemController.hpp"
#include"ConnectionController.hpp"

#include <QLinkedList>
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
            bool WriteDirectly = false;
            KMoveTargetType MoveTargetType = MoveCursor;
            KAddPosType AddPosType = AtCursor;
            void ToNextMoveTargetType(){
                this->MoveTargetType = KMoveTargetType((this->MoveTargetType + 1) % MoveTargetCount);
            }
            void ToNextAddPosType(){
                this->AddPosType = KAddPosType((this->AddPosType + 1) % AddPosCount);
            }
            void ToggleWriteDirect(){
                SetWriteDirect(!WriteDirectly);
            }
            void SetWriteDirect(bool WriteDirect){
                KTextItemView::IsWriteDirect() = this->WriteDirectly = WriteDirect;
            }
            KCanvasState(){
                SetWriteDirect(false);
            }
        };
    private:
        KCanvasState CanvasState;
        KCanvasView* CanvasView = new KCanvasView;
        KScene* Scene = new KScene;
        KSceneContext SceneContext;
        QList<KItemController*>ItemControlleres;
        QList<KConnectionController*>ConnectionControlleres;
        QLinkedList<KGraphicsObjectController*>SelectedControlleres;
        QMap<KItemController*, QLinkedList<KConnectionController*>> ItemConnections;
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
                switch (Event->key()) {
                case Qt::Key_R:
                    CanvasState.ToggleWriteDirect();
                    break;
                }
            }
            else{
                switch (Event->key()) {
                case Qt::Key_Delete:
                    OnDeletePress();
                    break;
                case Qt::Key_Space:
                    OnSpacePress();
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
            qreal Dx = Grid.CellW / Grid.SubCellWCount * XCount;
            qreal Dy = Grid.CellH / Grid.SubCellHCount * YCount;
            if(CanvasState.MoveTargetType == CanvasState.MoveCursor){
                Scene->SetCursorPos(Scene->GetCursorPos() + QPointF(Dx, Dy));
            }
            else if(CanvasState.MoveTargetType == CanvasState.MoveItem){
                auto Iter = SelectedControlleres.begin();
                while(Iter != SelectedControlleres.end()){
                    if((*Iter)->type() != KConnectionView::Type){
                        auto ItemView = (*Iter)->GetGraphicsObject();
                        ItemView->moveBy(Dx, Dy);
                        break;
                    }
                }
            }
        }

        void OnStartConnecting(KItemController* ItemController){
            if(SceneContext.DragingConnectionController)return;
            KConnectionController* Controller = CreateAndAddConnectionController();
            Controller->SetSrcItemController(ItemController);
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
                auto TextItemController = CreateAndAddItemController(KTextItemView::Type);
                auto Pos = SceneContext.DragingConnectionController->GetConnectionView()->GetTo();
                TextItemController->GetView()->setPos(Pos);
                SceneContext.DragingConnectionController->SetDstItemController(TextItemController);
                SceneContext.DragingConnectionController = nullptr;
            }
        }
    public:
        KCanvasController(){
            CanvasView->setScene(Scene);
            const int SceneSize = 65536;
            Scene->setSceneRect(
                        -SceneSize / 2,
                        -SceneSize / 2,
                        SceneSize,
                        SceneSize
                        );
            QObject::connect(CanvasView, &KCanvasView::KeyPressSignal,
                             this, &KCanvasController::OnKeyPress);
            QObject::connect(Scene, &KScene::DragMoveSignal,
                             this, &KCanvasController::OnDragMove);

        }

        const KCanvasState& GetCanvasState(){
            return CanvasState;
        }

        KCanvasView* GetCanvasView(){
            return CanvasView;
        }

        void OnDeletePress(){
            DeleteSelectedControlleres();
        }

        void OnSpacePress(){
            AddTextItem();
        }

        void OnConnectChanged(KConnectionController* ConnectionController,
                              KItemController* OldController,
                              KItemController* NewController){
            if(OldController == NewController)return;
            if(NewController){
                QLinkedList<KConnectionController*>
                        *NewConnections = GetConnectionsOfItem(NewController);
                if(NewConnections){
                    NewConnections->append(ConnectionController);
                }
                else{
                    ItemConnections.insert(NewController,
                                           QLinkedList<KConnectionController*>({ConnectionController}));
                }

            }
            if(OldController){
                QLinkedList<KConnectionController*>
                        *OldConnections = GetConnectionsOfItem(OldController);
                if(OldConnections){
                    OldConnections->removeOne(ConnectionController);
                }
            }
        }

        QLinkedList<KConnectionController*>* GetConnectionsOfItem(KItemController* Controller){
            if(Controller){
                auto It = ItemConnections.find(Controller);
                if(It != ItemConnections.end()){
                    return &It.value();
                }
            }
            return nullptr;
        }

        void OnDragMove(QGraphicsSceneDragDropEvent* event){
            if(SceneContext.DragingConnectionController){
                SceneContext.DragingConnectionController->UpdateDstPosition(event->scenePos());
                Scene->update();
            }
        }

        void AddTextItem(){
            auto Controller = CreateItemController(KTextItemView::Type);
            AddItem(Controller);
            ItemControlleres.append(Controller);
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
            Scene->clearSelection();
            auto Item = ItemController->GetView();
            Item->setSelected(true);
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

        void OnItemSelectedChanged(KGraphicsObjectController* Controller,
                                   bool Selected){
            if(Selected){
                this->SelectedControlleres.prepend(Controller);
            }
            else{
                this->SelectedControlleres.removeOne(Controller);
            }
        }

        // Deserializing Methods
        KItemController* CreateItemController(int ItemType){
            KItemController* Controller = nullptr;
            switch (ItemType) {
            case KTextItemView::Type:
                Controller = new KTextItemController;
                break;
            default:
                qDebug()<<"warning: unkown item type.";
                return nullptr;
                break;
            }
            connect(Controller, &KItemController::StartConnectingSignal,
                    this, &KCanvasController::OnStartConnecting);
            connect(Controller, &KItemController::EndConnectingSignal,
                    this, &KCanvasController::OnEndConnecting);
            connect(Controller, &KItemController::PosChangedSignal,
                    this, [=]{Scene->update();});
            connect(Controller, &KItemController::IgnoreDropSignal,
                    this, &KCanvasController::OnItemIgnoreDrop);
            connect(Controller, &KGraphicsObjectController::SelectedChangedSignal,
                    this, &KCanvasController::OnItemSelectedChanged);
            return Controller;
        }

        void AddItemContrller(KItemController* ItemController){
            this->ItemControlleres.append(ItemController);
            this->Scene->addItem(ItemController->GetView());
        }

        KItemController* CreateAndAddItemController(int ItemType){
            KItemController* Controller = CreateItemController(ItemType);
            if(Controller){
                AddItemContrller(Controller);
            }
            return Controller;
        }

        KConnectionController* CreateConnectionController(){
            KConnectionController* Controller = new KConnectionController;
            connect(Controller, &KGraphicsObjectController::SelectedChangedSignal,
                    this, &KCanvasController::OnItemSelectedChanged);
            connect(Controller, &KConnectionController::ConnectChangedSignal,
                    this, &KCanvasController::OnConnectChanged);
            return Controller;
        }

        void AddConnectionController(KConnectionController* Controller){
            this->ConnectionControlleres.append(Controller);
            this->Scene->addItem(Controller->GetConnectionView());
        }

        KConnectionController* CreateAndAddConnectionController(){
            KConnectionController* Controller = CreateConnectionController();
            AddConnectionController(Controller);
            return Controller;
        }

        KItemController* GetItemByIdentity(const QString& Identity){
            for(auto Controller : ItemControlleres){
                if(Controller->GetIdentity() == Identity){
                    return Controller;
                }
            }
            return nullptr;
        }

        void DeleteSelectedControlleres(){
            while (!SelectedControlleres.isEmpty()) {
                KGraphicsObjectController* Controller = SelectedControlleres.front();
                if(Controller->type() == KConnectionView::Type){
                    DeleteConnection(static_cast<KConnectionController*>(Controller));
                }
                else{
                    DeleteItem(static_cast<KItemController*>(Controller));
                }
            }
        }

        void DeleteItem(KItemController* Controller){
            ItemControlleres.removeOne(static_cast<KItemController*>(Controller));
            if(Controller->IsSelected()){
                SelectedControlleres.removeOne(Controller);
            }
            // update item connection
            QLinkedList<KConnectionController*>*Connections = GetConnectionsOfItem(Controller);
            if(Connections){
                QLinkedList<KConnectionController*>& Conns = *Connections;
                for(auto Conn : Conns){
                    DeleteConnection(Conn, Controller);
                }
            }
            ItemConnections.remove(Controller);

            delete Controller;
        }

        void DeleteConnection(KConnectionController* Controller,
                              KItemController* Exclude = nullptr){
            ConnectionControlleres.removeOne(static_cast<KConnectionController*>(Controller));
            if(Controller->IsSelected()){
                SelectedControlleres.removeOne(Controller);
            }
            KItemController* SrcController = Controller->GetSrcItemController();
            KItemController* DstController = Controller->GetDstItemController();
            if(SrcController && SrcController != Exclude){
                QLinkedList<KConnectionController*>*Connections = GetConnectionsOfItem(SrcController);
                if(Connections)
                    Connections->removeOne(Controller);
            }
            if(DstController && DstController != Exclude){
                QLinkedList<KConnectionController*>*Connections = GetConnectionsOfItem(DstController);
                if(Connections)
                    Connections->removeOne(Controller);
            }
            delete Controller;
        }

        void RefreshState(){
            if(!this->ItemControlleres.isEmpty()){
                CanvasState.HasContent = true;
            }
        }
    };
}
