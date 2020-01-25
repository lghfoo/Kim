    #pragma once
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include<QTime>
#include <QVBoxLayout>
#include"GraphicsObjectController.hpp"
#include"../view/ItemView.hpp"
namespace Kim {
    static const QString IdentityTimeFormat = "yyyy_MM_dd_hh_mm_ss_zzz";
    static const QString NormalTimeFormat = "yyyy/MM/dd hh:mm:ss";
    class KTextSerializer;
    class KItemController : public KGraphicsObjectController{
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
        void SizeChangedSignal(KItemController* Controller);
    public slots:
        void EmitStartConnectingSignal(){
            emit StartConnectingSignal(this);
        }
        void EmitEndConnectingSignal(){
            emit EndConnectingSignal(this);
        }
        void EmitPosChangeSignal(){
            emit PosChangedSignal(GraphicsObject->pos());
        }
        void EmitIgnoreDropSignal(){
            emit IgnoreDropSignal(this);
        }
        void EmitSizeChangedSignal(){
            emit SizeChangedSignal(this);
        }
    public:
        KItemController(KItemView* ItemView):KGraphicsObjectController(ItemView){
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
            connect(ItemView,
                    &KItemView::SizeChangedSignal,
                    this,
                    &KItemController::EmitPosChangeSignal);
        }
        virtual ~KItemController(){
            GraphicsObject->scene()->removeItem(GraphicsObject);
            delete GraphicsObject;
        }
        KItemView* GetView() {
            return static_cast<KItemView*>(GraphicsObject);
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
        Q_OBJECT
        friend class KTextSerializer;
    public slots:
        void OnEdit(){
            KEditTextDialog Dialog(dynamic_cast<KTextItemView*>(GraphicsObject)->GetText());
            connect(&Dialog,
                    &KEditTextDialog::OKSignal,
                    [&](){
                dynamic_cast<KTextItemView*>(GraphicsObject)->SetText(Dialog.GetText());
                Dialog.close();
            });
            Dialog.exec();
            return;
        }
    public:
        KTextItemController():KItemController(new KTextItemView){
            connect(dynamic_cast<KTextItemView*>(GraphicsObject),
                    &KTextItemView::EditSignal,
                    this,
                    &KTextItemController::OnEdit);
        }
    };
}
