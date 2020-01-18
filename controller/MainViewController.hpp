#pragma once
#include"../view/MainView.hpp"
#include"CanvasController.hpp"
namespace Kim {
    class KMainViewController : public QObject{
        Q_OBJECT
    private:
        KMainView* MainView = new KMainView;
        KCanvasController* CanvasController = new KCanvasController;
        QTimer* UpdateStatusBarTimer = new QTimer;
    public:
        KMainViewController(){
            MainView->setCentralWidget(CanvasController->GetCanvasView());
            connect(UpdateStatusBarTimer,
                    &QTimer::timeout,
                    this,
                    &KMainViewController::UpdateStatusBar);
            UpdateStatusBarTimer->start(200);
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
            const KCanvasState& CanvasState = CanvasController->GetCanvasState();
            QString MoveTargetType = "", AddPosType = "";
            switch (CanvasState.MoveTargetType) {
            case KCanvasState::MoveItem:
                MoveTargetType = "Item";
                break;
            case KCanvasState::MoveCursor:
                MoveTargetType = "Cursor";
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
            QString Message = QString("Move Target: %1|Add Object: %2")
                    .arg(MoveTargetType)
                    .arg(AddPosType);
            MainView->statusBar()->showMessage(Message);
        }
    };
}
