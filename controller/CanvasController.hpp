#pragma once
#include"../view/CanvasView.hpp"
#include"../view/ItemView.hpp"
#include"ItemController.hpp"
#include"ConnectionController.hpp"
#include"UnfoldViewController.hpp"
#include <QApplication>
#include <QClipboard>
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
        KScene* Scene = CanvasView->GetScene();
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
                case Qt::Key_V:
                    OnRequestPaste();
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
            // Key
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
                // 1: 添加文本
                // 2: 添加图片
                case Qt::Key_1:
                    AddTextItem();
                    break;
                case Qt::Key_2:
                    AddImageItem();
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
                CancelConnecting();
            }
        }

        void OnSceneDragMove(QGraphicsSceneDragDropEvent* event){
            if(SceneContext.DragingConnectionController){
                SceneContext.DragingConnectionController->UpdateDstPosition(event->scenePos());
                Scene->update();
            }
        }

        void OnSceneDrop(QGraphicsSceneDragDropEvent* Event){
            auto Data = Event->mimeData();
            // process image drop on scene
            if(KImageItemView::HasImageData(Data)){
                auto ScenePos = Event->scenePos();
                auto Controller = this->AddImageItem(&ScenePos);
                Controller->GetItemView<KImageItemView>()->SetImage(Data);
            }
            else if(KTextItemView::HasTextData(Data)){
                auto ScenePos = Event->scenePos();
                this->AddTextItem(&ScenePos)->GetItemView<KTextItemView>()->SetText(Data);
            }
            else if(SceneContext.DragingConnectionController){
                auto TextItemController = CreateAndAddItemController(KTextItemView::Type);
                auto Pos = SceneContext.DragingConnectionController->GetConnectionView()->GetTo();
                TextItemController->GetView()->setPos(Pos);
                SceneContext.DragingConnectionController->SetDstItemController(TextItemController);
                SceneContext.DragingConnectionController = nullptr;
            }
        }

        void OnSpecialInput(const QString& Text){
            for(auto Item : this->SelectedControlleres){
                if(Item->type() == KTextItemView::Type){
                    static_cast<KTextItemController*>(Item)->AppendText(Text);
                    break;
                }
            }
        }
    public:
        KCanvasController(){
            QObject::connect(CanvasView, &KCanvasView::KeyPressSignal,
                             this, &KCanvasController::OnKeyPress);
            QObject::connect(Scene, &KScene::DragMoveSignal,
                             this, &KCanvasController::OnSceneDragMove);
            QObject::connect(Scene, &KScene::DropSignal,
                             this, &KCanvasController::OnSceneDrop);

        }

        const KCanvasState& GetCanvasState(){
            return CanvasState;
        }

        KCanvasView* GetCanvasView(){
            return CanvasView;
        }

        void OnRequestPaste(){
            QClipboard* Clipboard = QApplication::clipboard();
            auto Data = Clipboard->mimeData();
            if(KImageItemView::HasImageData(Data)){
                AddImageItem()->GetItemView<KImageItemView>()->SetImage(Data);
            }
            else if(KTextItemView::HasTextData(Data)){
                AddTextItem()->GetItemView<KTextItemView>()->SetText(Data);
            }
        }

        void OnDeletePress(){
            DeleteSelectedControlleres();
        }

        void OnSpacePress(){
            AddTextItem();
        }

        void CancelConnecting(){
            if(SceneContext.DragingConnectionController){
                DeleteConnection(SceneContext.DragingConnectionController);
                SceneContext.DragingConnectionController = nullptr;
            }
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

        void FoldConnection(KConnectionController* ConnController){
            bool IsFolded = ConnController->IsFolded();
            // 更新折叠计数
            ConnController->SetFoldCount(ConnController->GetFoldCount() + 1);
            // 避免对已折叠的Connection进行处理
            if(IsFolded)return;
            auto ConnView = ConnController->GetConnectionView();
            this->Scene->removeItem(ConnView);

            auto DstItemController = ConnController->GetDstItemController();
            auto DstView = DstItemController->GetView();
            // 取消选中状态，防止意外删除
            if(DstView->isSelected()){
                DstView->setSelected(false);
            }
            this->Scene->removeItem(DstView);

            auto SrcItemController = ConnController->GetSrcItemController();
            auto SrcView = SrcItemController->GetView();
            auto FoldCnt = SrcItemController->GetFoldConnectionCount();
            SrcItemController->SetFoldConnectionCount(FoldCnt+1);

            QLinkedList<KConnectionController*>* DstConns = this->GetConnectionsOfItem(DstItemController);
            if(DstConns){
                auto Iter = DstConns->begin();
                while (Iter!=DstConns->end()) {
                    // avoid dead loop
                    if(*Iter != ConnController){
                        FoldConnection(*Iter);
                    }
                    Iter++;
                }
            }
        }

        void UnfoldConnection(KConnectionController* ConnController){
            ConnController->SetFoldCount(ConnController->GetFoldCount() - 1);
            if(ConnController->GetFoldCount() > 0)return;
            this->Scene->addItem(ConnController->GetConnectionView());

            auto SrcItem = ConnController->GetSrcItemController();
            SrcItem->SetFoldConnectionCount(SrcItem->GetFoldConnectionCount() - 1);

            auto DstItem = ConnController->GetDstItemController();
            this->Scene->addItem(DstItem->GetView());

            QLinkedList<KConnectionController*>* DstConns = this->GetConnectionsOfItem(DstItem);
            if(DstConns){
                auto Iter = DstConns->begin();
                while(Iter != DstConns->end()){
                    if(*Iter != ConnController){
                        UnfoldConnection(*Iter);
                    }
                    Iter++;
                }
            }
        }

        void OnConnectionFold(KConnectionController* ConnController){
            FoldConnection(ConnController);
        }

        void OnItemRequestUnfold(KItemController* ItemController){
            KUnfoldViewController* UnfoldController = new KUnfoldViewController(ItemController, ItemConnections);
            connect(UnfoldController,
                    &KUnfoldViewController::RequestUnfoldConnectionSignal,
                    this,
                    &KCanvasController::OnRequestUnfoldConnections);
            UnfoldController->Show();
        }

        void OnRequestUnfoldConnections(const QLinkedList<KConnectionController*>&
                                        Connections){
            for(auto Connection : Connections){
                UnfoldConnection(Connection);
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

        KTextItemController* AddTextItem(const QPointF* const Position = nullptr){
            auto Controller = CreateItemController(KTextItemView::Type);
            AddItem(Controller, Position);
            ItemControlleres.append(Controller);
            return static_cast<KTextItemController*>(Controller);
        }

        KImageItemController* AddImageItem(const QPointF* const Position = nullptr){
            auto Controller = CreateItemController(KImageItemView::Type);
            AddItem(Controller, Position);
            ItemControlleres.append(Controller);
            return static_cast<KImageItemController*>(Controller);
        }

        void AddItem(KItemController* ItemController,
                     const QPointF* const Position = nullptr){
            QPointF Pos(0, 0);
            if(Position){
                Pos = *Position;
            }
            else{
                switch (CanvasState.AddPosType) {
                case KCanvasState::AtMouse:{
                    Pos = CanvasView->mapToScene(CanvasView->mapFromGlobal(CanvasView->cursor().pos()));
                    break;
                }
                case KCanvasState::AtCursor:
                    Pos = Scene->GetCursorPos();
                    break;
                case KCanvasState::AtNearest:
                    break;
                default:
                    break;
                }
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
            case KImageItemView::Type:
                Controller = new KImageItemController;
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
            connect(Controller, &KItemController::RequestUnfoldSignal,
                    this, &KCanvasController::OnItemRequestUnfold);

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
            connect(Controller, &KConnectionController::FoldSignal,
                    this, &KCanvasController::OnConnectionFold);
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
