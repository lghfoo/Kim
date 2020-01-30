#pragma once
#include"../view/MainView.hpp"
#include"CanvasWrapperController.hpp"
namespace Kim {
    class KMainViewController : public QObject{
        Q_OBJECT
    private:
        KMainView* MainView = new KMainView;
        QList<KCanvasWrapperController*> CanvasWrapperControllers;
        KCanvasWrapperController* ActiveCanvasWrapper = nullptr;
        QTimer* UpdateStatusBarTimer = new QTimer;
    public:
        KMainViewController(){
            ActiveCanvasWrapper = new KCanvasWrapperController;
            this->MainView->setCentralWidget(ActiveCanvasWrapper->GetView());
            connect(UpdateStatusBarTimer,
                    &QTimer::timeout,
                    this,
                    &KMainViewController::UpdateStatusBar);
            UpdateStatusBarTimer->start(100);
        }
        ~KMainViewController(){
            UpdateStatusBarTimer->stop();
            delete UpdateStatusBarTimer;
        }
        KMainView* GetMainView(){
            return MainView;
        }
    public slots:
        void UpdateStatusBar(){
            using KCanvasState = KCanvasController::KCanvasState;
            QString Message = "";
            if(ActiveCanvasWrapper){
                const KCanvasState& CanvasState = ActiveCanvasWrapper->GetCanvasController()->GetCanvasState();
                QString MoveTargetType = "",
                        AddPosType = "",
                        WriteDirect = "";
                switch (CanvasState.MoveTargetType) {
                case KCanvasState::MoveItem:
                    MoveTargetType = "Item";
                    break;
                case KCanvasState::MoveCursor:
                    MoveTargetType = "Cursor";
                    break;
                default:
                    break;
                }

                switch (CanvasState.AddPosType) {
                case KCanvasState::AtMouse:
                    AddPosType = "AtMouse";
                    break;
                case KCanvasState::AtCursor:
                    AddPosType = "AtCursor";
                    break;
                case KCanvasState::AtNearest:
                    AddPosType = "AtNearest";
                    break;
                default:
                    break;
                }

                if(CanvasState.WriteDirectly){
                    WriteDirect = "On";
                }
                else{
                    WriteDirect = "Off";
                }
                Message = QString("Move Target: [%1] | Add Object: [%2] | Write Direct: [%3]")
                        .arg(MoveTargetType)
                        .arg(AddPosType)
                        .arg(WriteDirect);
            }
            MainView->statusBar()->showMessage(Message);
        }
    };
}
