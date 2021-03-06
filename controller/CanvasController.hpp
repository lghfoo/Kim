﻿#pragma once
#include"../view/CanvasView.hpp"
#include"../view/ItemView.hpp"
#include"ItemController.hpp"
#include"ItemGroupController.hpp"
#include"ConnectionController.hpp"
#include"UnfoldViewController.hpp"
#include <QApplication>
#include <QClipboard>
#include <QLinkedList>
#include <QMessageBox>
namespace Kim {
    class KTextSerializer;
    class KDBSerializer;
    class KCanvasController : public QObject{
        Q_OBJECT
        friend class KTextSerializer;
        friend class KDBSerializer;
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
        qint64 CanvasID = CreateID();
        QString CanvasName = "";
        KCanvasState CanvasState;
        KSceneContext SceneContext;
        KCanvasView* CanvasView = new KCanvasView;
        KScene* Scene = CanvasView->GetScene();
        QLinkedList<KItemController*>ItemControlleres = {};
        QLinkedList<KConnectionController*>ConnectionControlleres = {};
        QLinkedList<KGraphicsObjectController*>SelectedControlleres = {};
        QMap<qint64, KItemGroupController*> GroupControlleres = {};
    signals:
        void SaveSingal();
        void SaveAsSignal();
        void LoadSignal();
        void FocusedObjectChangedSignal(KGraphicsObjectController* Item);
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
                    AddItem(KTextItemView::Type);
                    break;
                case Qt::Key_2:
                    AddItem(KImageItemView::Type);
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
                    // can not connect self
                    if(ItemController == SceneContext.DragingConnectionController->GetSrcItemController()){
                        this->CancelConnecting();
                        return;
                    }
                    SceneContext.DragingConnectionController->SetDstItemController(ItemController);
                    SceneContext.DragingConnectionController = nullptr;
                }
            }
        }

        void OnItemRequestSelectAllChildren(KItemController* ItemController, SelectionType Type){
            SelectAllChildren(ItemController, Type);
        }

        void OnConnectionRequestSelectAllChildren(KConnectionController* ConnController, SelectionType Type){
            SelectAllChildren(ConnController, Type);
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
                auto Controller = this->AddItem(KImageItemView::Type, &ScenePos);
                Controller->GetItemView<KImageItemView>()->SetImage(Data);
            }
            else if(KTextItemView::HasTextData(Data)){
                auto ScenePos = Event->scenePos();
                this->AddItem(KTextItemView::Type, &ScenePos)->GetItemView<KTextItemView>()->SetText(Data);
            }
            else if(SceneContext.DragingConnectionController){
                auto TextItemController = CreateAndAddItemController(KTextItemView::Type);
                auto Pos = SceneContext.DragingConnectionController->GetConnectionView()->GetTo();
                TextItemController->GetView()->setPos(Pos);
                SceneContext.DragingConnectionController->SetDstItemController(TextItemController);
                auto SrcItem = SceneContext.DragingConnectionController->GetSrcItemController();
                auto ItemController = TextItemController;
                if(SrcItem && SrcItem->type() == ItemController->type()){
                    if(SrcItem->type() == KTextItemView::Type){
                        ItemController->GetItemView<KTextItemView>()->SetStyle(SrcItem->GetItemView<KTextItemView>()->GetStyle());
                    }
                }
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

        void OnConnectionDelete(KConnectionController* Controller){
            ConnectionControlleres.removeOne(Controller);
            if(Controller->IsSelected()){
                SelectedControlleres.removeOne(Controller);
            }
        }

        // 在所有选中的连接中插入一个item
        void OnInsertItem(int ItemType){
            QList<KConnectionController*>SelectedConns{};
            KItemController* SrcItem = nullptr;
            QPointF NewItemPos(0, 0);
            for(auto Controller : SelectedControlleres){
                if(Controller->type() == KConnectionView::Type){
                    SelectedConns.append(static_cast<KConnectionController*>(Controller));
                    auto Conn = SelectedConns.back();
                    auto SrcPos = Conn->GetSrcItemController()->GetPos();
                    auto DstPos = Conn->GetDstItemController()->GetPos();
                    NewItemPos += (SrcPos + DstPos) / 2.0;
                }
            }
            if(SelectedConns.isEmpty())return;
            NewItemPos /= SelectedConns.size();
            KItemController* NewItem = CreateItemController(ItemType);
            NewItem->SetPos(NewItemPos);
            AddItemContrller(NewItem);
            QMultiMap <KItemController*, KItemController*> LeftMap{}, RightMap{};
            while(!SelectedConns.isEmpty()){
                auto Conn = SelectedConns.back();
                SelectedConns.pop_back();
                if(!LeftMap.contains(Conn->GetSrcItemController(), NewItem)){
                    auto NewConnLeft = CreateConnectionController();
                    NewConnLeft->SetSrcItemController(Conn->GetSrcItemController());
                    NewConnLeft->SetDstItemController(NewItem);
                    AddConnectionController(NewConnLeft);
                    LeftMap.insert(Conn->GetSrcItemController(), NewItem);
                }
                if(!RightMap.contains(NewItem, Conn->GetDstItemController())){
                    auto NewConnRight = CreateConnectionController();
                    NewConnRight->SetSrcItemController(NewItem);
                    NewConnRight->SetDstItemController(Conn->GetDstItemController());
                    RightMap.insert(NewItem, Conn->GetDstItemController());
                    AddConnectionController(NewConnRight);
                }
                // TODO:利用旧边
                DeleteConnection(Conn);
            }
        }


        // when delete item, always delete connection first.
        // except for group item
        void OnItemDelete(KItemController* Controller){
            ItemControlleres.removeOne(Controller);
            if(Controller->IsSelected()){
                SelectedControlleres.removeOne(Controller);
            }
            // update item connection
            const auto& Connections = Controller->GetConnections();
            for(auto Conn : Connections){
                DeleteConnection(Conn);
            }
        }

        void OnItemGroupToCanvas(KItemController* Controller){

        }

        void OnItemUngroup(KItemController* Controller){
            auto ItemGroup = Controller->GetItemGroupController();
            if(ItemGroup){
                // process group
                const auto& PosWhenGrouping = ItemGroup->GetPosWhenGrouping();
                const auto& CurrentPos = Controller->GetView()->pos();
                const auto& DPos = CurrentPos - PosWhenGrouping;
                const auto& Items = ItemGroup->GetItems();
                const auto& Connections = ItemGroup->GetConnections();
                const auto& InConnections = ItemGroup->GetInConnections();
                const auto& OutConnections = ItemGroup->GetOutConnections();
                for(auto Item : Items){
                    if(!Item->IsCollapsed()){
                        Item->GetView()->moveBy(DPos.x(), DPos.y());
                    }
                    auto Pos = Item->GetView()->pos();
                    this->AddItem(Item, &Pos);
                }
                for(auto Conn : Connections){
                    this->AddConnectionController(Conn);
                }
                for(auto InConn : InConnections){
                    InConn->ConnectSrcItem();
                    this->AddConnectionController(InConn);
                }
                for(auto OutConn : OutConnections){
                    OutConn->ConnectDstItem();
                    this->AddConnectionController(OutConn);
                }
                ItemGroup->Clear();

                // process group item
                Controller->SetIsGroupping(false);
                while(!Controller->GetConnections().isEmpty()){
                    DeleteConnection(Controller->GetConnections().back());
                }
                auto PanItem = ItemGroup->GetPanItem();
                PanItem->moveBy(DPos.x(), DPos.y());
                AddItemAt(PanItem, PanItem->pos());
                const auto& PanBounding = PanItem->mapToScene(PanItem->boundingRect()).boundingRect();
                const auto& ItemBounding = Controller->GetView()->boundingRect();
                qreal PosX = PanBounding.center().x();
                qreal VGap = 5.0;
                qreal PosY = PanBounding.top() - VGap - ItemBounding.height() / 2.0;
                Controller->SetPos({PosX, PosY});
            }
        }

        void OnItemGroup(KItemController* Controller){
            auto ItemGroup = Controller->GetItemGroupController();
            if(ItemGroup && !Controller->IsGroupping()){
                Controller->SetIsGroupping(true);
                auto PanItem = ItemGroup->GetPanItem();
                const auto& PanBounding = PanItem->mapToScene(PanItem->boundingRect()).boundingRect();
                const auto& ItemsInPan = Scene->items(PanBounding);
                QSet<KItemController*>TargetItems{};
                bool Collapsed = true;
                for(auto Item : ItemsInPan){
                    switch (Item->type()) {
                    case KTextItemView::Type:
                    case KImageItemView::Type:
                    {
                        auto ItemView = static_cast<KItemView*>(Item);
                        auto ItemController = static_cast<KItemController*>(ItemView->GetController());
                        TargetItems.insert(ItemController);
                        GetAllChildren(ItemController, TargetItems, &Collapsed);
                        break;
                    }
                    default:
                        break;
                    }

                }

                QSet<KConnectionController*>Visited{};
                for(auto ItemController : TargetItems){
                    if(ItemController == Controller)break;
                    this->RemoveItem(ItemController);
                    ItemGroup->AddItem(ItemController);
                    const auto& Conns = ItemController->GetConnections();
                    for(auto Conn : Conns){
                        if(Visited.contains(Conn))continue;
                        Visited.insert(Conn);
                        this->RemoveConnection(Conn);
                        auto SrcItem = Conn->GetSrcItemController();
                        auto DstItem = Conn->GetDstItemController();
                        bool SrcContained = TargetItems.contains(SrcItem);
                        bool DstContained = TargetItems.contains(DstItem);
                        if(SrcContained && DstContained){
                            ItemGroup->AddConnection(Conn);
                        }
                        else if(SrcContained){
                            // !DstSelected
                            ItemGroup->AddOutConnection(Conn);
                            // 切断外界与group内部的连接
                            Conn->DisconnectDstItem();
                        }
                        else if(DstContained){
                            // !SrcSelected
                            ItemGroup->AddInConnection(Conn);
                            // 切断外界与group内部的连接
                            Conn->DisconnectSrcItem();
                        }
                        else{
                            // will not happen
                        }
                    }

                }
                const auto& SrcItems = ItemGroup->GetSrcItems();
                for(auto SrcItem : SrcItems){
                    auto Conn = CreateAndAddConnectionController();
                    Conn->SetSrcItemController(SrcItem);
                    Conn->SetDstItemController(Controller);
                    connect(Conn, &KConnectionController::DestroyedSignal,
                            ItemGroup, &KItemGroupController::OnInConnectionDestroyed);
                }
                const auto& DstItems = ItemGroup->GetDstItems();
                for(auto DstItem : DstItems){
                    auto Conn = CreateAndAddConnectionController();
                    Conn->SetSrcItemController(Controller);
                    Conn->SetDstItemController(DstItem);
                    connect(Conn, &KConnectionController::DestroyedSignal,
                            ItemGroup, &KItemGroupController::OnOutConnectionDestroyed);
                }
                ItemGroup->SetPosWhenGroupping(Controller->GetView()->pos());
                this->Scene->removeItem(PanItem);
            }
        }
        // group外的item和connection与group内的所有item和connection有引用
        // group内的connection与group外的item有引用
        void OnGroupToItem(int ItemType){
            if(this->SelectedControlleres.isEmpty())return;
            QPointF GroupPos(0, 0);
            int Cnt = 0;
            QSet<KConnectionController*>Visited{};
            KItemGroupController* ItemGroup = new KItemGroupController;
            auto Selectedes = this->SelectedControlleres;
            QSet<KItemController*>CollapsedChildren{};
            bool Collapsed = true;
            for(auto Controller : Selectedes){
                // 只处理item，忽略connection
                // item的所有connection都要被group
                if(Controller->type() != KConnectionView::Type){
                    auto ItemController = static_cast<KItemController*>(Controller);
                    CollapsedChildren.clear();
                    CollapsedChildren.insert(ItemController);
                    GetAllChildren(ItemController, CollapsedChildren, &Collapsed);
                    for(auto Item : CollapsedChildren){
                        this->RemoveItem(Item);
                        ItemGroup->AddItem(Item);
                        ++Cnt;
                        const auto& Conns = Item->GetConnections();
                        for(auto Conn : Conns){
                            if(Visited.contains(Conn))continue;
                            Visited.insert(Conn);
                            this->RemoveConnection(Conn);
                            auto SrcItem = Conn->GetSrcItemController();
                            auto DstItem = Conn->GetDstItemController();
                            bool SrcSelected = SrcItem->IsSelected() || CollapsedChildren.contains(SrcItem);
                            bool DstSelected = DstItem->IsSelected() || CollapsedChildren.contains(DstItem);
                            if(SrcSelected && DstSelected){
                                ItemGroup->AddConnection(Conn);
                            }
                            else if(SrcSelected){
                                // !DstSelected
                                ItemGroup->AddOutConnection(Conn);
                                // 切断外界与group内部的连接
                                Conn->DisconnectDstItem();
    //                            DstItem->RemoveInConnection(Conn);
                            }
                            else if(DstSelected){
                                // !SrcSelected
                                ItemGroup->AddInConnection(Conn);
                                // 切断外界与group内部的连接
                                Conn->DisconnectSrcItem();
    //                            SrcItem->RemoveOutConnection(Conn);
                            }
                            else{
                                // will not happen
                            }
                            ++Cnt;
                            GroupPos += Controller->GetGraphicsObject()->pos() + Conn->GetConnectionView()->boundingRect().center();
                        }
                    }
                }
            }
            if(Cnt == 0){
                delete ItemGroup;
                return;
            }
            GroupPos /= Cnt;
            auto GroupItem = CreateItemController(ItemType);
            GroupItem->SetItemGroupController(ItemGroup);
            ItemGroup->SetGroupItem(GroupItem);
            GroupControlleres.insert(GroupItem->GetIdentity(), ItemGroup);
            connect(GroupItem, &KItemController::DestroyedSignal, ItemGroup, &KItemGroupController::Destory);
            GroupItem->SetIsGroupping(true);
///             signal is disconnect when remove, so clear selected manually
///             although AddItem will clear selection
//            this->SelectedControlleres.clear();
            this->AddItem(GroupItem, &GroupPos);
            ItemGroup->SetPosWhenGroupping(GroupPos);
            const auto& SrcItems = ItemGroup->GetSrcItems();
            for(auto SrcItem : SrcItems){
                auto Conn = CreateAndAddConnectionController();
                Conn->SetSrcItemController(SrcItem);
                Conn->SetDstItemController(GroupItem);
                connect(Conn, &KConnectionController::DestroyedSignal,
                        ItemGroup, &KItemGroupController::OnInConnectionDestroyed);
            }
            const auto& DstItems = ItemGroup->GetDstItems();
            for(auto DstItem : DstItems){
                auto Conn = CreateAndAddConnectionController();
                Conn->SetSrcItemController(GroupItem);
                Conn->SetDstItemController(DstItem);
                connect(Conn, &KConnectionController::DestroyedSignal,
                        ItemGroup, &KItemGroupController::OnOutConnectionDestroyed);
            }
            ItemGroup->UpdatePanBounding();
        }
    public:
        KCanvasController(){
            QObject::connect(CanvasView, &KCanvasView::KeyPressSignal,
                             this, &KCanvasController::OnKeyPress);
            QObject::connect(Scene, &KScene::DragMoveSignal,
                             this, &KCanvasController::OnSceneDragMove);
            QObject::connect(Scene, &KScene::DropSignal,
                             this, &KCanvasController::OnSceneDrop);
            QObject::connect(Scene, &QGraphicsScene::focusItemChanged, [=](QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason){
                auto NewFocus = dynamic_cast<KGraphicsViewBase*>(newFocusItem);
                if(NewFocus){
                    emit FocusedObjectChangedSignal(NewFocus->GetController());
                }
            });

        }

        ~KCanvasController(){
            Clear();
        }

        void Clear(){
            while(!ItemControlleres.isEmpty()){
                auto Item = ItemControlleres.front();
                ItemControlleres.pop_front();
                delete Item;
            }
            ItemControlleres.clear();
            SelectedControlleres.clear();
            GroupControlleres.clear();
            ConnectionControlleres.clear();
        }

        qint64 GetCanvasID()const{
            return this->CanvasID;
        }

        void SetCanvasID(const qint64 ID){
            this->CanvasID = ID;
        }

        const KCanvasState& GetCanvasState(){
            return CanvasState;
        }

        KCanvasView* GetCanvasView(){
            return CanvasView;
        }

        QString GetCanvasName(){
            return CanvasName;
        }

        void SetCanvasName(const QString& Name){
            this->CanvasName = Name;
        }

        // Children should be empty
        void GetAllChildren(KItemController* ItemController, QSet<KItemController*>& Children, bool* Collapsed = nullptr){
            const auto& Conns = ItemController->GetOutConnections();
            for(auto Conn : Conns){
                GetAllChildren(Conn, Children, Collapsed);
            }
        }

        // Children should be empty
        void GetAllChildren(KConnectionController* ConnController, QSet<KItemController*>& Children, bool* Collapsed = nullptr){
            auto DstItem = ConnController->GetDstItemController();
            if(!Children.contains(DstItem)){
                if(Collapsed && (*Collapsed)!=DstItem->IsCollapsed()){
                    return;
                }
                Children.insert(DstItem);
                GetAllChildren(DstItem, Children);
            }
        }

        void SelectAllChildren(KItemController* ItemController, SelectionType Type){
            QSet<KItemController*> Children = {};
            GetAllChildren(ItemController, Children);
            for(auto Child : Children){
                if(Child->IsCollapsed())continue;
                if(Type == SelectionType::All)
                    Child->GetView()->setSelected(true);
                else if(Type == SelectionType::None)
                    Child->GetView()->setSelected(false);
                else if(Type == SelectionType::Reverse)
                    Child->GetView()->setSelected(!Child->GetView()->isSelected());
            }
            // select item will change focus
            ItemController->RequestFocus();
        }

        void SelectAllChildren(KConnectionController* ConnController, SelectionType Type){
            QSet<KItemController*> Children = {};
            GetAllChildren(ConnController, Children);
            for(auto Child : Children){
                if(Child->IsCollapsed())continue;
                if(Type == SelectionType::All)
                    Child->GetView()->setSelected(true);
                else if(Type == SelectionType::None)
                    Child->GetView()->setSelected(false);
                else if(Type == SelectionType::Reverse)
                    Child->GetView()->setSelected(!Child->GetView()->isSelected());
            }
            ConnController->RequestFocus();
        }

        bool HasConnectionOutsideChildren(KItemController* Controller){
            QSet<KItemController*>Children{};
            GetAllChildren(Controller, Children);
            for(auto Child : Children){
                const auto& Conns = Child->GetInConnections();
                for(auto Conn : Conns){
                    auto SrcItem = Conn->GetSrcItemController();
                    // the the src item is not the controller and
                    // if there is a connection between
                    // item outside the children and the child
                    // then, the item is not collapseable
                    if(SrcItem != Controller && !Children.contains(SrcItem)){
                        return true;
                    }
                }
            }
            return false;
        }

        bool HasCircuitInChildrenHelper(KItemController* Controller, QSet<KItemController*>& Visited){
            if(Visited.contains(Controller))return  true;
            Visited.insert(Controller);
            const auto& Conns = Controller->GetOutConnections();
            for(auto Conn : Conns){
                if(HasCircuitInChildrenHelper(Conn->GetDstItemController(), Visited))
                    return true;
            }
            return false;
        }

        // 孩子中是否有环，孩子不包括最原始的item
        bool HasCircuitInChildren(KItemController* Controller){
            QSet<KItemController*>Visited{};
            return HasCircuitInChildrenHelper(Controller, Visited);
        }

        bool IsTreeRoot(KItemController* Contrller){
            return true;
        }

        // 只有是树状才能折叠
        bool IsCollapseable(KItemController* Controller){
//            return !HasCircuitInChildren(Controller) && !HasConnectionOutsideChildren(Controller);
            // 1、不能有环路
            if(HasCircuitInChildren(Controller))return false;
            // 2、item的入度不能大于1
            QSet<KItemController*>Children{};
            GetAllChildren(Controller, Children);
            for(auto Item : Children){
                // 只要有一个item的入度大于1就不是树
                if(Item->GetInConnections().size() > 1){
                    return false;
                }
            }
            // 3、孩子不能与原始item有连接
            for(auto Item : Children){
                const auto& OutConns = Item->GetOutConnections();
                for(auto OutConn : OutConns){
                    if(OutConn->GetDstItemController() == Controller){
                        return false;
                    }
                }
            }
            return true;
        }


        void OnRequestPaste(){
            QClipboard* Clipboard = QApplication::clipboard();
            auto Data = Clipboard->mimeData();
            if(KImageItemView::HasImageData(Data)){
                AddItem(KImageItemView::Type)->GetItemView<KImageItemView>()->SetImage(Data);
            }
            else if(KTextItemView::HasTextData(Data)){
                AddItem(KTextItemView::Type)->GetItemView<KTextItemView>()->SetText(Data);
            }
        }

        void OnDeletePress(){
            DeleteSelectedControlleres();
            // delete connection controler point
            const auto& Items = Scene->selectedItems();
            for(auto Item : Items){
                if(Item->type() == KControlPoint::Type){
                    auto CtrlP = static_cast<KControlPoint*>(Item);
                    emit CtrlP->DeletedSignal(CtrlP);
                }
            }
        }

        void OnSpacePress(){
            AddItem(KTextItemView::Type);
        }

        void CancelConnecting(){
            if(SceneContext.DragingConnectionController){
                DeleteConnection(SceneContext.DragingConnectionController);
                SceneContext.DragingConnectionController = nullptr;
            }
        }

        //////////////////////////////// Collapse & Expand ////////////////////////////////
        //////////////////////////////// Expand ////////////////////////////////
        void ExpandItemHelper(KItemController* Controller,
                              bool Recursively = false){
            const auto& Conns = Controller->GetOutConnections();
            for(auto Conn : Conns){
                auto DstItem = Conn->GetDstItemController();
                if(DstItem->IsCollapsed()){
                    Conn->SetCollapsed(false);
                    DstItem->SetCollapsed(false);
                    ConnectItemController(DstItem);
                    ConnectConnectionController(Conn);
                    auto NewParentPos = Controller->GetView()->pos();
                    auto OldParentPos = DstItem->GetParentPosWhenCollapse();
                    auto DPos = NewParentPos - OldParentPos;
                    DstItem->GetView()->moveBy(DPos.x(), DPos.y());
                    Controller->SetFoldConnectionCount(Controller->GetFoldConnectionCount() - 1);
                    if(Recursively){
                        ExpandItemHelper(Controller);
                    }
                }
            }
        }

        void OnItemRequestExpand(KItemController* Controller){
            ExpandItemHelper(Controller);
        }
        //////////////////////////////// Collapse ////////////////////////////////
        void CollapseItemHelper(KItemController* Controller,
                                KItemController* Root){
            const auto& OutConns = Controller->GetOutConnections();
            if(OutConns.isEmpty())return;
            auto Iter = OutConns.begin();
            while(Iter != OutConns.end()){
                auto Conn = *Iter;
                if(!Conn->IsCollapsed()){
                    if(Conn->IsSelected())
                        Conn->SetSelected(false);
                    Conn->SetCollapsed(true);
                    DisconnectConnectionController(Conn);
                    Controller->SetFoldConnectionCount(Controller->GetFoldConnectionCount() + 1);
                    auto DstItem = Conn->GetDstItemController();
                    if(!DstItem->IsCollapsed()){
                        if(DstItem->IsSelected())
                            DstItem->SetSelected(false);
                        DstItem->SetParentPosWhenCollapse(Controller->GetView()->pos());
                        DstItem->SetCollapsed(true);
                        DisconnectItemConntroller(DstItem);
                        CollapseItemHelper(DstItem, Root);
                    }
                }
                ++Iter;
            }
        }

        void OnItemRequestCollapse(KItemController* Controller){
            QSet<KItemController*>Children{};
            if(!IsCollapseable(Controller)){
                QMessageBox Box;
                Box.setText("Fail to collapse: need the form of tree.");
                Box.exec();
                return;
            }
            CollapseItemHelper(Controller, Controller);
        }
        //////////////////////////////// Collapse & Expand ////////////////////////////////


        KItemController* AddItem(int ItemType, const QPointF* const Position = nullptr){
            auto Controller = CreateItemController(ItemType);
            AddItem(Controller, Position);
            return Controller;
        }

        // 默认在鼠标或光标处添加
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
            ItemControlleres.append(ItemController);
            ConnectItemController(ItemController);
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

        void ConnectItemController(KItemController* Controller){
            connect(Controller, &KItemController::StartConnectingSignal,
                    this, &KCanvasController::OnStartConnecting);
            connect(Controller, &KItemController::EndConnectingSignal,
                    this, &KCanvasController::OnEndConnecting);
            connect(Controller, &KItemController::IgnoreDropSignal,
                    this, &KCanvasController::OnItemIgnoreDrop);
            connect(Controller, &KGraphicsObjectController::SelectedChangedSignal,
                    this, &KCanvasController::OnItemSelectedChanged);
            connect(Controller, &KItemController::RequestSelectAllChildrenSignal,
                    this, &KCanvasController::OnItemRequestSelectAllChildren);
            connect(Controller, &KItemController::RequestExpandSignal,
                    this, &KCanvasController::OnItemRequestExpand);
            connect(Controller, &KItemController::RequestCollapseSignal,
                    this, &KCanvasController::OnItemRequestCollapse);
            connect(Controller, &KItemController::DestroyedSignal,
                    this, &KCanvasController::OnItemDelete);
            connect(Controller, &KItemController::UngroupSignal,
                    this, &KCanvasController::OnItemUngroup);
            connect(Controller, &KItemController::GroupSignal,
                    this, &KCanvasController::OnItemGroup);
            connect(Controller, &KItemController::GroupToNewCanvasSignal,
                    this, &KCanvasController::OnItemGroupToCanvas);

        }

        void DisconnectItemConntroller(KItemController* Controller){
            disconnect(Controller, &KItemController::StartConnectingSignal,
                    this, &KCanvasController::OnStartConnecting);
            disconnect(Controller, &KItemController::EndConnectingSignal,
                    this, &KCanvasController::OnEndConnecting);
            disconnect(Controller, &KItemController::IgnoreDropSignal,
                    this, &KCanvasController::OnItemIgnoreDrop);
            disconnect(Controller, &KGraphicsObjectController::SelectedChangedSignal,
                    this, &KCanvasController::OnItemSelectedChanged);
            disconnect(Controller, &KItemController::RequestSelectAllChildrenSignal,
                    this, &KCanvasController::OnItemRequestSelectAllChildren);
            disconnect(Controller, &KItemController::RequestExpandSignal,
                    this, &KCanvasController::OnItemRequestExpand);
            disconnect(Controller, &KItemController::RequestCollapseSignal,
                    this, &KCanvasController::OnItemRequestCollapse);
            disconnect(Controller, &KItemController::DestroyedSignal,
                    this, &KCanvasController::OnItemDelete);
            disconnect(Controller, &KItemController::UngroupSignal,
                    this, &KCanvasController::OnItemUngroup);
            disconnect(Controller, &KItemController::GroupToNewCanvasSignal,
                    this, &KCanvasController::OnItemGroupToCanvas);
        }

        void ConnectConnectionController(KConnectionController* Controller){
            connect(Controller, &KGraphicsObjectController::SelectedChangedSignal,
                    this, &KCanvasController::OnItemSelectedChanged);
            connect(Controller, &KConnectionController::SelectedAllChildrenSignal,
                    this, &KCanvasController::OnConnectionRequestSelectAllChildren);
            connect(Controller, &KConnectionController::DestroyedSignal,
                    this, &KCanvasController::OnConnectionDelete);
        }

        void DisconnectConnectionController(KConnectionController* Controller){
            disconnect(Controller, &KGraphicsObjectController::SelectedChangedSignal,
                    this, &KCanvasController::OnItemSelectedChanged);
            disconnect(Controller, &KConnectionController::SelectedAllChildrenSignal,
                    this, &KCanvasController::OnConnectionRequestSelectAllChildren);
            disconnect(Controller, &KConnectionController::DestroyedSignal,
                    this, &KCanvasController::OnConnectionDelete);
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
            return Controller;
        }

        void AddItemContrller(KItemController* ItemController){
            this->ItemControlleres.append(ItemController);
            this->Scene->addItem(ItemController->GetView());
            ConnectItemController(ItemController);
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
//            ConnectConnectionController(Controller);
            return Controller;
        }

        void AddConnectionController(KConnectionController* Controller){
            this->ConnectionControlleres.append(Controller);
            this->Scene->addItem(Controller->GetConnectionView());
            ConnectConnectionController(Controller);
        }

        KConnectionController* CreateAndAddConnectionController(){
            KConnectionController* Controller = CreateConnectionController();
            AddConnectionController(Controller);
            return Controller;
        }

        KItemController* GetItemByIdentity(const qint64 Identity){
            for(auto Controller : ItemControlleres){
                if(Controller->GetIdentity() == Identity){
                    return Controller;
                }
            }
            return nullptr;
        }

        QMap<qint64, KItemController*> CreateItemMap(){
            QMap<qint64, KItemController*> Ret {};
            for(auto Controller : ItemControlleres){
                Ret.insert(Controller->GetIdentity(), Controller);
            }
            return Ret;
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

        // remove item, keep connections of item, keep selected
        void RemoveItem(KItemController* Controller){
            SelectedControlleres.removeOne(Controller);
            DisconnectItemConntroller(Controller);
            Scene->removeItem(Controller->GetView());
            ItemControlleres.removeOne(Controller);
        }

        // remove connection, keep connections of item, keep selected
        void RemoveConnection(KConnectionController* Controller){
            SelectedControlleres.removeOne(Controller);
            DisconnectConnectionController(Controller);
            Scene->removeItem(Controller->GetConnectionView());
            ConnectionControlleres.removeOne(Controller);
        }

        void DeleteItem(KItemController* Controller){
            delete Controller;
        }

        void DeleteConnection(KConnectionController* Controller){
            delete Controller;
        }

        void RefreshState(){
            if(!this->ItemControlleres.isEmpty()){
                CanvasState.HasContent = true;
            }
        }
    };
}
