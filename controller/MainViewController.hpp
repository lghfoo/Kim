#pragma once
#include"../view/MainView.hpp"
#include"CanvasController.hpp"
namespace Kim {
    class KMainViewController{
    private:
        KMainView* MainView = new KMainView;
        KCanvasController* CanvasController = new KCanvasController;
    public:
        KMainViewController(){
            MainView->setWindowTitle("Kim-Keep Everything in Mind");
            MainView->resize(800, 600);
            MainView->setCentralWidget(CanvasController->GetCanvasView());
        }
        KMainView* GetMainView(){
            return MainView;
        }
    };
}
