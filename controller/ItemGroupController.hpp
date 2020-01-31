#pragma once
#include"ItemController.hpp"
#include"ConnectionController.hpp"

#include <QLinkedList>
namespace Kim {
    class KItemGroupController : public QObject{
        Q_OBJECT
    signals:
    public slots:
        void OnInConnectionDestroyed(KConnectionController* Controller){
            qDebug()<<"in destroy";
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
//            Iter = InOutConnections.begin();
//            bool HasDeleteInOutSrc = false;
//            while(Iter != InOutConnections.end()){
//                auto Conn = *Iter;
//                if(Conn->GetSrcItemController() == Controller->GetSrcItemController()){
//                    delete Conn;
//                    Iter = InOutConnections.erase(Iter);
//                    HasDeleteInOutSrc = true;
//                }
//                else{
//                    ++Iter;
//                }
//            }
//            // 删除inout的src后要检查dst
//            if(HasDeleteInOutSrc){
//                UpdateInOutConnectionByDstItem();
//                CheckExistence();
//            }
        }
        void OnOutConnectionDestroyed(KConnectionController* Controller){
            qDebug()<<"out destroy";
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
//            Iter = InOutConnections.begin();
//            bool HasDeleteInOutDst = false;
//            while(Iter != InOutConnections.end()){
//                auto Conn = *Iter;
//                if(Conn->GetDstItemController() == Controller->GetDstItemController()){
//                    delete Conn;
//                    Iter = InOutConnections.erase(Iter);
//                    HasDeleteInOutDst = true;
//                }
//                else{
//                    ++Iter;
//                }
//            }
//            // 删除inout的dst后要检查src
//            if(HasDeleteInOutDst){
//                UpdateInOutConnectionBySrcItem();
//                CheckExistence();
//            }
        }
    private:
        qint64 GroupID = CreateID();
        KItemController* GroupItem = nullptr;
        QLinkedList<KItemController*>Items = {};
        QLinkedList<KConnectionController*>Connections = {};
        QLinkedList<KConnectionController*>InConnections = {};
        QLinkedList<KConnectionController*>OutConnections = {};
//        QLinkedList<KConnectionController*>InOutConnections = {};
    public:
//        void UpdateInOutConnection(){
//            const auto& SrcItems = this->GetSrcItems();
//            const auto& DstItems = this->GetDstItems();
//            if(GroupItem){
//                auto InConns = GroupItem->GetInConnections();
//                auto OutConns = GroupItem->GetOutConnections();
//                for(auto InConn : InConns){
//                    if(!SrcItems.contains(InConn->GetSrcItemController())){
//                        delete InConn;
//                    }
//                }
//                for(auto OutConn : OutConns){
//                    if(!DstItems.contains(OutConn->GetDstItemController())){
//                        delete OutConn;
//                    }
//                }
//            }
//        }
//        void UpdateInOutConnectionBySrcItem(){
//            qDebug()<<"update by src";
//            const auto& SrcItems = this->GetSrcItems();
//            qDebug()<<"before size: "<<GroupItem->GetConnections().size();
//            if(GroupItem){
//                auto InConns = GroupItem->GetInConnections();
//                qDebug()<<SrcItems.empty();
//                for(auto InConn : InConns){
//                    if(SrcItems.empty() || !SrcItems.contains(InConn->GetSrcItemController())){
////                        InConn->disconnect();
//                        delete InConn;
//                    }
//                }
//            }
//            qDebug()<<"after size: "<<GroupItem->GetConnections().size();
//        }
//        void UpdateInOutConnectionByDstItem(){
//            qDebug()<<"update by dst";
//            const auto& DstItems = this->GetDstItems();
//            qDebug()<<"before size: "<<GroupItem->GetConnections().size();
//            if(GroupItem){
//                auto OutConns = GroupItem->GetOutConnections();
//                qDebug()<<DstItems.empty();
//                for(auto OutConn : OutConns){
//                    if(DstItems.empty() || !DstItems.contains(OutConn->GetDstItemController())){
////                        OutConn->disconnect();
//                        delete OutConn;
//                    }
//                }
//            }
//            qDebug()<<"after size: "<<GroupItem->GetConnections().size();
//        }
//        void CheckExistence(){
//            qDebug()<<"check exist";
//            if(this->GroupItem && this->Items.isEmpty()
//                    && this->Connections.isEmpty()
//                    && this->InConnections.isEmpty()
//                    && this->OutConnections.isEmpty()
//                    /*&& this->InOutConnections.isEmpty()*/){
//                qDebug()<<"delete";
////                GroupItem->disconnect();
//                qDebug()<<GroupItem->GetConnections().size();
//                delete GroupItem;
//                qDebug()<<"after delete group item";
//                delete this;
//                qDebug()<<"after delete this";
//            }
//        }
        void AddItem(KItemController* Item){
            Items.append(Item);
        }

