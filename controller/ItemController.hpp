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
    // TODO:
    // Modify signal
    class KItemController : public KGraphicsObjectController{
        Q_OBJECT
        friend class KTextSerializer;
    private:
        KFoldMark* FoldMark = nullptr;
    protected:
        QDateTime CreatedTime;
        QDateTime LastModifiedTime;
        QString Identity = "";
        QString Alias = "";
        int FoldConnectionCount = 0;
    signals:
        void StartConnectingSignal(KItemController* Controller);
        void EndConnectingSignal(KItemController* Controller);
        void IgnoreDropSignal(KItemController* Controller);
        void PosChangedSignal(const QPointF& NewPow);
        void SizeChangedSignal(KItemController* Controller);
        void RequestUnfoldSignal(KItemController* Controller);
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
        void EmitRquestUnfoldSignal(){
            emit RequestUnfoldSignal(this);
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
        int GetFoldConnectionCount(){return FoldConnectionCount;}
        void SetFoldConnectionCount(int FoldConnectionCount){
            int OldCount = this->FoldConnectionCount, NewCount = FoldConnectionCount;
            Q_ASSERT(OldCount >= 0 && NewCount >= 0);
            this->FoldConnectionCount = FoldConnectionCount;
            // add fold mark
            if(OldCount == 0 && NewCount > 0){
                Q_ASSERT(!FoldMark);
                FoldMark = new KFoldMark;
                connect(FoldMark,
                        &KFoldMark::ClickedSignal,
                        this,
                        &KItemController::EmitRquestUnfoldSignal);
                FoldMark->setParentItem(GraphicsObject);
                // update FoldMark position
                const auto& ItemBouding = GraphicsObject->boundingRect();
                const auto& MarkBouding = FoldMark->boundingRect();
                const qreal Gap = 5.0;
                const qreal PosY = ItemBouding.center().y();
                const qreal PosX = ItemBouding.left() - Gap - MarkBouding.width() / 2.0;
                FoldMark->setPos(PosX, PosY);
            }
            // remove fold mark
            else if(OldCount > 0 && NewCount == 0){
                Q_ASSERT(FoldMark);
                delete FoldMark;
                FoldMark = nullptr;
            }
        }
    };
    //////////////////////////////// Text Item ////////////////////////////////
    class KTextItemController : public KItemController{
        Q_OBJECT
        friend class KTextSerializer;
    private:
        KEditTextDialog* Dialog = nullptr;
    public slots:
        void OnEdit(){
            auto ItemView = static_cast<KTextItemView*>(GraphicsObject);
            if(!Dialog){
                Dialog = new KEditTextDialog(ItemView->GetText());
            }
            else{
                Dialog->SetText(ItemView->GetText());
            }
            connect(Dialog,
                    &KEditTextDialog::OKSignal,
                    [=](){
                ItemView->SetText(Dialog->GetText());
                Dialog->close();
            });
            Dialog->show();
            return;
        }
    public:
        KTextItemController():KItemController(new KTextItemView){
            connect(dynamic_cast<KTextItemView*>(GraphicsObject),
                    &KTextItemView::EditSignal,
                    this,
                    &KTextItemController::OnEdit);
        }
        virtual ~KTextItemController(){
            if(Dialog){
                delete Dialog;
                Dialog = nullptr;
            }
        }
    };
    //////////////////////////////// Image Item ////////////////////////////////
    class KImageItemController : public KItemController{
    public:
        KImageItemController():KItemController(new KImageItemView){

        }
    };
}
