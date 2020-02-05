#pragma once
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include<QWidget>
#include <QDesktopWidget>
#include"CanvasView.hpp"
#include"ToolView/SpecialInputView.hpp"
#include"ToolView/HelpView.hpp"
namespace Kim {
    class KCanvasWrapperController;
    class KPropertyView : public QWidget{
    private:
        QLabel* Holder = new QLabel("No Property");
        QWidget* OldWidget = nullptr;
        QVBoxLayout* Layout = new QVBoxLayout;
    public:
        KPropertyView(){
            Layout->addWidget(Holder);
            OldWidget = Holder;
            this->setLayout(Layout);
        }
        void SwitchWidget(QWidget* Widget){
            if(Widget == OldWidget)return;
            if(OldWidget){
                Layout->removeWidget(OldWidget);
                OldWidget->setParent(nullptr);
            }
            Layout->addWidget(Widget);
        }
    };

    class KCanvasWrapperView : public QWidget{
        Q_OBJECT
        friend class KCanvasWrapperController;
    signals:
        void SpecialInputSignal(const QString& Text);
        void SaveCanvasSignal();
        void LoadCanvasSignal();
        void SaveCanvasAsSignal();
        void GroupToTextItemSignal();
        void GroupToImageItemSignal();
        void InsertTextItemSignal();
        void InsertImageItemSignal();
        void CloseSignal();
    private:
        QWidget* ToolWidget = new QWidget;
        KCanvasView* CanvasView = nullptr;
        KSpecialInputView* SpecialInputView = new KSpecialInputView;
        KHelpView* HelpView = new KHelpView;
        KPropertyView* ObjectPropertyView = new KPropertyView;
        KCanvasWrapperController* Controller = nullptr;
    protected:
        virtual void resizeEvent(QResizeEvent* Event)override{
            QWidget::resizeEvent(Event);
//            ToolWidget->resize(this->width(), this->height());
//            UpdateMask();
        }

        virtual void closeEvent(QCloseEvent* Event)override{
            Event->ignore();
            emit CloseSignal();
        }


        virtual void keyPressEvent(QKeyEvent* Event)override{
            if(Event->key() == Qt::Key_Escape){
                ToolWidget->setVisible(!ToolWidget->isVisible());
            }
            QWidget::keyPressEvent(Event);
        }
    public:
        KCanvasWrapperView(KCanvasView* CanvasView):CanvasView(CanvasView){
            QLayout* Layout = new QHBoxLayout;
            Layout->addWidget(CanvasView);
            Layout->addWidget(ToolWidget);
            this->setLayout(Layout);

            QLayout* ToolLayout = new QVBoxLayout;
            ToolWidget->setLayout(ToolLayout);
            QPushButton* ShowHelpBtn = new QPushButton(tr("Help"));
            QPushButton* ShowSpecialInputBtn = new QPushButton(tr("Special Input"));
            QPushButton* LoadCanvasBtn = new QPushButton(tr("Load Canvas"));
            QPushButton* SaveCanvasBtn = new QPushButton(tr("Save Canvas"));
            QPushButton* SaveCanvasAsBtn = new QPushButton(tr("Save Canvas As"));
            QPushButton* GroupToTextBtn = new QPushButton(tr("Group To Text"));
            QPushButton* GroupToImageBtn = new QPushButton(tr("Group To Image"));
            QPushButton* InsertTextBtn = new QPushButton(tr("Insert Text Item"));
            QPushButton* InsertImageBtn = new QPushButton(tr("Insert Image Item"));
            auto Clicked = &QPushButton::clicked;
            connect(ShowHelpBtn,
                    Clicked,
                    [=]{
                HelpView->setWindowFlag(Qt::WindowStaysOnTopHint);
                HelpView->show();
            });
            connect(ShowSpecialInputBtn,
                    Clicked,
                    [=]{
                SpecialInputView->setWindowFlag(Qt::WindowStaysOnTopHint);
                SpecialInputView->show();
            });
            connect(LoadCanvasBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::LoadCanvasSignal);
            connect(SaveCanvasBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::SaveCanvasSignal);
            connect(SaveCanvasAsBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::SaveCanvasAsSignal);
            connect(GroupToTextBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::GroupToTextItemSignal);
            connect(GroupToImageBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::GroupToImageItemSignal);
            connect(InsertTextBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::InsertTextItemSignal);
            connect(InsertImageBtn,
                    Clicked,
                    this,
                    &KCanvasWrapperView::InsertImageItemSignal);

            ToolLayout->addWidget(ShowHelpBtn);
            ToolLayout->addWidget(ShowSpecialInputBtn);
            ToolLayout->addWidget(LoadCanvasBtn);
            ToolLayout->addWidget(SaveCanvasBtn);
            ToolLayout->addWidget(SaveCanvasAsBtn);
            ToolLayout->addWidget(GroupToTextBtn);
            ToolLayout->addWidget(GroupToImageBtn);
            ToolLayout->addWidget(InsertTextBtn);
            ToolLayout->addWidget(InsertImageBtn);
            ToolLayout->addWidget(ObjectPropertyView);
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

        void SetController(KCanvasWrapperController* Controller){
            this->Controller = Controller;
        }

        void MoveToScreenCenter()
        {
            move(qApp->desktop()->availableGeometry(this).center()-rect().center());
        }

        KCanvasWrapperController* GetController(){
            return this->Controller;
        }

    };
}
