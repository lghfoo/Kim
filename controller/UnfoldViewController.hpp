#pragma once
#include"../view/UnfoldView.hpp"
#include"ItemController.hpp"
#include"ConnectionController.hpp"
#include <QLinkedList>
namespace Kim {
    class KUnfoldViewController : public QObject{
        Q_OBJECT
    signals:
        void RequestUnfoldConnectionSignal(const QLinkedList<KConnectionController*>&);
    private:
        KUnfoldView* UnfoldView = new KUnfoldView;
        QLinkedList<KConnectionController*>SelectedControlleres = {};
    public:
        void Show(){UnfoldView->show();}
//        static void ShowUnfold(KItemController* ItemController,
//                               const QMap<KItemController*, QLinkedList<KConnectionController*>>& ItemConnections){
//            KUnfoldViewController* UnfoldController = new KUnfoldViewController(ItemController, ItemConnections);
//            auto View = UnfoldController->UnfoldView;
//            View->show();
//        }

        KUnfoldViewController(KItemController* ItemController,
                              const QMap<KItemController*, QLinkedList<KConnectionController*>>& ItemConnections){
            UnfoldView->setAttribute(Qt::WA_DeleteOnClose);
            connect(UnfoldView,
                    &KUnfoldView::CloseSignal,
                    [=]{
                delete this;
            });
            auto Scene = UnfoldView->CanvasView->GetScene();
            Scene->ShowCursor(false);
            auto SrcItemView = ItemController->GetView()->Clone();
            SrcItemView->setEnabled(false);
            Scene->addItem(SrcItemView);
            const auto& Iter = ItemConnections.find(ItemController);
            QSet<KItemController*>CloseControlleres = {ItemController};
            if(Iter != ItemConnections.end()){
                const auto& Conns = Iter.value();
                for(auto Conn : Conns){
                    if(Conn->IsFolded() && Conn->GetSrcItemController() == ItemController){
                        auto NewConn = static_cast<KConnectionView*>
                                (Conn->GetConnectionView()->Clone());
                        // Select only
                        NewConn->SetSelectedOnly(true);
                        Scene->addItem(NewConn);
                        auto DstItem = Conn->GetDstItemController();
                        AddItemRecursively(DstItem,
                                           ItemConnections,
                                           CloseControlleres
                                           );
                        connect(NewConn,
                                &KConnectionView::SelectedChangedSignal,
                                [=](bool Selected){
                            if(Selected){
                                this->SelectedControlleres.append(Conn);
                            }
                            else{
                                this->SelectedControlleres.removeOne(Conn);
                            }
                        });
                    }
                }
            }

            connect(UnfoldView->OkButton,
                    &QPushButton::clicked,
                    [=]{
                emit RequestUnfoldConnectionSignal(SelectedControlleres);
                UnfoldView->close();
            });

            connect(UnfoldView->CancelButton,
                    &QPushButton::clicked,
                    UnfoldView,
                    &QDialog::close);
        }

        void AddItemRecursively(KItemController* ItemController,
                                const QMap<KItemController*, QLinkedList<KConnectionController*>>& ItemConnections,
                                QSet<KItemController*>& CloseControlleres){
            if(CloseControlleres.contains(ItemController))return;
            CloseControlleres.insert(ItemController);
            auto Scene = UnfoldView->CanvasView->GetScene();
            auto SrcItemView = static_cast<KItemView*>(ItemController->GetView()->Clone());
            SrcItemView->setEnabled(false);
            Scene->addItem(SrcItemView);
            const auto& Iter = ItemConnections.find(ItemController);
            if(Iter != ItemConnections.end()){
                const auto& Conns = Iter.value();
                for(auto Conn : Conns){
                    if(Conn->GetDstItemController() == ItemController)continue;
                    auto NewConn = static_cast<KConnectionView*>
                            (Conn->GetConnectionView()->Clone());
                    // Disable
                    NewConn->setEnabled(false);
                    Scene->addItem(NewConn);
                    auto DstItem = Conn->GetDstItemController();
                    AddItemRecursively(DstItem,
                                       ItemConnections,
                                       CloseControlleres
                                       );

                }
            }
        }

        ~KUnfoldViewController(){
        }
    };

}
