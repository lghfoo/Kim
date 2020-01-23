#pragma once
#include<QTime>
#include"../view/ItemView.hpp"
namespace Kim {
    static const QString IdentityTimeFormat = "yyyy_MM_dd_hh_mm_ss_zzz";
    static const QString NormalTimeFormat = "yyyy/MM/dd hh:mm:ss";
    class KTextSerializer;
    class KItemController : public QObject{
        Q_OBJECT
        friend class KTextSerializer;
    protected:
        QDateTime CreatedTime;
        QDateTime LastModifiedTime;
        QString Identity = "";
        QString Alias = "";
    signals:
        void StartConnectingSignal(KItemController* Controller);
        void EndConnectingSignal(KItemController* Controller);
        void IgnoreDropSignal(KItemController* Controller);
        void PosChangedSignal(const QPointF& NewPow);
    public slots:
        void EmitStartConnectingSignal(){
            emit StartConnectingSignal(this);
        }
        void EmitEndConnectingSignal(){
            emit EndConnectingSignal(this);
        }
        void EmitPosChangeSignal(){
            emit PosChangedSignal(ItemView->GetCenterPos());
        }
        void EmitIgnoreDropSignal(){
            emit IgnoreDropSignal(this);
        }
    protected:
        KItemView* ItemView = nullptr;
    public:
        KItemController(KItemView* ItemView):ItemView(ItemView){
            this->SetCreatedTime(QDateTime::currentDateTime(), true);
            Identity = "Item_" + CreatedTime.toString(IdentityTimeFormat);
            connect(ItemView,
                    &KItemView::StartDragDropSignal,
                    this,
                    &KItemController::EmitStartConnectingSignal);
            connect(ItemView,
                    &KItemView::EndDragDropSignal,
                    this,
                    &KItemController::EmitEndConnectingSignal);
            connect(ItemView,
                    &KItemView::PosChangedSignal,
                    this,
                    &KItemController::EmitPosChangeSignal);
            connect(ItemView,
                    &KItemView::IgnoreDropSignal,
                    this,
                    &KItemController::EmitIgnoreDropSignal);
        }
        KItemView* GetView() {
            return ItemView;
        }
        void OnModified(){
            LastModifiedTime = QDateTime::currentDateTime();
        }
        void SetIdentity(const QString& Identity){
            this->Identity = Identity;
        }
        QString GetIdentity()const{
            return Identity;
        }
        void SetAlias(const QString& Alias){
            this->Alias = Alias;
        }
        QString GetAlias(){
            return Alias;
        }
        void SetCreatedTime(const QDateTime& Time, bool UpdateLastModified = false){
            CreatedTime = Time;
            if(UpdateLastModified){
                LastModifiedTime = CreatedTime;
            }
        }
    };

    class KTextItemController : public KItemController{
        friend class KTextSerializer;
    public:
        KTextItemController():KItemController(new KTextItemView){
        }
    };
}
