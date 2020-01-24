#pragma once
#include<QWidget>
#include<QStackedLayout>
#include"CanvasController.hpp"
#include"../common/serializer/TextSerializer.hpp"
namespace Kim {
    class KCanvasWrapperController : public QObject{
        Q_OBJECT
    private:
        QWidget* View = new QWidget;
        KCanvasController* CanvasController = new KCanvasController;
        QString SavedFilename = "";
        QString LastSavedDir = "";
        QString LastLoadDir = "";
    public slots:
        void Save(){
            if(SavedFilename.isEmpty())SaveAs();
            else SaveFileHelper(SavedFilename);
        }

        void Load(){
            const QString& LoadFilename = QFileDialog::getOpenFileName(View, QObject::tr("Load Kim File"),
                                      LastLoadDir, QObject::tr("Kim Files (*.kim)"));
            if(LoadFilename.isEmpty())return;
            QFileInfo FileInfo(LoadFilename);
            LastLoadDir = FileInfo.absolutePath();
            LoadFileHelper(LoadFilename);
        }

        void SaveAs(){
            SavedFilename = QFileDialog::getSaveFileName(View, QObject::tr("Save Kim File"),
                                                         LastSavedDir, QObject::tr("Kim Files (*.kim)"));
            if(SavedFilename.isEmpty())return;
            QFileInfo FileInfo(SavedFilename);
            LastSavedDir = FileInfo.absolutePath();
            SaveFileHelper(SavedFilename);
        }
    public:
        KCanvasWrapperController(){
            QLayout* Layout = new QStackedLayout;
            Layout->addWidget(CanvasController->GetCanvasView());
            View->setLayout(Layout);
            connect(CanvasController,
                    &KCanvasController::SaveSingal,
                    this,
                    &KCanvasWrapperController::Save);
            connect(CanvasController,
                    &KCanvasController::SaveAsSignal,
                    this,
                    &KCanvasWrapperController::SaveAs);
            connect(CanvasController,
                    &KCanvasController::LoadSignal,
                    this,
                    &KCanvasWrapperController::Load);
        }
        QWidget* GetView(){
            return View;
        }
        KCanvasController* GetCanvasController(){
            return CanvasController;
        }

        void SaveFileHelper(const QString& Filename){
            KTextSerializer Serializer;
            Serializer.Serialize(CanvasController, Filename);
        }

        void LoadFileHelper(const QString& Filename){
            KTextSerializer Serilizer;
            Serilizer.Deserialize(Filename, CanvasController);
        }

    };
}