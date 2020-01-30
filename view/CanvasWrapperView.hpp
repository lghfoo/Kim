#pragma once
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include<QWidget>
#include"CanvasView.hpp"
#include"ToolView/SpecialInputView.hpp"
#include"ToolView/HelpView.hpp"
namespace Kim {
    class KCanvasWrapperController;
    class KCanvasWrapperView : public QWidget{
        Q_OBJECT
        friend class KCanvasWrapperController;
    signals:
        void SpecialInputSignal(const QString& Text);
        void SaveCanvasSignal();
        void LoadCanvasSignal();
        void GroupToTextItem();
        void GroupToImageItem();
    private:
        QWidget* ToolWidget = new QWidget;
        KCanvasView* CanvasView = nullptr;
        KSpecialInputView* SpecialInputView = new KSpecialInputView;
        KHelpView* HelpView = new KHelpView;
    protected:
        virtual void resizeEvent(QResizeEvent* Event)override{
            QWidget::resizeEvent(Event);
//            ToolWidget->resize(this->width(), this->height());
//            UpdateMask();
        }
    public:
        KCanvasWrapperView(KCanvasView* CanvasView):CanvasView(CanvasView){
            QLayout* Layout = new QHBoxLayout;
            Layout->addWidget(CanvasView);
            this->setLayout(Layout);
            Layout->addWidget(ToolWidget);

            QLayout* ToolLayout = new QVBoxLayout;
            ToolWidget->setLayout(ToolLayout);
            QPushButton* ShowHelpBtn = new QPushButton(tr("Help"));
            QPushButton* ShowSpecialInputBtn = new QPushButton(tr("Special Input"));
            QPushButton* LoadCanvasBtn = new QPushButton(tr("Load Canvas"));
            QPushButton* SaveCanvasBtn = new QPushButton(tr("Save Canvas"));
            QPushButton* GroupToTextBtn = new QPushButton(tr("Group To Text"));
            connect(ShowHelpBtn,
                    &QPushButton::clicked,
                    [=]{
                HelpView->setWindowFlag(Qt::WindowStaysOnTopHint);
                HelpView->show();
            });
            connect(ShowSpecialInputBtn,
                    &QPushButton::clicked,
                    [=]{
                SpecialInputView->setWindowFlag(Qt::WindowStaysOnTopHint);
                SpecialInputView->show();
            });
            connect(LoadCanvasBtn,
                    &QPushButton::clicked,
                    this,
                    &KCanvasWrapperView::LoadCanvasSignal);
            connect(SaveCanvasBtn,
                    &QPushButton::clicked,
                    this,
                    &KCanvasWrapperView::SaveCanvasSignal);
            connect(GroupToTextBtn,
                    &QPushButton::clicked,
                    this,
                    &KCanvasWrapperView::GroupToTextItem);

            ToolLayout->addWidget(ShowHelpBtn);
            ToolLayout->addWidget(ShowSpecialInputBtn);
            ToolLayout->addWidget(LoadCanvasBtn);
            ToolLayout->addWidget(SaveCanvasBtn);
            ToolLayout->addWidget(GroupToTextBtn);
            ToolLayout->setAlignment(Qt::AlignTop);

            connect(SpecialInputView,
                    &KSpecialInputView::InputSignal,
                    this,
                    &KCanvasWrapperView::SpecialInputSignal);
//            QLayout* GridLayout = new QVBoxLayout;
//            GridLayout->addWidget(new QPushButton(tr("hello world")));
//            ToolWidget->setLayout(GridLayout);
//            ToolWidget->setParent(this);
//            GreeceInputView->show();
//            ToolWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
        }

        void UpdateMask(){
//            QRegion Reg = ToolWidget->frameGeometry();
//            Reg -= ToolWidget->geometry();
//            Reg += ToolWidget->childrenRegion();
            // TODO:
            // The code below may hurt performance.
            // Improve if can improve.
            auto Region = ToolWidget->childrenRegion();
            if(Region.isNull()){
                Region = QRegion(0, 0, 1, 1);
            }
            ToolWidget->setMask(Region);
        }
    };
}
