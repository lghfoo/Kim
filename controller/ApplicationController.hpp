#pragma once
#include"../view/MainView.hpp"
#include"MainViewController.hpp"
#include"../common/serializer/TextSerializer.hpp"
#include"../common/serializer/DBSerializer.hpp"
namespace Kim {
    class KApplicationController : public QObject{
        Q_OBJECT
    public slots:
        //////////////////////////////// Project ////////////////////////////////
        void OnSaveProject(){
            if(SavedProjectFilename.isEmpty())OnSaveProjectAs();
            else SaveProjectHelper(MainViewController, SavedProjectFilename);
        }

        void OnOpenProject(){
            const QString& OpenFilename = QFileDialog::getOpenFileName(MainViewController->GetMainView(), QObject::tr("Open Kim Project"),
                                      QFileInfo(SavedProjectFilename).absolutePath(), QObject::tr("Kim Files (*.kim)"));
            if(OpenFilename.isEmpty())return;
            OpenProjectHelper(MainViewController, OpenFilename);
        }

        void OnSaveProjectAs(){
            SavedProjectFilename = QFileDialog::getSaveFileName(MainViewController->GetMainView(), QObject::tr("Save Kim Project"),
                                                         QFileInfo(SavedProjectFilename).absolutePath(), QObject::tr("Kim Files (*.kim)"));
            if(SavedProjectFilename.isEmpty())return;
            SaveProjectHelper(MainViewController, SavedProjectFilename);
        }

        void OnOpenCanvas(){
            const QString& OpenFilename = QFileDialog::getOpenFileName(MainViewController->GetMainView(), QObject::tr("Open Kim Canvas"),
                                      QFileInfo(SavedProjectFilename).absolutePath(), QObject::tr("Kim Files (*.kim)"));
            if(OpenFilename.isEmpty())return;
            OpenCanvasHelper(OpenFilename);
        }

        void OnSaveCanvas(KCanvasController* Canvas, const QString& Filename){
            KDBSerializer Serializer(Filename);
            connect(&Serializer,
                    &KSerializer::FinishedSignal,
                    []{
                QMessageBox msgBox;
                msgBox.setText("Saved.");
                msgBox.exec();
            });
            Serializer.Serialize(Canvas);
        }

        void OnLoadCanvas(KCanvasController* Canvas, const QString& Filename){
            KDBSerializer Serilizer(Filename);
            Serilizer.Deserialize(Canvas);
        }
    private:
        KMainViewController* MainViewController = new KMainViewController;
        QString SavedProjectFilename = "";
    public:
        KApplicationController(){
            connect(MainViewController, &KMainViewController::SaveProjectSignal, this, &KApplicationController::OnSaveProject);
            connect(MainViewController, &KMainViewController::SaveAsProjectSignal, this, &KApplicationController::OnSaveProjectAs);
            connect(MainViewController, &KMainViewController::OpenProjectSignal, this, &KApplicationController::OnOpenProject);
            connect(MainViewController, &KMainViewController::OpenCanvasSignal, this, &KApplicationController::OnOpenCanvas);
            connect(MainViewController, &KMainViewController::SaveCanvasSignal, this, &KApplicationController::OnSaveCanvas);
            connect(MainViewController, &KMainViewController::LoadCanvasSignal, this, &KApplicationController::OnLoadCanvas);
        }
        static void Startup(){
            static KApplicationController* ApplicationController = new KApplicationController;
            ApplicationController->MainViewController->GetMainView()->show();
        }

        void SaveProjectHelper(KMainViewController* Project, const QString& Filename){
            KDBSerializer Serializer(Filename);
            connect(&Serializer,
                    &KSerializer::FinishedSignal,
                    []{
                QMessageBox msgBox;
                msgBox.setText("Saved.");
                msgBox.exec();
            });
            Serializer.Serialize(Project);
        }

        void OpenProjectHelper(KMainViewController* Project, const QString& Filename){
            KDBSerializer Serilizer(Filename);
            Serilizer.Deserialize(Project);
        }

        void OpenCanvasHelper(const QString& Filename){
            auto CanvasWrapper = MainViewController->CreateCanvasWrapper();
            auto Canvas = CanvasWrapper->GetCanvasController();
            OnLoadCanvas(Canvas, Filename);
            MainViewController->AddCanvasWrapper(CanvasWrapper);
            MainViewController->ChangeActiveWrapper(CanvasWrapper);
        }

    };
}
