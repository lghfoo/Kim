#pragma once

#include"../../controller/MainViewController.hpp"
namespace Kim {
    class KSerializer : public QObject{
        Q_OBJECT
    signals:
        void TotalProgressSignal(int Total);
        void ProgressSignal(int Current);
        void FinishedSignal();
    protected:
        QString Path;
    public:
        KSerializer(const QString& Path):Path(Path){

        }
        /**
         * @brief Serialize
         * @param CanvasController: the controller to serialize from, should be not null
         * @param OutputPath: output path
         */
        virtual void Serialize(KCanvasController* CanvasController) = 0;
        virtual void Serialize(KMainViewController* MainController) = 0;
        /**
         * @brief Deserialize
         * @param InputPath: input path
         * @param CanvasController: the controller to serialize to, should be not null
         */
        virtual void Deserialize(KCanvasController* CanvasController, const QVariant& CanvasID = QVariant()) = 0;
        virtual void Deserialize(KMainViewController* MainController) = 0;
        /**
         * @brief ~KSerializer
         */
        virtual ~KSerializer(){}
    };
}
