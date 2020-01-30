#pragma once
#include"ItemController.hpp"
#include"ConnectionController.hpp"

#include <QLinkedList>
namespace Kim {
    class KItemGroupController : public QObject{
        Q_OBJECT
    signals:
    public slots:
    private:
        qint64 GroupID = CreateID();
        QLinkedList<KItemController*>Items = {};
        QLinkedList<KConnectionController*>Connections = {};
        QLinkedList<KConnectionController*>InConnections = {};
        QLinkedList<KConnectionController*>OutConnections = {};
        QLinkedList<KConnectionController*>InOutConnections = {};
    public:
        void AddItem(KItemController* Item){
            Items.append(Item);
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

        void AddInOutConnection(KConnectionController* InOutConn){
            InOutConnections.append(InOutConn);
        }

        QSet<KItemController*> GetSrcItems(){
            QSet<KItemController*>Items{};
            for(auto Conn : InConnections){
                Items.insert(Conn->GetSrcItemController());
            }
            for(auto Conn : InOutConnections){
                Items.insert(Conn->GetSrcItemController());
            }
            return Items;
        }

        QSet<KItemController*> GetDstItems(){
            QSet<KItemController*>Items{};
            for(auto Conn : OutConnections){
                Items.insert(Conn->GetDstItemController());
            }
            for(auto Conn : InOutConnections){
                Items.insert(Conn->GetDstItemController());
            }
            return Items;
        }
    };
}