        void AddConnection(KConnectionController* Conn){
            Connections.append(Conn);
        }

        void AddInConnection(KConnectionController* InConn){
            InConnections.append(InConn);
//            connect(InConn, &KConnectionController::DestroyedSignal,
//                    [=](KConnectionController* Controller){
//                this->InConnections.removeOne(Controller);
//            });
//            connect(InConn->GetSrcItemController(), &KItemController::DestroyedSignal,
//                    InConn, &KConnectionController::Destroy);
        }

        void AddOutConnection(KConnectionController* OutConn){
            OutConnections.append(OutConn);
//            connect(OutConn, &KConnectionController::DestroyedSignal,
//                    [=](KConnectionController* Controller){
//                this->OutConnections.removeOne(Controller);
//            });
//            connect(OutConn->GetDstItemController(), &KItemController::DestroyedSignal,
//                    OutConn, &KConnectionController::Destroy);
        }

//        void AddInOutConnection(KConnectionController* InOutConn){
//            InOutConnections.append(InOutConn);
////            connect(InOutConn, &KConnectionController::DestroyedSignal,
////                    [=](KConnectionController* Controller){
////                this->InOutConnections.removeOne(Controller);
////            });
////            connect(InOutConn->GetSrcItemController(), &KItemController::DestroyedSignal,
////                    InOutConn, &KConnectionController::Destroy);
////            connect(InOutConn->GetDstItemController(), &KItemController::DestroyedSignal,
////                    InOutConn, &KConnectionController::Destroy);
//        }

//        QSet<KItemController*> GetInSrcItems(){

//        }

//        QSet<KItemController*> GetOutDstItems(){

//        }

//        QSet<KItemController*> GetInOutSrcItems(){

//        }

//        QSet<KItemController*> GetInOutDstItems(){

//        }

        QSet<KItemController*> GetSrcItems(){
            QSet<KItemController*>Items{};
            for(auto Conn : InConnections){
                Items.insert(Conn->GetSrcItemController());
            }
//            for(auto Conn : InOutConnections){
//                Items.insert(Conn->GetSrcItemController());
//            }
            return Items;
        }

        QSet<KItemController*> GetDstItems(){
            QSet<KItemController*>Items{};
            for(auto Conn : OutConnections){
                Items.insert(Conn->GetDstItemController());
            }
//            for(auto Conn : InOutConnections){
//                Items.insert(Conn->GetDstItemController());
//            }
            return Items;
        }

        QLinkedList<KConnectionController*>GetAllConnections(){
            return InConnections + OutConnections /*+ InOutConnections*/ + Connections;
        }

        void SetGroupItem(KItemController* GroupItem){
            this->GroupItem = GroupItem;
        }

//        void DirectDelete(){
//            if(this->GroupItem){
//                this->GroupItem->disconnect();
//                const auto& Conns = GroupItem->GetConnections();
//                for(auto Conn : Conns){
//                    Conn->disconnect();
//                    delete Conn;
//                }
//            }
//            const auto& AllConns = this->GetAllConnections();
//            for(auto Conn : AllConns){
//                Conn->disconnect();
//                delete Conn;
//            }
//            const auto& AllItems = this->Items;
//            for(auto Item : AllItems){
//                delete Item;
//            }
//            delete this;
//        }
    };
}
