﻿#pragma once
#include<QGraphicsObject>
#include"../view/GraphicsViewBase.hpp"
namespace Kim {
    class KGraphicsObjectController : public QObject{
        Q_OBJECT
    signals:
        void SelectedChangedSignal(KGraphicsObjectController* Controller,
                                   bool Selected);
    protected:
        QGraphicsObject* GraphicsObject = nullptr;
    public:
        KGraphicsObjectController(QGraphicsObject* Object):GraphicsObject(Object){
            auto View = static_cast<KGraphicsViewBase*>(Object);
            connect(View,
                    &KGraphicsViewBase::SelectedChangedSignal,
                    this,
                    [=](bool Selected){
                emit SelectedChangedSignal(this, Selected);
            });
            View->SetController(this);
        }

        void RequestFocus(Qt::FocusReason Reason = Qt::NoFocusReason){
            this->GraphicsObject->setFocus(Reason);
        }

        int type()const{
            return GraphicsObject->type();
        }

        QGraphicsObject* GetGraphicsObject(){
            return GraphicsObject;
        }

        void SetGraphicsObject(QGraphicsObject* Object){
            this->GraphicsObject = Object;
        }

        bool IsSelected(){
            return GraphicsObject->isSelected();
        }

        void SetSelected(bool Selected){
            GraphicsObject->setSelected(Selected);
        }
    };

}
