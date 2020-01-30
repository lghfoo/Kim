#pragma once
#include <QDockWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>
#include "CanvasView.hpp"
namespace Kim {
    class KMainView : public QMainWindow{
    private:
        QStatusBar* StatusBar = new QStatusBar;
        QTabWidget* CanvasTabs = new QTabWidget;
    public:
        KMainView(){
            this->setWindowTitle("Kim-Keep Everything in Mind");
            this->resize(800, 600);
            this->setStatusBar(StatusBar);

            QPushButton* SaveProjBtn = new QPushButton(tr("Save Proj"));
            QPushButton* OpenProjBtn = new QPushButton(tr("Open Proj"));
            QPushButton* AddCanvasBtn = new QPushButton(tr("New Canvas"));
            QPushButton* OpenCanvasBtn = new QPushButton(tr("Open Canvas"));
            QVBoxLayout* Layout = new QVBoxLayout;
            Layout->addWidget(SaveProjBtn);
            Layout->addWidget(OpenProjBtn);
            Layout->addWidget(AddCanvasBtn);
            Layout->addWidget(OpenCanvasBtn);
            QWidget* BtnWidget = new QWidget;
            BtnWidget->setLayout(Layout);
            QDockWidget* LeftDoc = new QDockWidget;
            LeftDoc->setWidget(BtnWidget);
            this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, LeftDoc);
        }
    };
}
