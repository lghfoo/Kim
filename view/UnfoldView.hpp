#pragma once
#include"CanvasView.hpp"

#include <QPushButton>
#include <QVBoxLayout>
namespace Kim {
    class KUnfoldViewController;
    class KUnfoldView : public QDialog{
        Q_OBJECT
        friend class KUnfoldViewController;
    signals:
        void CloseSignal();
    private:
        KCanvasView* CanvasView = new KCanvasView;
        QPushButton* OkButton = new QPushButton(tr("OK"));
        QPushButton* CancelButton = new QPushButton(tr("Cancel"));
        QVBoxLayout* Layout = new QVBoxLayout;
        QHBoxLayout* BtnLayout = new QHBoxLayout;
    protected:
        virtual void closeEvent(QCloseEvent* Event)override{
            emit CloseSignal();
            QDialog::closeEvent(Event);
        }
    public:
        KUnfoldView(){
            Layout->addWidget(CanvasView);
            BtnLayout->addWidget(OkButton);
            BtnLayout->addWidget(CancelButton);
            Layout->addLayout(BtnLayout);
            this->setLayout(Layout);
        }

        virtual ~KUnfoldView()override{
            // TODO:
            // need delete manually?
            // can not auto delete custom class or graphic view?
            delete Layout;
            delete CanvasView->GetScene();
            delete CanvasView;
        }
    };
}
