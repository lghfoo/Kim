#pragma once
#include <QDockWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>
#include "CanvasView.hpp"
namespace Kim {
    class KMainViewController;
    class KMainView : public QMainWindow{
        Q_OBJECT
        friend class KMainViewController;
    signals:
        void SaveProjectSignal();
        void SaveAsProjectSignal();
        void OpenProjectSignal();
        void OpenCanvasSignal();
        void NewCanvasSignal();
        void ShowSignal();
    private:
        QStatusBar* StatusBar = new QStatusBar;
        QTabWidget* CanvasTabs = new QTabWidget;
    protected:
        virtual void showEvent(QShowEvent* Event) override{
            QMainWindow::showEvent(Event);
            emit ShowSignal();
        }
    public:
        KMainView(){
            this->setWindowTitle("Kim-Keep Everything in Mind");
            this->resize(800, 600);
            this->setStatusBar(StatusBar);

            QPushButton* SaveProjBtn = new QPushButton(tr("Save Project"));
            QPushButton* SaveAsProjBtn = new QPushButton(tr("Save Project As..."));
            QPushButton* OpenProjBtn = new QPushButton(tr("Open Project"));
            QPushButton* AddCanvasBtn = new QPushButton(tr("New Canvas"));
            QPushButton* OpenCanvasBtn = new QPushButton(tr("Open Canvas"));
            connect(SaveProjBtn, &QPushButton::clicked, this, &KMainView::SaveProjectSignal);
            connect(SaveAsProjBtn, &QPushButton::clicked, this, &KMainView::SaveAsProjectSignal);
            connect(OpenProjBtn, &QPushButton::clicked, this, &KMainView::OpenProjectSignal);
            connect(AddCanvasBtn, &QPushButton::clicked, this, &KMainView::NewCanvasSignal);
            connect(OpenCanvasBtn, &QPushButton::clicked, this, &KMainView::OpenCanvasSignal);
            QVBoxLayout* Layout = new QVBoxLayout;
            Layout->addWidget(SaveProjBtn);
            Layout->addWidget(OpenProjBtn);
            Layout->addWidget(AddCanvasBtn);
            Layout->addWidget(OpenCanvasBtn);
            Layout->setAlignment(Qt::AlignTop);
            QWidget* BtnWidget = new QWidget;
            BtnWidget->setLayout(Layout);
            QDockWidget* LeftDoc = new QDockWidget;
            LeftDoc->setWidget(BtnWidget);
            this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, LeftDoc);
        }

    };
}
