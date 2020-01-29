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
    class KDBSerializer;
    // TODO:
    // Modify signal
    class KItemController : public KGraphicsObjectController{
        Q_OBJECT
        friend class KTextSerializer;
        friend class KDBSerializer;
    private:
        KFoldMark* ExpandMark = nullptr;
        KFoldMark* CollapseMark = nullptr;
    protected:
        QDateTime CreatedTime;
        QDateTime LastModifiedTime;
        QString Identity = "";
        QString Alias = "";
        int FoldConnectionCount = 0;
        int OutConnectionCount = 0;
        bool Collapsed = false;
        QPointF ParentPosWhenCollapse = {};
    signals:
        void StartConnectingSignal(KItemController* Controller);
        void EndConnectingSignal(KItemController* Controller);
        void IgnoreDropSignal(KItemController* Controller);
        void PosChangedSignal(const QPointF& NewPow);
        void SizeChangedSignal(KItemController* Controller);
        void RequestExpandSignal(KItemController* Controller);
        void RequestCollapseSignal(KItemController* Controller);
        void RequestSelectAllChildrenSignal(KItemController* Controller, SelectionType);
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
        void EmitRquestExpandSignal(){
            emit RequestExpandSignal(this);
        }
        void EmitRequestCollapseSignal(){
            emit RequestCollapseSignal(this);
        }
        void EmitRequestSelectAllChildrenSignal(SelectionType Type){
            emit RequestSelectAllChildrenSignal(this, Type);
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
            connect(ItemView,
                    &KItemView::SelectedAllChildrenSignal,
                    this,
                    &KItemController::EmitRequestSelectAllChildrenSignal);
        }
        virtual ~KItemController(){
            GraphicsObject->scene()->removeItem(GraphicsObject);
            delete GraphicsObject;
        }

        bool IsCollapsed() {
            return Collapsed;
        }

        void SetCollapsed(bool Collapsed){
            this->Collapsed = Collapsed;
        }

        void SetParentPosWhenCollapse(const QPointF& Pos){
            this->ParentPosWhenCollapse = Pos;
        }

        QPointF GetParentPosWhenCollapse(){
            return this->ParentPosWhenCollapse;
        }

        template<typename T>
        T* GetItemView(){
            return static_cast<T*>(GraphicsObject);
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
        int GetFoldConnectionCount()const{return FoldConnectionCount;}
        int GetOutConnectionCount()const{return OutConnectionCount;}
        void SetFoldConnectionCount(int FoldConnectionCount){
            this->FoldConnectionCount = FoldConnectionCount;
            UpdateMark();
//            int OldCount = this->FoldConnectionCount, NewCount = FoldConnectionCount;
//            Q_ASSERT(OldCount >= 0 && NewCount >= 0);
//            this->FoldConnectionCount = FoldConnectionCount;
//            // add expand mark
//            if(OldCount == 0 && NewCount > 0){
//                Q_ASSERT(!ExpandMark);
//                ExpandMark = new KFoldMark;
//                connect(ExpandMark,
//                        &KFoldMark::ClickedSignal,
//                        this,
//                        &KItemController::EmitRquestUnfoldSignal);
//                ExpandMark->setParentItem(GraphicsObject);
//                // update FoldMark position
//                const auto& ItemBouding = GraphicsObject->boundingRect();
//                const auto& MarkBouding = ExpandMark->boundingRect();
//                const qreal Gap = 5.0;
//                const qreal PosY = ItemBouding.center().y();
//                const qreal PosX = ItemBouding.left() - Gap - MarkBouding.width() / 2.0;
//                ExpandMark->setPos(PosX, PosY);
//            }
//            // remove expand mark
//            else if(OldCount > 0 && NewCount == 0){
//                Q_ASSERT(ExpandMark);
//                delete ExpandMark;
//                ExpandMark = nullptr;
//            }
        }
        void SetOutConnectionCount(int OutConnectionCount){
            this->OutConnectionCount = OutConnectionCount;
            UpdateMark();
//            int OldCount = this->OutConnectionCount, NewCount = OutConnectionCount;
//            Q_ASSERT(OldCount >= 0 && NewCount >= 0);
//            this->OutConnectionCount = NewCount;
//            // add collapse mark

//            // remove collapse mark
        }

        void UpdateMark(){
            // add expand mark
            if(this->FoldConnectionCount > 0 && !ExpandMark){
                ExpandMark = new KFoldMark(KFoldMark::Plus);
                ExpandMark->setParentItem(GraphicsObject);
                connect(ExpandMark,
                        &KFoldMark::ClickedSignal,
                        this,
                        &KItemController::EmitRquestExpandSignal);
            }
            // remove expand mark
            else if(this->FoldConnectionCount == 0 && ExpandMark){
                delete ExpandMark;
                ExpandMark = nullptr;
            }

            int VisibleOutConnection = OutConnectionCount - FoldConnectionCount;
            // add collapse mark
            if(VisibleOutConnection > 0 && !CollapseMark){
                CollapseMark = new KFoldMark(KFoldMark::Minus);
                CollapseMark->setParentItem(GraphicsObject);
                connect(CollapseMark,
                        &KFoldMark::ClickedSignal,
                        this,
                        &KItemController::EmitRequestCollapseSignal);
            }
            // remove collapse mark
            else if(VisibleOutConnection == 0 && CollapseMark){
                delete CollapseMark;
                CollapseMark = nullptr;
            }

            // update layout
            if(ExpandMark && CollapseMark){
                const auto& ItemBouding = GraphicsObject->boundingRect();
                const auto& ExpandMarkBouding = ExpandMark->boundingRect();
                const auto& CollapseMarkBouding = CollapseMark->boundingRect();
                qreal MaxW = qMax(ExpandMarkBouding.width(), CollapseMarkBouding.width());
                qreal MaxH = qMax(ExpandMarkBouding.height(), CollapseMarkBouding.height());
                const qreal HGap = 5.0;
                const qreal VGap = 5.0;
                const qreal PosYUp = ItemBouding.center().y() - VGap - MaxH / 2.0;
                const qreal PosYDown = ItemBouding.center().y() + VGap + MaxH / 2.0;
                const qreal PosX = ItemBouding.left() - HGap - MaxW / 2.0;
                ExpandMark->setPos(PosX, PosYUp);
                CollapseMark->setPos(PosX, PosYDown);
            }
            else{
                KFoldMark* TargetMark = nullptr;
                if(ExpandMark){
                    TargetMark = ExpandMark;
                }
                else if(CollapseMark){
                    TargetMark = CollapseMark;
                }
                if(TargetMark){
                    const auto& ItemBouding = GraphicsObject->boundingRect();
                    const auto& MarkBouding = TargetMark->boundingRect();
                    const qreal HGap = 5.0;
                    const qreal PosY = ItemBouding.center().y();
                    const qreal PosX = ItemBouding.left() - HGap - MarkBouding.width() / 2.0;
                    TargetMark->setPos(PosX, PosY);
                }
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

        void AppendText(const QString& Text){
            static_cast<KTextItemView*>(GraphicsObject)->AppendText(Text);
        }
    };
    //////////////////////////////// Image Item ////////////////////////////////
    class KImageItemController : public KItemController{
    public:
        KImageItemController():KItemController(new KImageItemView){

        }
    };
}
