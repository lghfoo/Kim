#pragma once
#include<QWidget>
#include<QStackedLayout>
#include <QProgressBar>
#include <QMessageBox>
#include"../view/CanvasWrapperView.hpp"
#include"CanvasController.hpp"
#include<QDockWidget>
namespace Kim {
    class KCanvasWrapperController : public QObject{
        Q_OBJECT
    signals:
        void SaveSignal(KCanvasController* Canvas);
        void SaveAsSignal(KCanvasController* Canvas);
        void LoadSignal(KCanvasController* Canvas);
    public slots:
    private:
        KCanvasController* CanvasController = new KCanvasController;
        KCanvasWrapperView* View = new KCanvasWrapperView(CanvasController->GetCanvasView());
    public:
        KCanvasWrapperController(){
            connect(CanvasController,
                    &KCanvasController::SaveSingal,
                    [=]{
                emit SaveSignal(CanvasController);
            });
            connect(CanvasController,
                    &KCanvasController::SaveAsSignal,
                    [=]{
                emit SaveAsSignal(CanvasController);
            });
            connect(CanvasController,
                    &KCanvasController::LoadSignal,
                    [=]{
                emit LoadSignal(CanvasController);
            });
            connect(View,
                    &KCanvasWrapperView::SaveCanvasSignal,
                    [=]{
                emit SaveSignal(CanvasController);
            });
            connect(View,
                    &KCanvasWrapperView::SaveCanvasAsSignal,
                    [=]{
                emit SaveAsSignal(CanvasController);
            });
            connect(View,
                    &KCanvasWrapperView::LoadCanvasSignal,
                    [=]{
               emit LoadSignal(CanvasController);
            });
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
        ~KCanvasWrapperController(){
            delete View;
            delete CanvasController;
        }
        QWidget* GetView(){
            return View;
        }
        KCanvasController* GetCanvasController(){
            return CanvasController;
        }
        QString GetCanvasName(){
            return CanvasController->GetCanvasName();
        }
        void SetCanvasName(const QString& Name){
            CanvasController->SetCanvasName(Name);
        }
    };
}
