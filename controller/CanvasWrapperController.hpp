#pragma once
#include<QWidget>
#include<QStackedLayout>
#include <QProgressBar>
#include <QMessageBox>
#include"../view/CanvasWrapperView.hpp"
#include"CanvasController.hpp"
#include<QDockWidget>
#include"../common/serializer/TextSerializer.hpp"
#include"../common/serializer/DBSerializer.hpp"
namespace Kim {
    class KCanvasWrapperController : public QObject{
        Q_OBJECT
    private:
        KCanvasController* CanvasController = new KCanvasController;
        KCanvasWrapperView* View = new KCanvasWrapperView(CanvasController->GetCanvasView());
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
            connect(View,
                    &KCanvasWrapperView::SpecialInputSignal,
                    CanvasController,
                    &KCanvasController::OnSpecialInput);
            connect(View,
                    &KCanvasWrapperView::GroupToTextItem,
                    [=]{
               this->CanvasController->OnGroupToItem(KTextItemView::Type);
            });
        }
        QWidget* GetView(){
            return View;
        }
        KCanvasController* GetCanvasController(){
            return CanvasController;
        }

        void SaveFileHelper(const QString& Filename){
            KDBSerializer Serializer;
            connect(&Serializer,
                    &KSerializer::FinishedSignal,
                    []{
                QMessageBox msgBox;
                msgBox.setText("Saved.");
                msgBox.exec();
            });
            Serializer.Serialize(CanvasController, Filename);
        }

        void LoadFileHelper(const QString& Filename){
            KDBSerializer Serilizer;
            Serilizer.Deserialize(Filename, CanvasController);
        }

    };
}
