#pragma once
#include"../view/CanvasView.hpp"
#include"../view/ItemView.hpp"
namespace Kim {
    class KSceneController{
    private:
        KScene* Scene = new KScene;
    public:
        KScene* GetScene(){
            return Scene;
        }
        void AddTextItem(){
            KTextItemView* View = new KTextItemView;
            Scene->addItem(View);
        }
    };

    class KCanvasController : public QObject{
        Q_OBJECT
    private:
        KCanvasView* CanvasView = new KCanvasView;
        KSceneController* SceneController = new KSceneController;
    public slots:
        void OnKeyRelease(QKeyEvent* Event){
            switch (Event->key()) {
            case Qt::Key_Space:
                OnSpaceRelease();
                break;
            default:
                break;
            }
        }
    public:
        KCanvasController(){
            CanvasView->setScene(SceneController->GetScene());
            QObject::connect(CanvasView, &KCanvasView::KeyReleaseSignal,
                             this, &KCanvasController::OnKeyRelease);
        }
        KCanvasView* GetCanvasView(){
            return CanvasView;
        }
        void OnSpaceRelease(){
            SceneController->AddTextItem();
        }
    };
}
