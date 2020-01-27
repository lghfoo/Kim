#pragma once
#include <QGridLayout>
#include <QPushButton>
#include<QWidget>
#include"CanvasView.hpp"
namespace Kim {
    class KCanvasWrapperView : public QWidget{
    private:
        QWidget* ToolWidget = new QWidget;
        KCanvasView* CanvasView = nullptr;
    protected:
        virtual void resizeEvent(QResizeEvent* Event)override{
            QWidget::resizeEvent(Event);
            ToolWidget->resize(this->width(), this->height());
//            UpdateMask();
        }
    public:
        KCanvasWrapperView(KCanvasView* CanvasView):CanvasView(CanvasView){
            QLayout* Layout = new QHBoxLayout;
            Layout->addWidget(CanvasView);
            this->setLayout(Layout);
            QLayout* GridLayout = new QVBoxLayout;
//            QPushButton* Btn = new QPushButton(tr("hello world"));
//            GridLayout->addWidget(Btn);
//            GridLayout->addWidget(new QPushButton(tr("emmmmm...")));
            ToolWidget->setLayout(GridLayout);
            ToolWidget->setParent(this);
            // Mask当没有children时不太好用
            // 暂时使用Attribute
            ToolWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
//            UpdateMask();
        }

        void UpdateMask(){
            QRegion Reg = ToolWidget->frameGeometry();
            qDebug()<<"frame geo:"<<Reg;
            Reg -= ToolWidget->geometry();
            qDebug()<<"reg"<<Reg;
            Reg += ToolWidget->childrenRegion();
            qDebug()<<"child"<<ToolWidget->childrenRegion();
            qDebug()<<Reg;
            ToolWidget->setMask(Reg);
//            ToolWidget->setMask(ToolWidget->childrenRegion());
        }
    };
}
