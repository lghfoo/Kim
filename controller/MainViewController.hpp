#pragma once
#include"../view/MainView.hpp"
#include"CanvasWrapperController.hpp"
namespace Kim {
    class KSerializer;
    class KDBSerializer;
    class KTextSerializer;
    class KMainViewController : public QObject{
        friend class KSerializer;
        friend class KDBSerializer;
        friend class KTextSerializer;
        Q_OBJECT
    signals:
        void SaveProjectSignal();
        void SaveAsProjectSignal();
        void OpenProjectSignal();
        void OpenCanvasSignal();
        void LoadCanvasSignal(KCanvasController*);
        void SaveCanvasSignal(KCanvasController*);
        void SaveAsCanvasSignal(KCanvasController*);
    public slots:
        void OnNewCanvas(){
            CreateAndChangeActive();
        }
    private:
        KMainView* MainView = new KMainView;
        QList<KCanvasWrapperController*> CanvasWrapperControllers;
        KCanvasWrapperController* ActiveCanvasWrapper = nullptr;
        QTimer* UpdateStatusBarTimer = new QTimer;
    public:
        KMainViewController(){
            connect(MainView, &KMainView::SaveProjectSignal, this, &KMainViewController::SaveProjectSignal);
            connect(MainView, &KMainView::SaveAsProjectSignal, this, &KMainViewController::SaveAsProjectSignal);
            connect(MainView, &KMainView::OpenProjectSignal, this, &KMainViewController::OpenProjectSignal);
            connect(MainView, &KMainView::OpenCanvasSignal, this, &KMainViewController::OpenCanvasSignal);
            connect(MainView, &KMainView::NewCanvasSignal, this, &KMainViewController::OnNewCanvas);
            connect(MainView, &KMainView::ShowSignal, this,[=]{
                // 刚打开应用的时候
                // 要等整个window show之后才进行居中
                if(ActiveCanvasWrapper){
                    ActiveCanvasWrapper->GetCanvasController()->GetCanvasView()->ScrollToCenter();
                }
            });
            connect(MainView->CanvasTabs, &QTabWidget::currentChanged,
                    [=](int Index){
                for(auto Wrapper : CanvasWrapperControllers){
                    if(Wrapper->GetView() == MainView->CanvasTabs->widget(Index)){
                        this->ActiveCanvasWrapper = Wrapper;
                        break;
                    }
                }
            });
            this->MainView->setCentralWidget(MainView->CanvasTabs);
            connect(UpdateStatusBarTimer,
                    &QTimer::timeout,
                    this,
                    &KMainViewController::UpdateStatusBar);
            UpdateStatusBarTimer->start(100);
            CreateAndChangeActive();
        }
        ~KMainViewController(){
            Clear();
            UpdateStatusBarTimer->stop();
            delete UpdateStatusBarTimer;
        }
        void Clear(){
            MainView->CanvasTabs->clear();
            ActiveCanvasWrapper = nullptr;
            while(!CanvasWrapperControllers.isEmpty()){
                auto Wrapper = CanvasWrapperControllers.front();
                CanvasWrapperControllers.pop_front();
                delete Wrapper;
            }
        }
        KMainView* GetMainView(){
            return MainView;
        }
        QList<KCanvasController*>GetCanvasControlleres(){
            QList<KCanvasController*>Ret{};
            for(auto Canvas : CanvasWrapperControllers){
                Ret.append(Canvas->GetCanvasController());
            }
            return Ret;
        }
        void CreateAndChangeActive(){
            auto Wrapper = CreateCanvasWrapper();
            AddCanvasWrapper(Wrapper);
            ChangeActiveWrapper(Wrapper);
        }
        void Connect(KCanvasWrapperController* Controller){
            connect(Controller, &KCanvasWrapperController::SaveSignal,
                    this, &KMainViewController::SaveCanvasSignal);
            connect(Controller, &KCanvasWrapperController::SaveAsSignal,
                    this, &KMainViewController::SaveAsCanvasSignal);
            connect(Controller, &KCanvasWrapperController::LoadSignal,
                    this, &KMainViewController::LoadCanvasSignal);
        }
        void Disconnect(){

        }
        KCanvasWrapperController* CreateCanvasWrapper(){
            auto Wrapper = new KCanvasWrapperController;
            Wrapper->SetCanvasName(CreateCanvasName());
            return Wrapper;
        }
        void AddCanvasWrapper(KCanvasWrapperController* Controller){
            ActiveCanvasWrapper = Controller;
            MainView->CanvasTabs->addTab(Controller->GetView(), Controller->GetCanvasName());
            CanvasWrapperControllers.append(Controller);
            Connect(Controller);
        }
        void ChangeActiveWrapper(KCanvasWrapperController* Active){
            ActiveCanvasWrapper = Active;
            MainView->CanvasTabs->setCurrentWidget(Active->GetView());
        }
        void RefreshActive(){
            if(ActiveCanvasWrapper)
                MainView->CanvasTabs->setCurrentWidget(ActiveCanvasWrapper->GetView());
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
