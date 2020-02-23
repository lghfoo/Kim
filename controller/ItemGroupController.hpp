#pragma once
#include"ItemController.hpp"
#include"ConnectionController.hpp"
#include <QLinkedList>
namespace Kim {
    class KItemGroupController : public QObject{
        Q_OBJECT
    signals:
    public slots:
        void Destory(){
            const auto& Connections = GetAllConnections();
            for(auto Conn : Connections){
                delete Conn;
            }
            for(auto Item : Items){
                delete Item;
            }
            delete this;
        }
        void OnInConnectionDestroyed(KConnectionController* Controller){
            auto Iter = InConnections.begin();
            while(Iter != InConnections.end()){
                auto Conn = *Iter;
                if(Conn->GetSrcItemController() == Controller->GetSrcItemController()){
                    delete Conn;
                    Iter = InConnections.erase(Iter);
                }
                else{
                    ++Iter;
                }
            }
        }
        void OnOutConnectionDestroyed(KConnectionController* Controller){
            auto Iter = OutConnections.begin();
            while(Iter != OutConnections.end()){
                auto Conn = *Iter;
                if(Conn->GetDstItemController() == Controller->GetDstItemController()){
                    delete Conn;
                    Iter = OutConnections.erase(Iter);
                }
                else{
                    ++Iter;
                }
            }
        }
    private:
        KGroupPanItemView* PanItem = new KGroupPanItemView;
        QPointF PosWhenGrouping = {};
        KItemController* GroupItem = nullptr;
        QLinkedList<KItemController*>Items = {};
        QLinkedList<KConnectionController*>Connections = {};
        QLinkedList<KConnectionController*>InConnections = {};
        QLinkedList<KConnectionController*>OutConnections = {};
    public:
        void SetPosWhenGroupping(const QPointF& Pos){
            this->PosWhenGrouping = Pos;
        }

        QPointF GetPosWhenGrouping(){
            return PosWhenGrouping;
        }

        void AddItem(KItemController* Item){
            Items.append(Item);
        }

        void UpdatePanBounding(){
            PanItem->SetBounding({});
            for(auto Item : Items){
                auto ItemBounding = PanItem->mapFromItem(Item->GetView(), Item->GetView()->boundingRect()).boundingRect();
                if(Item == Items.front()){
                    PanItem->SetBounding(ItemBounding);
                }
                else{
                    auto Rect = PanItem->GetBounding();
                    ExtendRect(Rect, ItemBounding);
                    PanItem->SetBounding(Rect);
                }
            }
        }

        void AddConnection(KConnectionController* Conn){
            Connections.append(Conn);
        }

        void AddInConnection(KConnectionController* InConn){
            InConnections.append(InConn);
        }

        void AddOutConnection(KConnectionController* OutConn){
            OutConnections.append(OutConn);
        }

        const QLinkedList<KItemController*>& GetItems(){
            return this->Items;
        }

        const QLinkedList<KConnectionController*>& GetInConnections(){
            return this->InConnections;
        }

        const QLinkedList<KConnectionController*>& GetOutConnections(){
            return this->OutConnections;
        }

        const QLinkedList<KConnectionController*>& GetConnections(){
            return this->Connections;
        }

        QSet<KItemController*> GetSrcItems(){
            QSet<KItemController*>Items{};
            for(auto Conn : InConnections){
                Items.insert(Conn->GetSrcItemController());
            }
            return Items;
        }

        QSet<KItemController*> GetDstItems(){
            QSet<KItemController*>Items{};
            for(auto Conn : OutConnections){
                Items.insert(Conn->GetDstItemController());
            }
            return Items;
        }

        QLinkedList<KConnectionController*>GetAllConnections(){
            return InConnections + OutConnections + Connections;
        }

        void SetGroupItem(KItemController* GroupItem){
            this->GroupItem = GroupItem;
        }

        KItemController* GetGroupItem(){
            return this->GroupItem;
        }

        KGroupPanItemView* GetPanItem(){
            return PanItem;
        }
        // use when ungroup
        void Clear(){
            this->Items.clear();
            this->Connections.clear();
            this->InConnections.clear();
            this->OutConnections.clear();
        }

        qint64 GetGroupID(){
            if(GroupItem){
                return GroupItem->GetIdentity();
            }
            return 0;
        }
    };
}
