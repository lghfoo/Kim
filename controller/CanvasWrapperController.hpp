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
        void SaveSignal(KCanvasController* Canvas, const QString& Filename);
        void LoadSignal(KCanvasController* Canvas, const QString& Filename);
    public slots:
        //////////////////////////////// Canvas ////////////////////////////////
        void OnSaveCanvas(){
            if(SavedCanvasFilename.isEmpty())OnSaveCanvasAs();
            else emit SaveSignal(CanvasController, SavedCanvasFilename);
        }

        void OnLoadCanvas(){
            const QString& LoadFilename = QFileDialog::getOpenFileName(View, QObject::tr("Load Kim Canvas"),
                                      QFileInfo(SavedCanvasFilename).absolutePath(), QObject::tr("Kim Files (*.kim)"));
            if(LoadFilename.isEmpty())return;
            SavedCanvasFilename = LoadFilename;
            emit LoadSignal(CanvasController, LoadFilename);
        }

        void OnSaveCanvasAs(){
            SavedCanvasFilename = QFileDialog::getSaveFileName(View, QObject::tr("Save Kim Canvas"),
                                                         QFileInfo(SavedCanvasFilename).absolutePath(), QObject::tr("Kim Files (*.kim)"));
            if(SavedCanvasFilename.isEmpty())return;
            emit SaveSignal(CanvasController, SavedCanvasFilename);
        }

        void OnCanvasFocusObjectChanged(KGraphicsObjectController* Object){
            if(Object){
                if(Object->type() == KTextItemView::Type){
                    TextItemPropertyController->SetItem(static_cast<KTextItemController*>(Object));
                    View->ObjectPropertyView->SwitchWidget(TextItemPropertyController->GetView());
                }
            }
            else{
                qDebug()<<"focus is null";
            }
        }
    private:
        QString SavedCanvasFilename = "";
        KCanvasController* CanvasController = new KCanvasController;
        KCanvasWrapperView* View = new KCanvasWrapperView(CanvasController->GetCanvasView());
        KTextItemPropertyController* TextItemPropertyController = new KTextItemPropertyController;
    public:
        KCanvasWrapperController(){
            View->SetController(this);
            connect(CanvasController,
                    &KCanvasController::SaveSingal,
                    this,
                    &KCanvasWrapperController::OnSaveCanvas);
            connect(CanvasController,
                    &KCanvasController::SaveAsSignal,
                    this,
                    &KCanvasWrapperController::OnSaveCanvasAs);
            connect(CanvasController,
                    &KCanvasController::LoadSignal,
                    this,
                    &KCanvasWrapperController::OnLoadCanvas);
            connect(View,
                    &KCanvasWrapperView::SaveCanvasSignal,
                    this,
                    &KCanvasWrapperController::OnSaveCanvas);
            connect(View,
                    &KCanvasWrapperView::SaveCanvasAsSignal,
                    this,
                    &KCanvasWrapperController::OnSaveCanvasAs);
            connect(View,
                    &KCanvasWrapperView::LoadCanvasSignal,
                    this,
                    &KCanvasWrapperController::OnLoadCanvas);
            connect(View,
                    &KCanvasWrapperView::SpecialInputSignal,
                    CanvasController,
                    &KCanvasController::OnSpecialInput);
            connect(View,
                    &KCanvasWrapperView::GroupToTextItemSignal,
                    [=]{
               this->CanvasController->OnGroupToItem(KTextItemView::Type);
            });
            connect(View,
                    &KCanvasWrapperView::GroupToImageItemSignal,
                    [=]{
                this->CanvasController->OnGroupToItem(KImageItemView::Type);
            });
            connect(View,
                    &KCanvasWrapperView::InsertTextItemSignal,
                    [=]{
               this->CanvasController->OnInsertItem(KTextItemView::Type);
            });
            connect(View,
                    &KCanvasWrapperView::InsertImageItemSignal,
                    [=]{
                this->CanvasController->OnInsertItem(KImageItemView::Type);
            });
            connect(CanvasController, &KCanvasController::FocusedObjectChangedSignal,
                    this, &KCanvasWrapperController::OnCanvasFocusObjectChanged);
        }
        ~KCanvasWrapperController(){
            delete View;
            delete CanvasController;
        }
        QString GetSavedCanvasFilename(){
            return SavedCanvasFilename;
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
