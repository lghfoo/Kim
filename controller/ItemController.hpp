﻿#pragma once
#include <QDialog>
#include <QLinkedList>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>
#include"GraphicsObjectController.hpp"
#include"../view/ItemView.hpp"
namespace Kim {
    class KTextSerializer;
    class KDBSerializer;
    class KItemGroupController;
    class KConnectionController;
    // TODO:
    // Modify signal
    class KItemController : public KGraphicsObjectController{
        Q_OBJECT
        friend class KTextSerializer;
        friend class KDBSerializer;
        friend class KConnectionController;
    private:
        KItemMark* ExpandMark = nullptr;
        KItemMark* CollapseMark = nullptr;
        KItemMark* UngroupMark = nullptr;
        KItemMark* GroupMark = nullptr;
        KItemMark* GroupToNewCanvasMark = nullptr;
        KItemGroupController* ItemGroupController = nullptr;
        bool Groupping = false;
        QLinkedList<KConnectionController*>OutConnections = {};
        QLinkedList<KConnectionController*>InConnections = {};
    protected:
        // need serialize
        QDateTime CreatedTime;
        QDateTime LastModifiedTime;
        qint64 Identity = CreateID();
        QString Alias = "";
        int FoldConnectionCount = 0;
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
        void UngroupSignal(KItemController* Controller);
        void GroupSignal(KItemController* Controller);
        void GroupToNewCanvasSignal(KItemController* Controller);
        void DestroyedSignal(KItemController* Controller);
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
        void EmitUngroupSignal(){
            emit UngroupSignal(this);
        }
        void EmitGroupSignal(){
            emit GroupSignal(this);
        }
        void EmitGroupToNewCanvasSignal(){
            emit GroupToNewCanvasSignal(this);
        }
        void UpdateMark(){
            qreal VGap = 2.0;
            //////////////////////////////// Expand & Collapse ////////////////////////////////
            {
                // add expand mark
                if(this->FoldConnectionCount > 0 && !ExpandMark){
                    ExpandMark = new KItemMark(KItemMark::Plus);
                    ExpandMark->setParentItem(GraphicsObject);
                    connect(ExpandMark,
                            &KItemMark::ClickedSignal,
                            this,
                            &KItemController::EmitRquestExpandSignal);
                }
                // remove expand mark
                else if(this->FoldConnectionCount == 0 && ExpandMark){
                    delete ExpandMark;
                    ExpandMark = nullptr;
                }

                int VisibleOutConnection = OutConnections.size() - FoldConnectionCount;
                // add collapse mark
                if(VisibleOutConnection > 0 && !CollapseMark){
                    CollapseMark = new KItemMark(KItemMark::Minus);
                    CollapseMark->setParentItem(GraphicsObject);
                    connect(CollapseMark,
                            &KItemMark::ClickedSignal,
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
                    const qreal PosYUp = ItemBouding.center().y() - VGap - MaxH / 2.0;
                    const qreal PosYDown = ItemBouding.center().y() + VGap + MaxH / 2.0;
                    const qreal PosX = ItemBouding.left() - HGap - MaxW / 2.0;
                    ExpandMark->setPos(PosX, PosYUp);
                    CollapseMark->setPos(PosX, PosYDown);
                }
                else{
                    KItemMark* TargetMark = nullptr;
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

            //////////////////////////////// Group & Ungroup ////////////////////////////////
            {
                // add group mark
                if(this->ItemGroupController && this->IsGroupping() && !UngroupMark){
                    UngroupMark = new KItemMark(KItemMark::Plus);
                    GroupToNewCanvasMark = new KItemMark(KItemMark::Ellipse);
//                    UngroupMark->SetRadius(16);
//                    GroupToNewCanvasMark->SetRadius(16);
                    UngroupMark->setParentItem(GraphicsObject);
                    GroupToNewCanvasMark->setParentItem(GraphicsObject);
                    connect(UngroupMark,
                            &KItemMark::ClickedSignal,
                            this,
                            &KItemController::EmitUngroupSignal);
                    connect(GroupToNewCanvasMark,
                            &KItemMark::ClickedSignal,
                            this,
                            &KItemController::EmitGroupToNewCanvasSignal);
                }
                // remove group mark
                else if((!this->ItemGroupController || !this->IsGroupping()) && UngroupMark){
                    delete UngroupMark;
//                    delete GroupToNewCanvasMark;
                    UngroupMark = nullptr;
//                    GroupToNewCanvasMark = nullptr;
                    if(this->ItemGroupController && !this->IsGroupping() && !GroupMark){
                        GroupMark = new KItemMark(KItemMark::Minus);
//                        GroupMark->SetRadius(16);
                        GroupMark->setParentItem(GraphicsObject);
                        connect(GroupMark,
                                &KItemMark::ClickedSignal,
                                this,
                                &KItemController::EmitGroupSignal);
                    }
                }

                // update layout
                if((UngroupMark||GroupMark) && GroupToNewCanvasMark){
                    KItemMark* TargetMark = UngroupMark;
                    if(!TargetMark)TargetMark = GroupMark;
                    const auto& ItemBouding = GraphicsObject->boundingRect();
                    const auto& TargetMarkBouding = TargetMark->boundingRect();
                    const auto& GroupToNewCanvasMarkBouding = GroupToNewCanvasMark->boundingRect();
                    qreal MaxW = qMax(TargetMarkBouding.width(), GroupToNewCanvasMarkBouding.width());
                    qreal MaxH = qMax(TargetMarkBouding.height(), GroupToNewCanvasMarkBouding.height());
                    const qreal HGap = 5.0;
                    const qreal PosYUp = ItemBouding.center().y() - VGap - MaxH / 2.0;
                    const qreal PosYDown = ItemBouding.center().y() + VGap + MaxH / 2.0;
                    const qreal PosX = ItemBouding.right() + HGap + MaxW / 2.0;
                    TargetMark->setPos(PosX, PosYUp);
                    GroupToNewCanvasMark->setPos(PosX, PosYDown);

//                    const auto& ItemBouding = GraphicsObject->boundingRect();
//                    const auto& TargetMarkBouding = TargetMark->boundingRect();
//                    const auto& GroupToNewCanvasMarkBouding = GroupToNewCanvasMark->boundingRect();
//                    const qreal UngroupW = TargetMarkBouding.width();
//                    const qreal GroupToCanvasW = GroupToNewCanvasMarkBouding.width();
//                    const qreal HGap = 5.0;
//                    const qreal PosY = ItemBouding.center().y();
//                    const qreal UngroupPosX = ItemBouding.right() + HGap + UngroupW / 2.0;
//                    const qreal GroupToNewCanvasPosX = UngroupPosX  + UngroupW / 2.0 + HGap + GroupToCanvasW / 2.0;;
//                    TargetMark->setPos(UngroupPosX, PosY);
//                    GroupToNewCanvasMark->setPos(GroupToNewCanvasPosX, PosY);
                }
                else{
                    KItemMark* TargetMark = nullptr;
                    if(UngroupMark){
                        TargetMark = UngroupMark;
                    }
                    else if(GroupToNewCanvasMark){
                        TargetMark = GroupToNewCanvasMark;
                    }
                    else if(GroupMark){
                        TargetMark = GroupMark;
                    }
                    if(TargetMark){
                        const auto& ItemBouding = GraphicsObject->boundingRect();
                        const auto& MarkBouding = TargetMark->boundingRect();
                        const qreal HGap = 5.0;
                        const qreal PosY = ItemBouding.center().y();
                        const qreal PosX = ItemBouding.right() + HGap + MarkBouding.width() / 2.0;
                        TargetMark->setPos(PosX, PosY);
                    }
                }
            }
        }
    public:
        KItemController(KItemView* ItemView):KGraphicsObjectController(ItemView){
            this->SetCreatedTime(QDateTime::currentDateTime(), true);
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
            connect(ItemView,
                    &KItemView::SizeChangedSignal,
                    this,
                    &KItemController::UpdateMark);

        }
        virtual ~KItemController(){
            emit DestroyedSignal(this);
            delete GraphicsObject;
            GraphicsObject = nullptr;
            // CollapseMark & ExpandMark will be auto delete
            // because they are the child of grahics obj
            CollapseMark = nullptr;
            ExpandMark = nullptr;
            UngroupMark = nullptr;
            GroupToNewCanvasMark = nullptr;
        }

        bool IsCollapsed() {
            return Collapsed;
        }

        void SetCollapsed(bool Collapsed){
            this->Collapsed = Collapsed;
            this->GraphicsObject->setVisible(!Collapsed);
        }

        void SetParentPosWhenCollapse(const QPointF& Pos){
            this->ParentPosWhenCollapse = Pos;
        }

        QPointF GetParentPosWhenCollapse(){
            return this->ParentPosWhenCollapse;
        }

        void SetItemGroupController(KItemGroupController* Controller){
            this->ItemGroupController = Controller;
            UpdateMark();
        }

        void SetIsGroupping(bool InGroupping){
            this->Groupping = InGroupping;
            this->UpdateMark();
        }

        bool IsGroupping() const {
            return this->Groupping;
        }

        KItemGroupController* GetItemGroupController(){
            return this->ItemGroupController;
        }

        bool IsGroupItem(){
            return this->ItemGroupController != nullptr;
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
        void SetIdentity(qint64 Identity){
            this->Identity = Identity;
        }
        qint64 GetIdentity()const{
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
        void SetLastModifiedTime(const QDateTime& Time){
            LastModifiedTime = Time;
        }
        int GetFoldConnectionCount()const{return FoldConnectionCount;}
        void SetFoldConnectionCount(int FoldConnectionCount){
            this->FoldConnectionCount = FoldConnectionCount;
            UpdateMark();
        }

        const QLinkedList<KConnectionController*>& GetOutConnections(){
            return OutConnections;
        }

        const QLinkedList<KConnectionController*>& GetInConnections(){
            return InConnections;
        }

        QLinkedList<KConnectionController*> GetConnections(){
            return GetOutConnections() + GetInConnections();
        }

        void SetPos(const QPointF& Pos){
            this->GraphicsObject->setPos(Pos);
        }

        QPointF GetPos(){
            return this->GraphicsObject->pos();
        }

        // only use for group & ungroup
        void RemoveOutConnection(KConnectionController* Controller){
            OutConnections.removeOne(Controller);
        }

        void RemoveInConnection(KConnectionController* Controller){
            InConnections.removeOne(Controller);
        }

        void AddOutConnection(KConnectionController* Controller){
            OutConnections.append(Controller);
        }

        void AddInConnection(KConnectionController* Controller){
            InConnections.append(Controller);
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
            Dialog->setWindowFlag(Qt::WindowStaysOnTopHint);
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

    class KTextItemPropertyController {
    private:
        KTextItemController* Item = nullptr;
        KTextItemPropertyView* PropertyView = new KTextItemPropertyView;
    public:
        KTextItemPropertyController(){
            QObject::connect(PropertyView, &KTextItemPropertyView::FontChangedSignal,[=](const QFont& Font){
                if(this->Item){
                    Item->GetItemView<KTextItemView>()->prepareGeometryChange();
                    Item->GetItemView<KTextItemView>()->Style.Font = Font;
                    Item->GetItemView<KTextItemView>()->update();
                }
            });
            QObject::connect(PropertyView, &KTextItemPropertyView::AliasChangedSignal,[=](const QString& Alias){
                if(this->Item){
                    Item->SetAlias(Alias);
                    Item->GetItemView<KTextItemView>()->update();
                }
            });
            QObject::connect(PropertyView, &KTextItemPropertyView::BorderColorChangedSignal,[=](const QColor& Color){
                if(this->Item){
                    Item->GetItemView<KTextItemView>()->Style.BorderColor = Color;
                    Item->GetItemView<KTextItemView>()->update();
                }
            });
            QObject::connect(PropertyView, &KTextItemPropertyView::FontColorChangedSignal,[=](const QColor& Color){
                if(this->Item){
                    Item->GetItemView<KTextItemView>()->Style.TextColor = Color;
                    Item->GetItemView<KTextItemView>()->update();
                }
            });
            QObject::connect(PropertyView, &KTextItemPropertyView::BackgroundColorChangedSignal,[=](const QColor& Color){
                if(this->Item){
                    Item->GetItemView<KTextItemView>()->Style.BackgroundColor = Color;
                    Item->GetItemView<KTextItemView>()->update();
                }
            });
        }
        void SetItem(KTextItemController* Item){
            if(this->Item){
                // disconnect
            }
            this->Item = Item;
            if(Item){
                auto View = Item->GetItemView<KTextItemView>();
                const auto& Style = View->Style;
                PropertyView->FontPicker->SetFont(Style.Font);
                PropertyView->ItemAliasEdit->setText(Item->GetAlias());
                PropertyView->FontColorPicker->SetColor(Style.TextColor);
                PropertyView->BackgroundColorPicker->SetColor(Style.BackgroundColor);
                PropertyView->BorderColorPicker->SetColor(Style.BorderColor);
            }
        }
        KTextItemPropertyView* GetView(){
            return PropertyView;
        }
    };

    //////////////////////////////// Image Item ////////////////////////////////
    class KImageItemController : public KItemController{
    public:
        KImageItemController():KItemController(new KImageItemView){

        }
    };
}
