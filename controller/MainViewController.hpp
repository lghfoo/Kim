#pragma once
#include"../view/MainView.hpp"
#include"CanvasWrapperController.hpp"
namespace Kim {
    class KMainViewController : public QObject{
        Q_OBJECT
    signals:
        void SaveProjectSignal();
        void OpenProjectSignal();
    public slots:
        void OnNewCanvas(){
            CreateAndChangeActive();
        }

        void OnOpenCanvas(){

        }
    private:
        KMainView* MainView = new KMainView;
        QList<KCanvasWrapperController*> CanvasWrapperControllers;
        KCanvasWrapperController* ActiveCanvasWrapper = nullptr;
        QTimer* UpdateStatusBarTimer = new QTimer;
    public:
        KMainViewController(){
            connect(MainView, &KMainView::SaveProjectSignal, this, &KMainViewController::SaveProjectSignal);
            connect(MainView, &KMainView::OpenProjectSignal, this, &KMainViewController::OpenProjectSignal);
            connect(MainView, &KMainView::NewCanvasSignal, this, &KMainViewController::OnNewCanvas);
            connect(MainView, &KMainView::OpenCanvasSignal, this, &KMainViewController::OnOpenCanvas);
            this->MainView->setCentralWidget(MainView->CanvasTabs);
            connect(UpdateStatusBarTimer,
                    &QTimer::timeout,
                    this,
                    &KMainViewController::UpdateStatusBar);
            UpdateStatusBarTimer->start(100);
            CreateAndChangeActive();
        }
        ~KMainViewController(){
            UpdateStatusBarTimer->stop();
            delete UpdateStatusBarTimer;
        }
        KMainView* GetMainView(){
            return MainView;
        }
        void CreateAndChangeActive(){
            ActiveCanvasWrapper = new KCanvasWrapperController;
            ActiveCanvasWrapper->SetCanvasName(CreateCanvasName());
            MainView->CanvasTabs->addTab(ActiveCanvasWrapper->GetView(), ActiveCanvasWrapper->GetCanvasName());
            MainView->CanvasTabs->setCurrentWidget(ActiveCanvasWrapper->GetView());
            CanvasWrapperControllers.append(ActiveCanvasWrapper);
        }
        void ChangeActive(KCanvasWrapperController* Active){
            MainView->CanvasTabs->setCurrentWidget(Active->GetView());
        }
        QString CreateCanvasName(){
            int MinOrder = 1;
            for(auto Controller : CanvasWrapperControllers){
                const auto& Name = Controller->GetCanvasName();
                const auto& List = Name.split('_');
                if(List.size() == 2 && List[0] == "Canvas"){
                    bool Ok = true;
                    int Cnt = List[1].toInt(&Ok);
                    if(Ok){
                        if(Cnt >= MinOrder){
                            MinOrder = Cnt + 1;
                        }
                    }
                }
            }
            return QString("Canvas_") + QString::number(MinOrder);
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
