#pragma once
#include<QGraphicsObject>
namespace Kim {
    class KGraphicsViewBase : public QGraphicsObject{
        Q_OBJECT
    signals:
        void SelectedChangedSignal(bool Selected);
    public:
        KGraphicsViewBase(){
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges);
        }

        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)override{
            if(change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged){
                emit SelectedChangedSignal(this->isSelected());
            }
            return QGraphicsObject::itemChange(change, value);
        }
    };

}
