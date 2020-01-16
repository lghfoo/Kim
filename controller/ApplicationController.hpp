#pragma once
#include"../view/MainView.hpp"
#include"MainViewController.hpp"
namespace Kim {
    class KApplicationController{
    private:
        KMainViewController* MainViewController = new KMainViewController;
    public:
        static void Startup(){
            static KApplicationController* ApplicationController = new KApplicationController;
            ApplicationController->MainViewController->GetMainView()->show();
        }
    };
}
